#include <yaclib/async/run.hpp>
#include <yaclib/executor/thread_pool.hpp>

#include <iostream>
#include <string>

#include <gtest/gtest.h>

namespace {

using namespace yaclib;
using namespace std::chrono_literals;

template <typename FutureType, typename ErrorType>
void ErrorsCheck(ErrorType expected) {
  static_assert(std::is_same_v<ErrorType, std::exception_ptr> || std::is_same_v<ErrorType, std::error_code>);
  auto [f, p] = async::MakeContract<FutureType>();
  EXPECT_FALSE(f.IsReady());
  std::move(p).Set(expected);
  EXPECT_TRUE(f.IsReady());
  auto result = std::move(f).Get();
  if constexpr (std::is_same_v<ErrorType, std::exception_ptr>) {
    EXPECT_EQ(result.State(), util::ResultState::Exception);
    EXPECT_EQ(std::move(result).Exception(), expected);
  } else {
    EXPECT_EQ(result.State(), util::ResultState::Error);
    EXPECT_EQ(std::move(result).Error(), expected);
  }
}

// FutureType must be default constructible
template <typename FutureType>
void ValueCheck() {
  auto [f, p] = async::MakeContract<FutureType>();
  EXPECT_FALSE(f.IsReady());
  if constexpr (std::is_void_v<FutureType>) {
    std::move(p).Set();
  } else {
    std::move(p).Set(FutureType{});
  }
  EXPECT_TRUE(f.IsReady());
  auto result = std::move(f).Get();
  EXPECT_EQ(result.State(), util::ResultState::Value);
  if constexpr (!std::is_void_v<FutureType>) {
    EXPECT_EQ(std::move(result).Value(), FutureType{});
  } else {
    using ResultType = decltype(std::move(result).Value());
    using ExpectedType = decltype(util::Result<FutureType>::Default().Value());
    static_assert(std::is_same_v<ResultType, ExpectedType>);
  }
}

void AsyncGetResult(int num_threads) {
  static const std::string kMessage = "Hello, world!";
  auto tp = executor::MakeThreadPool(num_threads);

  auto [f, p] = async::MakeContract<std::string>();
  tp->Execute([p = std::move(p)]() mutable {
    std::move(p).Set(kMessage);
  });
  EXPECT_EQ(std::move(f).Get().Ok(), kMessage);
  tp->HardStop();
  tp->Wait();
}

TEST(JustWorks, Value) {
  ValueCheck<double>();
}

TEST(JustWorks, ErrorCode) {
  ErrorsCheck<double>(std::error_code{});
}

TEST(JustWorks, Exception) {
  ErrorsCheck<double>(std::make_exception_ptr(std::runtime_error{""}));
}

TEST(JustWorks, Run) {
  bool called = false;
  async::Run(executor::MakeInlineExecutor(), [&] {
    called = true;
  });
  EXPECT_TRUE(called);
}

TEST(JustWorks, RunException) {
  auto result = async::Run(executor::MakeInlineExecutor(),
                           [] {
                             throw std::runtime_error{""};
                           })
                    .Then([] {
                      return 1;
                    })
                    .Get();
  EXPECT_THROW(std::move(result).Ok(), std::runtime_error);
}

TEST(JustWorks, RunError) {
  auto result = async::Run(executor::MakeInlineExecutor(),
                           [] {
                             return util::Result<void>{std::error_code{}};
                           })
                    .Then([] {
                      return 1;
                    })
                    .Get();
  EXPECT_THROW(std::move(result).Ok(), util::ResultError);
}

TEST(JustWorks, VoidThen) {
  auto f = async::Run(executor::MakeInlineExecutor(), [] {
           }).Then([] {
    return util::Result<int>(1);
  });
  EXPECT_EQ(std::move(f).Get().Ok(), 1);
}

TEST(VoidJustWorks, Simple) {
  ValueCheck<void>();
}

TEST(VoidJustWorks, ErrorCode) {
  ErrorsCheck<void>(std::error_code{});
}

TEST(VoidJustWorks, Exception) {
  ErrorsCheck<void>(std::make_exception_ptr(std::runtime_error{""}));
}

TEST(JustWorks, AsyncGetResult) {
  AsyncGetResult(1);
  AsyncGetResult(4);
}

TEST(JustWorks, AsyncRun) {
  auto tp = executor::MakeThreadPool(3);

  {
    auto good = [&] {
      EXPECT_EQ(executor::CurrentThreadPool(), tp);
      return std::string{"Hello!"};
    };
    EXPECT_EQ(async::Run(tp, good).Get().Ok(), "Hello!");
  }

  {
    auto bad = [&] {
      EXPECT_EQ(executor::CurrentThreadPool(), tp);
      throw std::logic_error("test");
      return int{};
    };

    EXPECT_THROW(async::Run(tp, bad).Get().Ok(), std::logic_error);
  }

  tp->Stop();
}

TEST(JustWorks, Promise) {
  auto tp = executor::MakeThreadPool(2);
  auto [f, p] = async::MakeContract<void>();

  int i = 0;
  auto g = std::move(f).Then(tp, [&] {
    i = 1;
  });
  std::move(p).Set();
  std::move(g).Get();
  EXPECT_EQ(i, 1);
  tp->Stop();
  tp->Wait();
}

// Subscribe
TEST(Subscribe, ValueSimple) {
  auto [f, p] = async::MakeContract<int>();

  std::move(p).Set(17);

  bool called = false;

  std::move(f).Subscribe([&called](int result) {
    EXPECT_EQ(result, 17);
    called = true;
  });
  EXPECT_TRUE(called);
}

TEST(Subscribe, ResultSimple) {
  auto [f, p] = async::MakeContract<int>();

  std::move(p).Set(17);

  bool called = false;

  std::move(f).Subscribe([&called](util::Result<int> result) {
    EXPECT_EQ(std::move(result).Ok(), 17);
    called = true;
  });

  EXPECT_TRUE(called);
}

TEST(Subscribe, ExceptionSimple) {
  auto [f, p] = async::MakeContract<int>();

  auto result = util::Result<int>{};
  result.Set(std::make_exception_ptr(std::runtime_error{""}));

  std::move(p).Set(std::move(result));

  bool called = false;

  std::move(f).Subscribe([&called](util::Result<int> result) {
    called = true;
    EXPECT_THROW(std::move(result).Ok(), std::runtime_error);
  });

  EXPECT_TRUE(called);
}

TEST(Subscribe, AsyncSimple) {
  auto tp = executor::MakeThreadPool(1);

  auto [f, p] = async::MakeContract<std::string>();

  std::atomic<bool> called{false};

  std::move(f).Subscribe([&](util::Result<std::string> result) {
    EXPECT_EQ(executor::CurrentThreadPool(), tp);
    EXPECT_EQ(std::move(result).Ok(), "Hello!");
    called.store(true, std::memory_order_release);
  });

  EXPECT_FALSE(called.load(std::memory_order_acquire));

  tp->Execute([p = std::move(p)]() mutable {
    std::move(p).Set("Hello!");
  });

  tp->Stop();
  tp->Wait();

  EXPECT_TRUE(called.load(std::memory_order_relaxed));
}

TEST(Subscribe, SubscribeVia) {
  auto tp = executor::MakeThreadPool(1);

  auto [f, p] = async::MakeContract<int>();

  std::move(p).Set(17);

  std::atomic<bool> called = false;

  auto callback = [&called](util::Result<int> result) mutable {
    EXPECT_EQ(std::move(result).Ok(), 17);
    called.store(true);
  };

  // Schedule to thread pool immediately
  std::move(f).Subscribe(tp, callback);

  tp->SoftStop();
  tp->Wait();

  EXPECT_TRUE(called);
}

TEST(Subscribe, SubscribeVia2) {
  auto tp1 = executor::MakeThreadPool(1);
  auto tp2 = executor::MakeThreadPool(1);

  auto [f, p] = async::MakeContract<int>();

  std::atomic<bool> called = false;

  auto callback = [&called](util::Result<int> result) mutable {
    EXPECT_EQ(std::move(result).Ok(), 42);
    called.store(true);
  };

  std::move(f).Subscribe(tp2, callback);

  tp1->Execute([p = std::move(p)]() mutable {
    std::move(p).Set(42);
  });

  tp1->SoftStop();
  tp1->Wait();
  tp2->SoftStop();
  tp2->Wait();

  EXPECT_TRUE(called);
}

TEST(Simple, Then) {
  auto [f, p] = async::MakeContract<int>();

  auto g = std::move(f).Then([](int v) {
    return v * 2 + 1;
  });
  std::move(p).Set(3);
  EXPECT_EQ(std::move(g).Get().Ok(), 7);
}

TEST(Simple, ThenThreadPool) {
  auto tp = executor::MakeThreadPool(4);

  auto compute = [] {
    return 42;
  };

  auto process = [](int v) -> int {
    return v + 1;
  };

  async::Future<int> f1 = async::Run(tp, compute);

  async::Future<int> f2 = std::move(f1).Then(process);

  EXPECT_EQ(std::move(f2).Get().Ok(), 43);

  tp->Stop();
  tp->Wait();
}

class Unit {};

TEST(Simple, ThenVia) {
  auto tp = executor::MakeThreadPool(2);
  auto [f, p] = async::MakeContract<Unit>();

  auto g = std::move(f).Then(tp, [&](Unit) {
    EXPECT_EQ(executor::CurrentThreadPool(), tp);
    return 42;
  });

  // Launch
  std::move(p).Set(Unit{});

  EXPECT_EQ(std::move(g).Get().Ok(), 42);

  tp->Stop();
  tp->Wait();
}

TEST(Simple, ThenExecutor) {
  auto tp = executor::MakeThreadPool(1);
  {
    std::cerr << " ";
    async::Promise<Unit> p;
    {
      auto f{p.MakeFuture()};
      {
        auto g = std::move(f).Then(tp, [](Unit) {
          return 42;
        });
        tp->SoftStop();
        tp->Wait();
        std::cerr << " ";
      }
      std::cerr << " ";
    }
    std::cerr << " ";
  }
  std::cerr << " ";
  tp->Stop();
  tp->Wait();
}

TEST(Pipeline, Simple) {
  auto tp = executor::MakeThreadPool(1);

  // Pipeline stages:

  auto first = [] {
    return 42;
  };

  auto second = [](int value) {
    return value * 2;
  };

  auto third = [](int value) {
    return value + 1;
  };

  auto f = async::Run(tp, first).Then(second).Then(third);

  EXPECT_EQ(std::move(f).Get().Ok(), 42 * 2 + 1);

  tp->Stop();
  tp->Wait();
}

TEST(Errors, Simple1) {
  auto tp = executor::MakeThreadPool(4);

  // Pipeline stages:

  auto first = [] {
    return 1;
  };

  auto second = [](int v) {
    std::cout << v << " x 2" << std::endl;
    return v * 2;
  };

  auto third = [](int v) {
    std::cout << v << " + 1" << std::endl;
    return v + 1;
  };

  auto error_handler = [](std::exception_ptr) -> int {
    return 42;
  };

  auto last = [&](int v) -> int {
    EXPECT_EQ(executor::CurrentThreadPool(), tp);
    return v + 11;
  };

  auto f = async::Run(tp, first).Then(second).Then(third).Then(error_handler).Then(last);

  EXPECT_EQ(std::move(f).Get().Ok(), 14);

  tp->Stop();
  tp->Wait();
}

TEST(Errors, Simple2) {
  auto tp = executor::MakeThreadPool(1);

  // Pipeline stages:

  auto first = [] {
    throw std::runtime_error("first");
    return int{};
  };

  auto second = [](int v) {
    std::cout << v << " x 2" << std::endl;
    return v * 2;
  };

  auto third = [](int v) {
    std::cout << v << " + 1" << std::endl;
    return v + 1;
  };

  auto error_handler = [](std::exception_ptr) -> int {
    return 42;
  };

  auto last = [&](int v) {
    EXPECT_EQ(executor::CurrentThreadPool(), tp);
    return v + 11;
  };

  auto pipeline = async::Run(tp, first).Then(second).Then(third).Then(error_handler).Then(last);

  EXPECT_EQ(std::move(pipeline).Get().Value(), 53);
  tp->Stop();
  tp->Wait();
}

class Calculator {
 public:
  Calculator(executor::IThreadPoolPtr tp) : _tp(tp) {
  }

