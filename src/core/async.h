// Copyright [2016] <Malinovsky Rodion>

#pragma once

#include "core/alias.h"
#include "core/async_op_state.h"

namespace cppecho {
namespace core {

class IScheduler;

AsyncOpState GoAsync(HandlerType handler, IScheduler& scheduler);
AsyncOpState GoAsync(HandlerType handler);
void GoAsyncN(std::int32_t n, HandlerType handler);

class StateGuard {
 public:
  StateGuard();
  ~StateGuard();
};

}  // namespace core
}  // namespace cppecho
