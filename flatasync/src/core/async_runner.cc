// Copyright [2018] <Malinovsky Rodion>

#include "core/async_runner.h"
#include <atomic>
#include <cassert>
#include <exception>
#include <functional>
#include <thread>
#include <utility>
#include "core/ischeduler.h"
#include "util/enum_util.h"
#include "util/thread_util.h"

namespace {

thread_local rms::core::AsyncRunner* thrd_ptr_async_runner = nullptr;

class RunnerCountTag;

}  // namespace

rms::core::AsyncRunner::AsyncRunner(IScheduler& scheduler)
    : is_events_allowed_(true), scheduler_(&scheduler), count_(++util::GetAtomicInstance<RunnerCountTag>()) {
  LOG_AUTO_TRACE();
  LOG_DEBUG("Created runner. Count=" << count_);
}

rms::core::AsyncRunner::~AsyncRunner() {
  LOG_AUTO_TRACE();
  const auto count = --util::GetAtomicInstance<RunnerCountTag>();
  LOG_DEBUG("Destroying runner. Count=" << count);
}

void rms::core::AsyncRunner::Proceed() {
  LOG_AUTO_TRACE();
  Schedule([this] { ProceedInternal(); });
}

rms::core::HandlerType rms::core::AsyncRunner::ProceedHandler() {
  LOG_AUTO_TRACE();
  return [this] { Proceed(); };
}

void rms::core::AsyncRunner::Defer(HandlerType handler) {
  LOG_AUTO_TRACE();
  HandleEvents();
  defer_handler_ = std::move(handler);
  LOG_TRACE("Yielding coroutine");
  Yield();
  LOG_TRACE("Resumed from Yield in coroutine");
  HandleEvents();
}

void rms::core::AsyncRunner::DeferProceed(ProceedHandlerType proceed) {
  LOG_AUTO_TRACE();
  Defer([this, proceed = std::move(proceed)] { proceed(ProceedHandler()); });
}

void rms::core::AsyncRunner::SwitchTo(IScheduler& dst) {
  LOG_AUTO_TRACE();
  if (&dst == scheduler_) {
    LOG_DEBUG("The same destination context, skipping switch to [" << dst.GetName() << "]");
    return;
  }
  LOG_DEBUG("Switching context [" << scheduler_->GetName() << "] to [" << dst.GetName() << "]");
  scheduler_ = &dst;
  Defer(ProceedHandler());
}

void rms::core::AsyncRunner::HandleEvents() const {
  LOG_AUTO_TRACE();
  // Can be called from destructor
  if (!is_events_allowed_) {
    LOG_TRACE("Skipping events handling: events are not allowed");
    return;
  }
  if (std::uncaught_exception()) {
    LOG_TRACE("Skipping events handling: uncaught exception");
    return;
  }
  auto op_status = op_state_.GetStatus();
  if (op_status == AsyncOpStatus::Normal) {
    return;
  }
  LOG_TRACE("Throwing AsyncOpStatusException: " << util::enum_util::EnumToString(op_status));
  throw AsyncOpStatusException(op_status);
}

void rms::core::AsyncRunner::DisableEvents() {
  LOG_AUTO_TRACE();
  HandleEvents();
  is_events_allowed_ = false;
}

void rms::core::AsyncRunner::EnableEvents() {
  LOG_AUTO_TRACE();
  is_events_allowed_ = true;
  HandleEvents();
}

rms::core::IScheduler& rms::core::AsyncRunner::GetScheduler() {
  return *scheduler_;
}

rms::core::IIoService& rms::core::AsyncRunner::GetIoService() {
  return util::ThreadUtil::GetCurrentThreadIoService();
}

rms::core::AsyncOpState rms::core::AsyncRunner::GetOpState() const {
  return op_state_;
}

rms::core::AsyncOpState rms::core::AsyncRunner::Create(HandlerType handler, IScheduler& scheduler) {
  LOG_AUTO_TRACE();
  // This is not a leak. Start will schedule handler and create
  // Guard for AsyncRunner's this.
  // Thus AsyncRunner will be deleted in OnExit
  return (new AsyncRunner(scheduler))->Start(std::move(handler));
}

void rms::core::AsyncRunner::WaitAll() {
  LOG_AUTO_TRACE();

  const auto is_can_break = [&]() {
    const auto counter = util::GetAtomicInstance<RunnerCountTag>().load();
    if (counter == 0) {
      LOG_TRACE("Wait done: count=" << counter);
      return true;
    }
    LOG_TRACE("Keep waiting: count=" << counter);
    return false;
  };

  while (true) {
    std::this_thread::yield();
    if (is_can_break()) {
      break;
    }
  }
}

rms::core::AsyncOpState rms::core::AsyncRunner::Start(HandlerType handler) {
  LOG_AUTO_TRACE();
  auto op_state = GetOpState();
  Schedule([handler = std::move(handler), this]() mutable {
    MakeGuard()->Start([handler = std::move(handler)] {
      LOG_DEBUG("Coroutine started");
      try {
        handler();
      } catch (const std::exception& e) {
        LOG_DEBUG("Exception in Deferrer (will not be propagated): " << e.what());
      }
      LOG_DEBUG("Coroutine ended");
    });
  });
  return op_state;
}

void rms::core::AsyncRunner::Schedule(HandlerType handler) {
  LOG_AUTO_TRACE();
  assert(scheduler_ != nullptr && "Scheduler must be set in AsyncRunner");
  LOG_TRACE("Scheduling handler on [" << scheduler_->GetName() << "]");
  scheduler_->Schedule(std::move(handler));
}

rms::core::AsyncRunner::Guard rms::core::AsyncRunner::MakeGuard() {
  LOG_AUTO_TRACE();
  LOG_TRACE("Making guard for this of AsyncRunner");
  return Guard(*this);
}

void rms::core::AsyncRunner::ProceedInternal() {
  LOG_AUTO_TRACE();
  MakeGuard()->Resume();
}

void rms::core::AsyncRunner::OnEnter() {
  LOG_AUTO_TRACE();
  assert(thrd_ptr_async_runner == nullptr);
  thrd_ptr_async_runner = this;
}

void rms::core::AsyncRunner::OnExit() noexcept {
  LOG_AUTO_TRACE();
  assert(thrd_ptr_async_runner != nullptr);
  if (defer_handler_) {
    LOG_TRACE("Processing defer_handler");
    HandlerType handler = std::move(defer_handler_);
    defer_handler_ = nullptr;
    LOG_TRACE("Calling defer handler");
    handler();
    LOG_TRACE("Exited defer handler");
  } else {
    LOG_TRACE("Deleting this");
    delete this;
  }
  LOG_TRACE("thrd_ptr_async_runner = nullptr;");
  thrd_ptr_async_runner = nullptr;
}

DECLARE_GLOBAL_GET_LOGGER("Core.AsyncRunner")

bool rms::core::IsCurrentThreadHasAsyncRunner() {
  return thrd_ptr_async_runner != nullptr;
}

rms::core::AsyncRunner& rms::core::GetCurrentThreadAsyncRunner() {
  assert(IsCurrentThreadHasAsyncRunner() && "AsyncRunner is not assigned to current thread");
  return *thrd_ptr_async_runner;
}
