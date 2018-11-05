// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include <memory>

#include <atomic>
#include <boost/asio/deadline_timer.hpp>
#include <boost/optional/optional.hpp>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <utility>

#include "core/alias.h"
#include "core/async_op_state.h"
#include "util/logger.h"

namespace rms {
namespace core {

class IScheduler;
class IIoService;

/**
 * Run operation asynchronously
 * @param handler Operation to run. Any callable.
 * @param scheduler Context where operation should run.
 * @return Operation status which allows to cancel operation
 */
AsyncOpState RunAsync(HandlerType handler, IScheduler& scheduler);

/**
 * Run operation asynchronously using default scheduler assigned to current thread.
 * @param handler Operation to run. Any callable.
 * @return Operation status which allows to cancel operation
 */
AsyncOpState RunAsync(HandlerType handler);

/**
 * Run operation asynchronously n times sequentally one by one using default scheduler assigned to current thread.
 * @param n Exectution count. Must be grate that 0.
 * @param handler Operation to run. Any callable.
 */
void RunAsyncTimes(int32_t n, HandlerType handler);

/**
 * Switch current execution contect to the new one. Must be called within async operation.
 * @param scheduler New context to swtich to.
 */
void SwitchTo(IScheduler& scheduler);

/**
 * Get scheduler attached to current thread. Gets information from current AsyncRunner. Assert if no runner is
 * attheched.
 * @return Ref to scheduler attchaed to current thread.
 */
IScheduler& GetCurrentThreadScheduler();

/**
 * Get ioservice attached to current thread. Gets information from current AsyncRunner. Assert if no runner is
 * attheched.
 * @return Ref to ioservice attchaed to current thread.
 */
IIoService& GetCurrentThreadIoService();

/**
 * Process pending event in AsyncRunner attached to current thread.
 */
void HandleEvents();

/**
 * Disable events processing by AsyncRunner attached to current thread.
 */
void DisableEvents();

/**
 * Enable events processing by AsyncRunner attached to current thread.
 */
void EnableEvents();

/**
 * Block current thread until all async tasks attached to this thread will finish.
 */
void WaitAll();

/**
 * Scope guard for async event. In ctor disables events and automatically enables events on exit.
 */
class AsyncOpStateGuard {
 public:
  AsyncOpStateGuard();

  ~AsyncOpStateGuard();
};

/**
 * Schedule to execute operation after current async task will be finished. Building block for writing continuation.
 * Yields execution.
 * @param handler Operation to execute after current async task will be finished.
 */
void Defer(HandlerType handler);

using ProceedHandlerType = std::function<void(HandlerType)>;
/**
 * Wraps Defer to execute continuation after current async task is done.
 * @param proceed Operation to execute after current async task will be finished.
 */
void DeferProceed(ProceedHandlerType proceed);

/**
 * Runs the list of async tasks and waits until all of them will be finished.
 * @param handlers List of async tasks.
 */
void RunAsyncWait(std::initializer_list<HandlerType> handlers);

/**
 * Helper class which allows to run async task and wait when required until task is finished.
 */
class Waiter {
 public:
  Waiter();
  ~Waiter();

  /**
   * Run async task and return ref to self to allow wait for result later.
   * @param handler Async task to run.
   * @return Ref to self to allow to wait fo result.
   */
  Waiter& RunAsync(HandlerType handler);

  /**
   * Blocks execution until assigned assync task is finished.
   */
  void Wait();

 private:
  DECLARE_GET_LOGGER("Core.Async.Waiter")

  void init();

  HandlerType proceed_;

  std::shared_ptr<Waiter> proceeder_;
};

/**
 * Runs the list of async operations, waits and returns the index of the fist finished operation.
 * @param handlers
 * @return Index of the fist finished async operation.
 */
std::size_t RunAsyncAnyWait(std::initializer_list<HandlerType> handlers);

/**
 * Runs the list of async operations, waits and returns result of the first finished operation.
 * @tparam T Type of the result
 * @param handlers List os async operations
 * @return Returns result of fist finished operation. Results of other operations are not waited.
 */
template <typename T>
boost::optional<T> RunAsyncAnyResult(std::initializer_list<std::function<boost::optional<T>()>> handlers) {
  using ResultType = boost::optional<T>;
  using ResultHandlerType = std::function<void(ResultType &&)>;

  struct Counter {
    explicit Counter(ResultHandlerType&& proceed) : counter_(0), proceed_(std::move(proceed)) {}

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
  DeferProceed([handlers = std::move(handlers), &result](HandlerType proceed) {
    std::shared_ptr<Counter> counter = std::make_shared<Counter>([&result, proceed](ResultType&& res) {
      result = std::move(res);
      proceed();
    });
    for (const auto& handler : handlers) {
      RunAsync([counter, handler = std::move(handler)] {
        ResultType result = handler();
        if (result) {
          counter->TryProceed(std::move(result));
        }
      });
    }
  });
  return result;
}

/**
 * Allows to specify timeout for async operation in which it was created.
 */
class Timeout {
 public:
  explicit Timeout(int ms);
  ~Timeout();

 private:
  DECLARE_GET_LOGGER("Core.Async.Timeout")

  boost::asio::deadline_timer timer_;
};

}  // namespace core
}  // namespace rms
