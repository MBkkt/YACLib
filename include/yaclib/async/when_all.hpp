#pragma once

#include <yaclib/async/core.hpp>
#include <yaclib/async/future.hpp>
#include <yaclib/async/promise.hpp>
#include <yaclib/executor/executor.hpp>

#include <type_traits>
namespace yaclib::async {

template <template <typename> typename Combinator, typename T>
auto Combine(std::vector<Future<T>> inputs) {
  auto [future, combinator] = Combinator<T>::Make(inputs.size());
  for (auto& future : inputs) {
    std::move(future).Then([combinator = combinator](T result) mutable {
      combinator->Combine(std::move(result));
      combinator.reset();
    });
  }
  return std::move(future);
}

template <typename T>
class AllCombinator {
 public:
  static std::pair<Future<std::vector<T>>,
                   container::intrusive::Ptr<AllCombinator>>
  Make(size_t size) {
    auto [future, promise] = MakeContract<std::vector<T>>();
    if (size == 0) {
      std::move(promise).SetValue({});
      return {std::move(future), nullptr};
    }
    return {std::move(future),
            std::make_shared<AllCombinator>(std::move(promise), size)};
  }

  void Combine(wheels::Result<T> result) {
    if (done_.load(std::memory_order_acquire)) {
      return;
    }
    auto ticket = ticket_.fetch_add(1, std::memory_order_acq_rel);
    if (result.IsOk()) {
      results_[ticket] = std::move(result.ValueUnsafe());
    } else {
      if (!done_.exchange(true, std::memory_order_acq_rel)) {
        std::move(promise_).Set(wheels::make_result::PropagateError(result));
      }
    }
  }

  AllCombinator(Promise<std::vector<T>> promise, size_t size)
      : done_{false}, ticket_{0}, promise_{std::move(promise)} {
    results_.resize(size);
  }

  ~AllCombinator() {
    if (!done_.load(std::memory_order_acquire)) {
      std::move(promise_).SetValue(std::move(results_));
    }
  }

 private:
  alignas(64) twist::stdlike::atomic<bool> done_;
  alignas(64) twist::stdlike::atomic<size_t> ticket_;
  Promise<std::vector<T>> promise_;
  std::vector<T> results_;
};

// std::vector<Future<T>> -> Future<std::vector<T>>
// All values | first error

template <typename T>
Future<std::vector<T>> All(std::vector<Future<T>> inputs) {
  return detail::Combine<detail::AllCombinator, T>(std::move(inputs));
}

template <typename T, typename... Fs>
Future<std::vector<T>> All(Future<T>&& first, Fs&&... rest) {
  return All(wheels::ToVector(std::move(first), std::forward<Fs>(rest)...));
}

}  // namespace yaclib::async
