// Copyright [2016] <Malinovsky Rodion>

#include "core/async.h"
#include <limits>
#include <utility>
#include "core/async_runner.h"
#include "core/default_scheduler_accessor.h"
#include "core/ischeduler.h"
#include "util/singleton.h"

DECLARE_GLOBAL_GET_LOGGER("Core.Async")

using cppecho::core::AsyncOpState;

AsyncOpState cppecho::core::RunAsync(HandlerType handler,
                                     IScheduler& scheduler) {
  return AsyncRunner::Create(std::move(handler), scheduler);
}

AsyncOpState cppecho::core::RunAsync(HandlerType handler) {
  return RunAsync(std::move(handler), GetDefaultSchedulerAccessorInstance());
}

void cppecho::core::RunAsyncTimes(std::int32_t n, HandlerType handler) {
  assert(n > 0);
  RunAsync(n == 1 ? handler : [n, handler] {
    for (std::int32_t i = 0; i < n; ++i) {
      RunAsync(handler);
    }
  });
}

cppecho::core::AsyncOpStateGuard::AsyncOpStateGuard() {
  DisableEvents();
}

cppecho::core::AsyncOpStateGuard::~AsyncOpStateGuard() {
  EnableEvents();
}

void cppecho::core::SwitchTo(IScheduler& scheduler) {
  GetCurrentThreadAsyncRunner().SwitchTo(scheduler);
}

int cppecho::core::GetCurrentThreadContextSwitcherIndex() {
  return GetCurrentThreadAsyncRunner().GetIndex();
}

void cppecho::core::HandleEvents() {
  GetCurrentThreadAsyncRunner().HandleEvents();
}

void cppecho::core::DisableEvents() {
  GetCurrentThreadAsyncRunner().DisableEvents();
}

void cppecho::core::EnableEvents() {
  GetCurrentThreadAsyncRunner().EnableEvents();
}

void cppecho::core::WaitAll() {
  AsyncRunner::WaitAll();
}

void cppecho::core::Defer(HandlerType handler) {
  GetCurrentThreadAsyncRunner().Defer(handler);
}

void cppecho::core::DeferProceed(ProceedHandlerType proceed) {
  GetCurrentThreadAsyncRunner().DeferProceed(proceed);
}

void cppecho::core::RunAsyncWait(std::initializer_list<HandlerType> handlers) {
  DeferProceed([&handlers](HandlerType proceed) {
    std::shared_ptr<void> proceeder(nullptr, [proceed](void*) { proceed(); });
    for (const auto& handler : handlers) {
      RunAsync([proceeder, &handler] { handler(); });
    }
  });
}

cppecho::core::Waiter::Waiter() {
  proceed_ = GetCurrentThreadAsyncRunner().ProceedHandler();
  init();
}

cppecho::core::Waiter::~Waiter() {
  proceed_ = nullptr;  // skip unnecessary processing
}

cppecho::core::Waiter& cppecho::core::Waiter::RunAsync(HandlerType handler) {
  auto& holder = proceeder_;
  cppecho::core::RunAsync([holder, handler] { handler(); });
  return *this;
}

void cppecho::core::Waiter::Wait() {
  if (proceeder_.unique()) {
    LOG_TRACE("All async tasks has finished.");
    return;
  }
  Defer([this] { auto to_destroy = std::move(proceeder_); });
  init();
}

void cppecho::core::Waiter::init() {
  proceeder_.reset(this, [](Waiter* waiter) {
    if (waiter->proceed_ != nullptr) {
      LOG_TRACE("Wait completed. Proceeding.");
      waiter->proceed_();
    }
  });
}

std::size_t cppecho::core::RunAsyncAnyWait(
    std::initializer_list<HandlerType> handlers) {
  assert(handlers.size() >= 1 && "Handlers count must be positive");

  std::size_t index = std::numeric_limits<std::size_t>::max();
  DeferProceed([&handlers, &index](HandlerType proceed) {
    std::shared_ptr<std::atomic_int> counter =
        std::make_shared<std::atomic_int>();
    std::size_t i = 0u;
    for (const auto& handler : handlers) {
      RunAsync([counter, proceed, &handler, i, &index] {
        handler();
        if (++(*counter) == 1) {
          index = i;
          proceed();
        }
      });
      ++i;
    }
  });
  assert(index < handlers.size() && "Incorrect index returned");
  return index;
}

cppecho::core::Timeout::Timeout(int ms)
    : timer_(GetTimeoutServiceAccessorInstance().GetRef().GetAsioService(),
             boost::posix_time::milliseconds(ms)) {
  LOG_AUTO_TRACE();
  auto op_state = GetCurrentThreadAsyncRunner().GetOpState();
  timer_.async_wait([op_state](const boost::system::error_code& error) mutable {
    // mutable, because we change captured state
    LOG_TRACE("Handling timeout. Status: " << error.message());
    if (!error) {
      LOG_TRACE("Operation timedout");
      op_state.Timedout();
    }
  });
}

cppecho::core::Timeout::~Timeout() {
  LOG_AUTO_TRACE();
  timer_.cancel_one();
  HandleEvents();
}
