// Copyright [2016] <Malinovsky Rodion>

#include "core/async.h"
#include <utility>
#include "core/async_runner.h"
#include "core/default_scheduler_accessor.h"
#include "core/ischeduler.h"

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