  async::Future<int> Increment(int value) {
    return async::Run(_tp, [value] {
      return value + 1;
    });
  }

  async::Future<int> Double(int value) {
    return async::Run(_tp, [value] {
      return value * 2;
    });
  }

 private:
  executor::IThreadPoolPtr _tp;
};

TEST(AsyncThen, Simple) {
  auto tp = executor::MakeThreadPool(4);

  Calculator calculator(tp);

  auto pipeline = calculator.Increment(1)
                      .Then([&](int value) {
                        return calculator.Double(value);
                      })
                      .Then([&](int value) {
                        return calculator.Increment(value);
                      });
  EXPECT_EQ(std::move(pipeline).Get().Value(), 5);

  tp->SoftStop();
  tp->Wait();
}

TEST(Pipeline, Simple2) {
  auto tp1 = executor::MakeThreadPool(2);
  auto tp2 = executor::MakeThreadPool(3);

  auto [f, p] = async::MakeContract<std::string>();

  auto make_stage = [](int index, executor::IThreadPoolPtr current_executor) {
    return [index, current_executor](std::string path) {
      EXPECT_EQ(executor::CurrentThreadPool(), current_executor);
      std::cout << "At stage " << index << std::endl;
      return path + "->" + std::to_string(index);
    };
  };

  auto almost_there = std::move(f).Then(tp1, make_stage(1, tp1)).Then(make_stage(2, tp1)).Then(tp2, make_stage(3, tp2));

  std::move(p).Set("start");
  std::this_thread::sleep_for(100ms);

  auto finally = std::move(almost_there).Then(make_stage(4, tp2)).Then(tp1, make_stage(5, tp1));

  EXPECT_EQ(std::move(finally).Get().Ok(), "start->1->2->3->4->5");
  tp1->Stop();
  tp1->Wait();
  tp2->Stop();
  tp2->Wait();
}

}  // namespace