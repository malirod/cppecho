// Copyright [2016] <Malinovsky Rodion>

#include "core/async_runner.h"
#include <thread>
#include <utility>
#include "util/thread_util.h"

namespace {

thread_local cppecho::core::AsyncRunner* thrd_ptr_async_runner = nullptr;

class CtorCountTag;
class DtorCountTag;

}  // namesapce

cppecho::core::AsyncRunner::AsyncRunner(IScheduler& scheduler)
    : op_state_()
    , is_events_allowed_(true)
    , scheduler_(&scheduler)
    , /*defer_handler_(),*/ coro_helper_()
    , index_(++util::GetAtomicInstance<CtorCountTag>()) {
  LOG_AUTO_TRACE();
}

cppecho::core::AsyncRunner::~AsyncRunner() {
  LOG_AUTO_TRACE();
  ++util::GetAtomicInstance<DtorCountTag>();
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
  Yield();
  // yield core
  // coro_();
  // coro_helper_ = CoroHelper(std::move(handler));
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

void cppecho::core::AsyncRunner::HandleEvents() {
  LOG_AUTO_TRACE();
  if (!is_events_allowed_ || std::uncaught_exception()) {
    return;
  }
  auto op_status = op_state_.Reset();
  if (op_status == AsyncOpStatus::Normal) {
    return;
  }
  throw AsyncOpStatusException(op_status);
}

void cppecho::core::AsyncRunner::DisableEvents() {
  HandleEvents();
  is_events_allowed_ = false;
}

void cppecho::core::AsyncRunner::EnableEvents() {
  is_events_allowed_ = true;
  HandleEvents();
}

cppecho::core::IScheduler& cppecho::core::AsyncRunner::GetScheduler() {
  return *scheduler_;
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
  return (new AsyncRunner(scheduler))->Start(std::move(handler));
}

void cppecho::core::AsyncRunner::WaitAll() {
  LOG_AUTO_TRACE();
  while (util::GetAtomicInstance<CtorCountTag>() !=
         util::GetAtomicInstance<DtorCountTag>()) {
    std::this_thread::yield();
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
      } catch (std::exception& e) {
        LOG_DEBUG("Exception in Deferer: " << e.what());
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
  return Guard(*this);
}

void cppecho::core::AsyncRunner::ProceedInternal() {
  LOG_AUTO_TRACE();
  MakeGuard()->Resume();
}

void cppecho::core::AsyncRunner::OnEnter() {
  LOG_AUTO_TRACE();
  assert(thrd_ptr_async_runner == nullptr);
  LOG_INFO("!!! setting ptr");
  thrd_ptr_async_runner = this;
  LOG_INFO("!!! thrd_ptr_async_runner: " << thrd_ptr_async_runner);
}

void cppecho::core::AsyncRunner::OnExit() noexcept {
  LOG_AUTO_TRACE();
  assert(thrd_ptr_async_runner != nullptr);
  if (defer_handler_) {
    HandlerType handler = std::move(defer_handler_);
    defer_handler_ = nullptr;
    handler();
  }
  // coro_helper_.Resume();
  thrd_ptr_async_runner = nullptr;
  delete this;
  /*
  if (defer_handler_ == nullptr) {
      delete this;
  } else {
      HandlerType handler = std::move(defer_handler_);
      defer_handler_ = nullptr;
      handler();
  }
  */
}

DECLARE_GLOBAL_GET_LOGGER("Core.AsyncRunner")

cppecho::core::AsyncRunner& cppecho::core::GetCurrentThreadAsyncRunner() {
  LOG_INFO("!!! thrd_ptr_async_runner: " << thrd_ptr_async_runner);
  assert(thrd_ptr_async_runner != nullptr &&
         "AsyncRunner is not assigned to current thread");
  return *thrd_ptr_async_runner;
}
