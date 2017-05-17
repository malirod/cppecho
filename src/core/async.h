// Copyright [2016] <Malinovsky Rodion>

#pragma once

#include <memory>
#include <utility>

#include "boost/optional.hpp"
#include "core/alias.h"
#include "core/async_op_state.h"
#include "util/logger.h"

namespace cppecho {
namespace core {

class IScheduler;

AsyncOpState RunAsync(HandlerType handler, IScheduler& scheduler);

AsyncOpState RunAsync(HandlerType handler);

void RunAsyncTimes(std::int32_t n, HandlerType handler);

void SwitchTo(IScheduler& scheduler);

int GetCurrentThreadContextSwitcherIndex();

void HandleEvents();

void DisableEvents();

void EnableEvents();

void WaitAll();

class AsyncOpStateGuard {
 public:
  AsyncOpStateGuard();

  ~AsyncOpStateGuard();
};

void Defer(HandlerType handler);

using ProceedHandlerType = std::function<void(HandlerType)>;
void DeferProceed(ProceedHandlerType proceed);

void RunAsyncWait(std::initializer_list<HandlerType> handlers);

class Waiter {
 public:
  Waiter();
  ~Waiter();

  Waiter& RunAsync(HandlerType h);

  void Wait();

 private:
  DECLARE_GET_LOGGER("Core.Async.Waiter")

  void init();

  HandlerType proceed_;

  std::shared_ptr<Waiter> proceeder_;
};

std::size_t RunAsyncAnyWait(std::initializer_list<HandlerType> handlers);

template <typename T>
boost::optional<T> RunAsyncAnyResult(
    std::initializer_list<std::function<boost::optional<T>()>> handlers) {
  using ResultType = boost::optional<T>;
  using ResultHandlerType = std::function<void(ResultType &&)>;

  struct Counter {
    explicit Counter(ResultHandlerType proceed)
        : counter_(0), proceed_(std::move(proceed)) {}

    ~Counter() {
      TryProceed(ResultType());
    }

    void TryProceed(ResultType&& result) {
      if (++counter_ == 1)
        proceed_(std::move(result));
    }

    std::atomic_int counter_;

    ResultHandlerType proceed_;
  };

  ResultType result;
  DeferProceed([&handlers, &result](HandlerType proceed) {
    std::shared_ptr<Counter> counter =
        std::make_shared<Counter>([&result, proceed](ResultType&& res) {
          result = std::move(res);
          proceed();
        });
    for (const auto& handler : handlers) {
      RunAsync([counter, &handler] {
        ResultType result = handler();
        if (result) {
          counter->TryProceed(std::move(result));
        }
      });
    }
  });
  return result;
}

class Timeout {
 public:
  explicit Timeout(int ms);
  ~Timeout();

 private:
  DECLARE_GET_LOGGER("Core.Async.Timeout")

  boost::asio::deadline_timer timer_;
};

}  // namespace core
}  // namespace cppecho
