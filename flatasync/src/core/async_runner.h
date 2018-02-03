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

class AsyncRunner {
 public:
  ~AsyncRunner();

  void Proceed();

  HandlerType ProceedHandler();

  void Defer(HandlerType handler);

  void DeferProceed(ProceedHandlerType proceed);

  void SwitchTo(IScheduler& dst);

  void HandleEvents() const;

  void DisableEvents();

  void EnableEvents();

  IScheduler& GetScheduler();

  IIoService& GetIoService();

  int GetIndex() const;

  AsyncOpState GetOpState() const;

  static AsyncOpState Create(HandlerType handler, IScheduler& scheduler);

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

  const int index_;

  const int count_;
};

bool IsCurrentThreadHasAsyncRunner();

AsyncRunner& GetCurrentThreadAsyncRunner();

}  // namespace core
}  // namespace rms
