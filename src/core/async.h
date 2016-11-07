// Copyright [2016] <Malinovsky Rodion>

#pragma once

#include "core/alias.h"
#include "core/async_op_state.h"

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

}  // namespace core
}  // namespace cppecho
