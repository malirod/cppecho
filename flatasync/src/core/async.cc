// Copyright [2018] <Malinovsky Rodion>

#include "core/async.h"
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/system/error_code.hpp>
#include <cassert>
#include <limits>
#include "core/async_runner.h"
#include "core/default_scheduler_accessor.h"
#include "core/iioservice.h"

DECLARE_GLOBAL_GET_LOGGER("Core.Async")

using rms::core::AsyncOpState;

AsyncOpState rms::core::RunAsync(HandlerType handler, IScheduler& scheduler) {
  return AsyncRunner::Create(std::move(handler), scheduler);
}

AsyncOpState rms::core::RunAsync(HandlerType handler) {
  auto& scheduler =
      IsCurrentThreadHasAsyncRunner() ? GetCurrentThreadScheduler() : GetDefaultSchedulerAccessorInstance();
  return RunAsync(std::move(handler), scheduler);
}

void rms::core::RunAsyncTimes(int32_t n, HandlerType handler) {
  assert(n > 0);
  RunAsync(n == 1 ? handler : [n, handler = std::move(handler)] {
    for (int32_t i = 0; i < n; ++i) {
      RunAsync(handler);
    }
  });
}

rms::core::AsyncOpStateGuard::AsyncOpStateGuard() {
  DisableEvents();
}

rms::core::AsyncOpStateGuard::~AsyncOpStateGuard() {
  EnableEvents();
}

void rms::core::SwitchTo(IScheduler& scheduler) {
  GetCurrentThreadAsyncRunner().SwitchTo(scheduler);
}

void rms::core::HandleEvents() {
  GetCurrentThreadAsyncRunner().HandleEvents();
}

void rms::core::DisableEvents() {
  GetCurrentThreadAsyncRunner().DisableEvents();
}

void rms::core::EnableEvents() {
  GetCurrentThreadAsyncRunner().EnableEvents();
}

void rms::core::WaitAll() {
  AsyncRunner::WaitAll();
}

void rms::core::Defer(HandlerType handler) {
  GetCurrentThreadAsyncRunner().Defer(std::move(handler));
}

void rms::core::DeferProceed(ProceedHandlerType proceed) {
  GetCurrentThreadAsyncRunner().DeferProceed(std::move(proceed));
}

void rms::core::RunAsyncWait(std::initializer_list<HandlerType> handlers) {
  DeferProceed([handlers](HandlerType proceed) {
    std::shared_ptr<void> proceeder(nullptr, [proceed](void*) { proceed(); });
    for (auto&& handler : handlers) {
      RunAsync([proceeder, handler] { handler(); });
    }
  });
}

rms::core::Waiter::Waiter() : proceed_(GetCurrentThreadAsyncRunner().ProceedHandler()) {
  init();
}

rms::core::Waiter::~Waiter() {
  proceed_ = nullptr;  // skip unnecessary processing
}

rms::core::Waiter& rms::core::Waiter::RunAsync(HandlerType handler) {
  auto& holder = proceeder_;
  core::RunAsync([holder, handler = std::move(handler)] { handler(); });
  return *this;
}

void rms::core::Waiter::Wait() {
  if (proceeder_.unique()) {
    LOG_TRACE("All async tasks has finished.");
    return;
  }
  Defer([this] { auto to_destroy = std::move(proceeder_); });
  init();
}

void rms::core::Waiter::init() {
  proceeder_.reset(this, [](Waiter* waiter) {
    if (waiter->proceed_ != nullptr) {
      LOG_TRACE("Wait completed. Proceeding.");
      waiter->proceed_();
    }
  });
}

std::size_t rms::core::RunAsyncAnyWait(std::initializer_list<HandlerType> handlers) {
  assert(handlers.size() >= 1 && "Handlers count must be positive");

  std::size_t index = std::numeric_limits<std::size_t>::max();
  DeferProceed([&handlers, &index](HandlerType proceed) {
    std::shared_ptr<std::atomic_int> counter = std::make_shared<std::atomic_int>();
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

rms::core::Timeout::Timeout(int ms)
    : timer_(GetTimeoutServiceAccessorInstance().GetRef().GetAsioService(), boost::posix_time::milliseconds(ms)) {
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

rms::core::Timeout::~Timeout() {
  LOG_AUTO_TRACE();
  // Use cancel_one with explicit error code to prevent potentian throw from another version of cancel_one
  boost::system::error_code error_code;
  timer_.cancel_one(error_code);
}

rms::core::IScheduler& rms::core::GetCurrentThreadScheduler() {
  return GetCurrentThreadAsyncRunner().GetScheduler();
}

rms::core::IIoService& rms::core::GetCurrentThreadIoService() {
  return GetCurrentThreadAsyncRunner().GetIoService();
}
