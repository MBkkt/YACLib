#include <util/async_suite.hpp>
#include <util/time.hpp>

#include <yaclib/algo/wait_group.hpp>
#include <yaclib/async/contract.hpp>
#include <yaclib/async/run.hpp>
#include <yaclib/coro/await.hpp>
#include <yaclib/coro/current_executor.hpp>
#include <yaclib/coro/future.hpp>
#include <yaclib/coro/guard_sticky.hpp>
#include <yaclib/coro/guard_unique.hpp>
#include <yaclib/coro/mutex.hpp>
#include <yaclib/coro/on.hpp>
#include <yaclib/coro/task.hpp>
#include <yaclib/exe/manual.hpp>
#include <yaclib/exe/submit.hpp>
#include <yaclib/runtime/fair_thread_pool.hpp>
#include <yaclib/util/detail/intrusive_list.hpp>

#include <array>
#include <exception>
#include <utility>
#include <yaclib_std/thread>

#include <gtest/gtest.h>

namespace test {
namespace {

TYPED_TEST(AsyncSuite, JustWorks) {
  yaclib::Mutex<> m;
  yaclib::FairThreadPool tp;

  const std::size_t kCoros = 10'000;

  std::array<yaclib::Future<>, kCoros> futures;
  std::size_t cs = 0;

  auto coro1 = [&]() -> typename TestFixture::Type {
    if constexpr (TestFixture::kIsFuture) {
      co_await On(tp);
    }
    co_await m.Lock();
    ++cs;
    m.UnlockHere();
    co_return{};
  };

  for (std::size_t i = 0; i < kCoros; ++i) {
    if constexpr (TestFixture::kIsFuture) {
      futures[i] = coro1();
    } else {
      futures[i] = coro1().ToFuture(tp).On(nullptr);
    }
  }

  yaclib::Wait(futures.begin(), futures.size());

  EXPECT_EQ(kCoros, cs);
  tp.HardStop();
  tp.Wait();
}

TYPED_TEST(AsyncSuite, Counter) {
#if defined(GTEST_OS_WINDOWS) && !(defined(NDEBUG) && defined(_WIN64))
  GTEST_SKIP();  // Doesn't work for Win32 or Debug, I think its probably because bad symmetric transfer implementation
  // TODO(kononovk) Try to confirm problem and localize it with ifdefs
#endif
  yaclib::Mutex<true> m;
  yaclib::FairThreadPool tp;

  const std::size_t kCoros = 20;
  const std::size_t kCSperCoro = 2000;
  std::array<yaclib::Future<>, kCoros> futures;
  std::size_t cs = 0;

  auto coro1 = [&]() -> typename TestFixture::Type {
    if constexpr (TestFixture::kIsFuture) {
      co_await On(tp);
    }
    for (std::size_t j = 0; j < kCSperCoro; ++j) {
      co_await m.Lock();
      ++cs;
      co_await m.Unlock();
    }
    co_return{};
  };

  for (std::size_t i = 0; i < kCoros; ++i) {
    if constexpr (TestFixture::kIsFuture) {
      futures[i] = coro1();
    } else {
      futures[i] = coro1().ToFuture(tp).On(nullptr);
    }
  }
  Wait(futures.begin(), futures.end());

  EXPECT_EQ(kCoros * kCSperCoro, cs);
  tp.HardStop();
  tp.Wait();
}

TEST(MutexSuite, TryLock) {
  yaclib::Mutex<> m;
  EXPECT_TRUE(m.TryLock());
  EXPECT_FALSE(m.TryLock());
  m.UnlockHere();
  EXPECT_TRUE(m.TryLock());
  m.UnlockHere();
}

TEST(MutexSuite, UniqueGuard) {
  yaclib::Mutex<> m;
  {
    auto lock = m.TryGuard();
    EXPECT_TRUE(lock.OwnsLock());
    {
      auto lock2 = m.TryGuard();
      EXPECT_FALSE(lock2.OwnsLock());
    }
  }
  EXPECT_TRUE(m.TryLock());
  m.UnlockHere();
}

TYPED_TEST(AsyncSuite, LockAsync) {
#if defined(GTEST_OS_WINDOWS) && !(defined(NDEBUG) && defined(_WIN64))
  GTEST_SKIP();  // Doesn't work for Win32 or Debug, I think its probably because bad symmetric transfer implementation
  // TODO(kononovk) Try to confirm problem and localize it with ifdefs
#endif
  yaclib::Mutex<> m;
  auto manual = yaclib::MakeManual();
  auto [f1, p1] = yaclib::MakeContract<bool>();
  auto [f2, p2] = yaclib::MakeContract<bool>();

  std::size_t value = 0;

  auto coro = [&](yaclib::Future<bool>& future) -> typename TestFixture::Type {
    if constexpr (TestFixture::kIsFuture) {
      co_await On(*manual);
    }
    co_await m.Lock();
    value++;
    co_await Await(future);
    value++;
    co_await m.UnlockOn(*manual);
    co_return{};
  };
  auto run_coro = [&](auto&& func, auto&& arg) {
    if constexpr (TestFixture::kIsFuture) {
      return func(arg);
    } else {
      return func(arg).ToFuture(*manual).On(nullptr);
    }
  };
  auto c1 = run_coro(coro, f1);
  std::ignore = static_cast<yaclib::ManualExecutor&>(*manual).Drain();
  EXPECT_EQ(1, value);
  EXPECT_FALSE(m.TryLock());

  auto c2 = run_coro(coro, f2);
  std::ignore = static_cast<yaclib::ManualExecutor&>(*manual).Drain();
  EXPECT_EQ(1, value);
  EXPECT_FALSE(m.TryLock());

  std::move(p1).Set(true);
  std::ignore = static_cast<yaclib::ManualExecutor&>(*manual).Drain();
  EXPECT_EQ(3, value);
  EXPECT_FALSE(m.TryLock());

  std::move(p2).Set(true);
  std::ignore = static_cast<yaclib::ManualExecutor&>(*manual).Drain();
  EXPECT_EQ(4, value);
  EXPECT_TRUE(m.TryLock());
  m.UnlockHere();
}

TYPED_TEST(AsyncSuite, ScopedLockAsync) {
  yaclib::Mutex<> m;
  auto manual = yaclib::MakeManual();
  auto [f1, p1] = yaclib::MakeContract<bool>();
  auto [f2, p2] = yaclib::MakeContract<bool>();

  std::size_t value = 0;

  auto coro = [&](yaclib::Future<bool>& future) -> typename TestFixture::Type {
    if constexpr (TestFixture::kIsFuture) {
      co_await On(*manual);
    }
    auto guard = co_await m.UniqueGuard();
    value++;
    co_await Await(future);
    value++;
    co_return{};
  };

  auto run_coro = [&](auto&& func, auto&& arg) {
    if constexpr (TestFixture::kIsFuture) {
      return func(arg);
    } else {
      return func(arg).ToFuture(*manual).On(nullptr);
    }
  };

  auto c1 = run_coro(coro, f1);
  std::ignore = static_cast<yaclib::ManualExecutor&>(*manual).Drain();
  EXPECT_EQ(1, value);
  EXPECT_FALSE(m.TryLock());

  auto c2 = run_coro(coro, f2);
  std::ignore = static_cast<yaclib::ManualExecutor&>(*manual).Drain();
  EXPECT_EQ(1, value);
  EXPECT_FALSE(m.TryLock());

  std::move(p1).Set(true);
  std::ignore = static_cast<yaclib::ManualExecutor&>(*manual).Drain();
  EXPECT_EQ(3, value);
  EXPECT_FALSE(m.TryLock());

  std::move(p2).Set(true);
  std::ignore = static_cast<yaclib::ManualExecutor&>(*manual).Drain();
  EXPECT_EQ(4, value);
  EXPECT_TRUE(m.TryLock());
  m.UnlockHere();
}

TYPED_TEST(AsyncSuite, GuardRelease) {
#if defined(GTEST_OS_WINDOWS) && !(defined(NDEBUG) && defined(_WIN64))
  GTEST_SKIP();  // Doesn't work for Win32 or Debug, I think its probably because bad symmetric transfer implementation
  // TODO(kononovk) Try to confirm problem and localize it with ifdefs
#endif
  yaclib::Mutex<> m;
  yaclib::FairThreadPool tp{2};

  const std::size_t kCoros = 20;
  const std::size_t kCSperCoro = 200;

  std::array<yaclib::Future<int>, kCoros> futures;
  std::size_t cs = 0;
  using Coro = std::conditional_t<TestFixture::kIsFuture, yaclib::Future<int>, yaclib::Task<int>>;
  auto coro1 = [&]() -> Coro {
    for (std::size_t j = 0; j < kCSperCoro; ++j) {
      if constexpr (TestFixture::kIsFuture) {
        co_await On(tp);
      }
      auto g = co_await m.UniqueGuard();
      auto another = yaclib::Mutex<>::GuardUnique{*g.Release(), std::adopt_lock};
      ++cs;
      co_await another.UnlockOn(tp);
    }
    co_return 42;
  };
  for (std::size_t i = 0; i < kCoros; ++i) {
    if constexpr (TestFixture::kIsFuture) {
      futures[i] = coro1();
    } else {
      futures[i] = coro1().ToFuture(tp).On(nullptr);
    }
  }

  Wait(futures.begin(), futures.end());

  EXPECT_EQ(kCoros * kCSperCoro, cs);
  tp.HardStop();
  tp.Wait();
}

TYPED_TEST(AsyncSuite, UnlockHereBehaviour) {
  using namespace std::chrono_literals;
  constexpr std::size_t kThreads = 4;
  constexpr std::size_t kCoros = 4;

  yaclib::FairThreadPool tp{kThreads};
  yaclib::Mutex<> m;
  std::array<yaclib::Future<>, kCoros> futures;
  yaclib_std::atomic_bool start{false};

  auto coro1 = [&]() -> typename TestFixture::Type {
    if constexpr (TestFixture::kIsFuture) {
      co_await On(tp);
    }
    co_await m.Lock();
    start.store(true, std::memory_order_release);
    auto id = yaclib_std::this_thread::get_id();
    yaclib_std::this_thread::sleep_for(1s);
    m.UnlockHere();
    EXPECT_EQ(id, yaclib_std::this_thread::get_id());
    co_return{};
  };
  auto coro2 = [&]() -> typename TestFixture::Type {
    if constexpr (TestFixture::kIsFuture) {
      co_await On(tp);
    }
    co_await m.Lock();
    auto id = yaclib_std::this_thread::get_id();
    m.UnlockHere();
    yaclib_std::this_thread::sleep_for(1s);
    EXPECT_EQ(id, yaclib_std::this_thread::get_id());
    co_return{};
  };

  util::StopWatch sw;
  if constexpr (TestFixture::kIsFuture) {
    futures[0] = coro1();
  } else {
    futures[0] = coro1().ToFuture(tp).On(nullptr);
  }
  while (!start.load(std::memory_order_acquire)) {
    yaclib_std::this_thread::sleep_for(10ms);
  }
  for (std::size_t i = 1; i < kCoros; ++i) {
    if constexpr (TestFixture::kIsFuture) {
      futures[i] = coro2();
    } else {
      futures[i] = coro2().ToFuture(tp).On(nullptr);
    }
  }
  Wait(futures.begin(), futures.end());

  // 1s (coro1 sleep with acquired lock) + 1s (coro2 parallel sleep)
  EXPECT_LT(sw.Elapsed(), 2.5s);

  tp.HardStop();
  tp.Wait();
}

TYPED_TEST(AsyncSuite, UnlockOnBehaviour) {
#if defined(GTEST_OS_WINDOWS) && !(defined(NDEBUG) && defined(_WIN64))
  GTEST_SKIP();  // Doesn't work for Win32 or Debug, I think its probably because bad symmetric transfer implementation
  // TODO(kononovk) Try to confirm problem and localize it with ifdefs
#endif
  using namespace std::chrono_literals;
  constexpr std::size_t kThreads = 4;
  constexpr std::size_t kCoros = 4;

  yaclib::FairThreadPool tp{kThreads};
  yaclib::Mutex<> m;
  std::array<yaclib::Future<>, kCoros> futures;

  yaclib_std::atomic_bool start{false};
  yaclib_std::thread::id locked_id{};

  auto coro1 = [&]() -> typename TestFixture::Type {
    if constexpr (TestFixture::kIsFuture) {
      co_await On(tp);
    }
    co_await m.Lock();
    start.store(true, std::memory_order_release);
    locked_id = yaclib_std::this_thread::get_id();
    yaclib_std::this_thread::sleep_for(1s);
    co_await m.UnlockOn(tp);
    co_return{};
  };
  auto coro2 = [&]() -> typename TestFixture::Type {
    if constexpr (TestFixture::kIsFuture) {
      co_await On(tp);
    }
    co_await m.Lock();
#ifdef GTEST_OS_LINUX
    EXPECT_EQ(locked_id, yaclib_std::this_thread::get_id());
#endif
    co_await m.UnlockOn(tp);
    yaclib_std::this_thread::sleep_for(1s);
    co_return{};
  };

  util::StopWatch sw;
  if constexpr (TestFixture::kIsFuture) {
    futures[0] = coro1();
  } else {
    futures[0] = coro1().ToFuture(tp).On(nullptr);
  }
  while (!start.load(std::memory_order_acquire)) {
    yaclib_std::this_thread::sleep_for(10ms);
  }
  for (std::size_t i = 1; i < kCoros; ++i) {
    if constexpr (TestFixture::kIsFuture) {
      futures[i] = coro2();
    } else {
      futures[i] = coro2().ToFuture(tp).On(nullptr);
    }
  }
  Wait(futures.begin(), futures.end());

  // 1s (coro1 sleep with acquired lock) + 1s (coro2 parallel sleep)
  EXPECT_LT(sw.Elapsed(), 2.5s);

  tp.HardStop();
  tp.Wait();
}

TEST(Mutex, StickyGuard) {
#if defined(GTEST_OS_WINDOWS) && !(defined(NDEBUG) && defined(_WIN64))
  GTEST_SKIP();  // Doesn't work for Win32 or Debug, I think its probably because bad symmetric transfer implementation
  // TODO(kononovk) Try to confirm problem and localize it with ifdefs
#endif
  yaclib::Mutex<> m;
  std::uint64_t counter = 0;
  yaclib::FairThreadPool tp1{1};
  yaclib::FairThreadPool tp2{1};
  auto coro = [&](yaclib::IExecutor& executor) -> yaclib::Future<> {
    co_await On(executor);
    auto& schedulerBeforeLock = co_await yaclib::CurrentExecutor();
    EXPECT_EQ(&executor, &schedulerBeforeLock);
    for (int i = 0; i != 1000; ++i) {
      auto guard = co_await m.StickyGuard();
      YACLIB_ASSERT(guard.OwnsLock());
      auto& schedulerAfterLock = co_await yaclib::CurrentExecutor();
      counter += &schedulerBeforeLock != &schedulerAfterLock;
      YACLIB_ASSERT(guard.OwnsLock());
      yaclib_std::this_thread::sleep_for(std::chrono::nanoseconds{1});
      YACLIB_ASSERT(guard.OwnsLock());
      co_await guard.Unlock();
      YACLIB_ASSERT(!guard.OwnsLock());
      auto& schedulerAfterUnlock = co_await yaclib::CurrentExecutor();
      EXPECT_EQ(&schedulerBeforeLock, &schedulerAfterUnlock);
      YACLIB_ASSERT(!guard.OwnsLock());
    }
    co_return{};
  };
  auto f1 = coro(tp1);
  auto f2 = coro(tp2);
  Wait(f1, f2);
  EXPECT_GT(counter, 0);
  tp1.Stop();
  tp1.Wait();
  tp2.Stop();
  tp2.Wait();
}

}  // namespace
}  // namespace test
