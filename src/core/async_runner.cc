// Copyright [2016] <Malinovsky Rodion>

#include "core/async_runner.h"
#include <mutex>
#include <thread>
#include <utility>
#include "util/enum_util.h"
#include "util/singleton.h"
#include "util/thread_util.h"

namespace {

thread_local cppecho::core::AsyncRunner* thrd_ptr_async_runner = nullptr;

class RunnerIndexTag;
class RunnerCountTag;

}  // namesapce

cppecho::core::AsyncRunner::AsyncRunner(IScheduler& scheduler)
    : op_state_()
    , is_events_allowed_(true)
    , scheduler_(&scheduler)
    , coro_helper_()
    , index_(++util::GetAtomicInstance<RunnerIndexTag>())
    , count_(++util::GetAtomicInstance<RunnerCountTag>()) {
  LOG_AUTO_TRACE();
  LOG_DEBUG("Created runner with index=" << index_ << ", count=" << count_);
}

cppecho::core::AsyncRunner::~AsyncRunner() {
  LOG_AUTO_TRACE();
  const auto count = --util::GetAtomicInstance<RunnerCountTag>();
  LOG_DEBUG("Destroying runner with index=" << index_ << ". count=" << count);
}

void cppecho::core::AsyncRunner::Proceed() {
  LOG_AUTO_TRACE();
  Schedule([this] { ProceedInternal(); });
}

cppecho::core::HandlerType cppecho::core::AsyncRunner::ProceedHandler() {
  LOG_AUTO_TRACE();
  return [this] { Proceed(); };
}

void cppecho::core::AsyncRunner::Defer(HandlerType handler) {
  LOG_AUTO_TRACE();
  HandleEvents();
  defer_handler_ = std::move(handler);
  LOG_TRACE("Yielding coroutine");
  Yield();
  LOG_TRACE("Resumed from Yield in coroutine");
  HandleEvents();
}

void cppecho::core::AsyncRunner::DeferProceed(ProceedHandlerType proceed) {
  LOG_AUTO_TRACE();
  Defer([this, proceed] { proceed(ProceedHandler()); });
}

void cppecho::core::AsyncRunner::SwitchTo(IScheduler& dst) {
  LOG_AUTO_TRACE();
  if (&dst == scheduler_) {
    LOG_DEBUG("The same destination context, skipping switch to ["
              << dst.GetName()
              << "]");
    return;
  }
  LOG_DEBUG("Switching context [" << scheduler_->GetName() << "] to ["
                                  << dst.GetName()
                                  << "]");
  scheduler_ = &dst;
  Defer(ProceedHandler());
}

void cppecho::core::AsyncRunner::HandleEvents() const {
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
  LOG_TRACE("Throwing AsyncOpStatusException: "
            << util::enum_util::EnumToString(op_status));
  throw AsyncOpStatusException(op_status);
}

void cppecho::core::AsyncRunner::DisableEvents() {
  LOG_AUTO_TRACE();
  HandleEvents();
  is_events_allowed_ = false;
}

void cppecho::core::AsyncRunner::EnableEvents() {
  LOG_AUTO_TRACE();
  is_events_allowed_ = true;
  HandleEvents();
}

cppecho::core::IScheduler& cppecho::core::AsyncRunner::GetScheduler() {
  return *scheduler_;
}

cppecho::core::IIoService& cppecho::core::AsyncRunner::GetIoService() {
  return util::ThreadUtil::GetCurrentThreadIoSerivce();
}

int cppecho::core::AsyncRunner::GetIndex() const {
  return index_;
}

cppecho::core::AsyncOpState cppecho::core::AsyncRunner::GetOpState() const {
  return op_state_;
}

cppecho::core::AsyncOpState cppecho::core::AsyncRunner::Create(
    HandlerType handler, IScheduler& scheduler) {
  LOG_AUTO_TRACE();
  // This is not a leak. Start will schedule handler and create
  // Guard for AsyncRunner's this.
  // Thus AsyncRunner will be deleted in OnExit
  return (new AsyncRunner(scheduler))->Start(std::move(handler));
}

void cppecho::core::AsyncRunner::WaitAll() {
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

cppecho::core::AsyncOpState cppecho::core::AsyncRunner::Start(
    HandlerType handler) {
  LOG_AUTO_TRACE();
  auto op_state = GetOpState();
  Schedule([handler, this] {
    LOG_AUTO_TRACE();
    MakeGuard()->Start([handler] {
      LOG_DEBUG("Coroutine started");
      try {
        handler();
      } catch (const std::exception& e) {
        LOG_ERROR("Exception in Deferer: " << e.what());
      }
      LOG_DEBUG("Coroutine ended");
    });
  });
  return op_state;
}

void cppecho::core::AsyncRunner::Schedule(HandlerType handler) {
  LOG_AUTO_TRACE();
  assert(scheduler_ != nullptr && "Scheduler must be set in AsyncRunner");
  LOG_TRACE("Scheduling handler on [" << scheduler_->GetName() << "]");
  scheduler_->Schedule(std::move(handler));
}

cppecho::core::AsyncRunner::Guard cppecho::core::AsyncRunner::MakeGuard() {
  LOG_AUTO_TRACE();
  LOG_TRACE("Making guard for this of AsyncRunner");
  return Guard(*this);
}

void cppecho::core::AsyncRunner::ProceedInternal() {
  LOG_AUTO_TRACE();
  MakeGuard()->Resume();
}

void cppecho::core::AsyncRunner::OnEnter() {
  LOG_AUTO_TRACE();
  assert(thrd_ptr_async_runner == nullptr);
  thrd_ptr_async_runner = this;
}

void cppecho::core::AsyncRunner::OnExit() noexcept {
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

bool cppecho::core::IsCurrentThreadHasAsyncRunner() {
  return thrd_ptr_async_runner != nullptr;
}

cppecho::core::AsyncRunner& cppecho::core::GetCurrentThreadAsyncRunner() {
  assert(IsCurrentThreadHasAsyncRunner() &&
         "AsyncRunner is not assigned to current thread");
  return *thrd_ptr_async_runner;
}
