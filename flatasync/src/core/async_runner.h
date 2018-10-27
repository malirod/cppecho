// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include "core/alias.h"
#include "core/async_op_state.h"
#include "core/coro_helper.h"
#include "util/logger.h"

namespace rms {
namespace core {
class IIoService;
}
}  // namespace rms
namespace rms {
namespace core {
class IScheduler;
}
}  // namespace rms

namespace rms {
namespace core {

/**
 * Class which encapsulates work with async tasks bound to thread context.
 */
class AsyncRunner {
 public:
  /**
   * Destroys runner.
   */
  ~AsyncRunner();

  /**
   * Default ctor deleted explicitly. Make cppcheck happy.
   */
  AsyncRunner() = delete;

  /**
   * Schedule execution of the continuation.
   */
  void Proceed();

  /**
   * Return handler which allows to schedule continuation.
   * @return Callable which schedules continuation.
   */
  HandlerType ProceedHandler();

  /**
   * Schedule to execute operation after current async task will be finished. Building block for writing continuation.
   * Yields execution.
   * @param handler Operation to execute after current async task will be finished.
   */
  void Defer(HandlerType handler);

  /**
   * Wraps Defer to execute continuation after current async task is done.
   * @param proceed Operation to execute after current async task will be finished.
   */
  void DeferProceed(ProceedHandlerType proceed);

  /**
   * Switch execution context (scheduler) of the async operation to the new one. Schedule continuation to be executed
   * within new context.
   * @param dst New execution context of the async task.
   */
  void SwitchTo(IScheduler& dst);

  /**
   * Process async op state (Cancel, Timedout) and break execution if required.
   */
  void HandleEvents() const;

  /**
   * Disable events (Cancel, Timedout) processing.
   */
  void DisableEvents();

  /**
   * Enable events (Cancel, Timedout) processing and trigger events processing.
   */
  void EnableEvents();

  /**
   * Get scheduler of the async runner.
   * @return Reference to scheduler of this async runner.
   */
  IScheduler& GetScheduler();

  /**
   * Get asio io service of the async runner.
   * @return Reference to asio io service of this async runner.
   */
  IIoService& GetIoService();

  /**
   * Get OpState of the async runner.
   * @return OpState of this async runner.
   */
  AsyncOpState GetOpState() const;

  /**
   * Factory method which creates AsyncRunner with specified handler and scheduler.
   * @param handler Async operation to be executed.
   * @param scheduler Scheduler which will process async operation.
   * @return Async operation state which allows to control exception flow (Cancel, Timedout)
   */
  static AsyncOpState Create(HandlerType handler, IScheduler& scheduler);

  /**
   * Blocks current execution and waits until all async tasks will be finished.
   */
  static void WaitAll();

 private:
  DECLARE_GET_LOGGER("Core.AsyncRunner")

  explicit AsyncRunner(IScheduler& scheduler);

  class Guard {
   public:
    explicit Guard(AsyncRunner& async_runner) : async_runner_(async_runner) {
      LOG_AUTO_TRACE();
      async_runner_.OnEnter();
    }

    ~Guard() {
      LOG_AUTO_TRACE();
      async_runner_.OnExit();
    }

    CoroHelper* operator->() {
      return &async_runner_.coro_helper_;
    }

   private:
    AsyncRunner& async_runner_;
  };

  AsyncOpState Start(HandlerType handler);

  void Schedule(HandlerType handler);

  Guard MakeGuard();

  void ProceedInternal();

  void OnEnter();

  // Called from Guard dtor, must not throw
  void OnExit() noexcept;

  AsyncOpState op_state_;

  bool is_events_allowed_;

  IScheduler* scheduler_;

  HandlerType defer_handler_;

  CoroHelper coro_helper_;

  /**
   * Used track async ops creation \ deletion
   */
  const int count_;
};

/**
 * Check whether current thread has AsyncRunner attached so async facility can be used.
 * @return True if current thread has AsyncRunner attached. False otherwise.
 */
bool IsCurrentThreadHasAsyncRunner();

/**
 * Return ref to runner assigned to current thread. Assert if no AsyncRunner is attached.
 * @return AsyncRunner attached to current thread.
 */
AsyncRunner& GetCurrentThreadAsyncRunner();

}  // namespace core
}  // namespace rms
