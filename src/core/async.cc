// Copyright [2016] <Malinovsky Rodion>

#include "core/async.h"

using cppecho::core::AsyncOpState;

AsyncOpState cppecho::core::GoAsync(HandlerType handler,
                                    IScheduler& scheduler) {
  (void)(handler);
  (void)(scheduler);
  return AsyncOpState();
  // return Journey::create(std::move(handler), scheduler);
}

AsyncOpState cppecho::core::GoAsync(HandlerType handler) {
  (void)(handler);
  return AsyncOpState();
  // return Journey::create(std::move(handler), scheduler<DefaultTag>());
}

void cppecho::core::GoAsyncN(std::int32_t n, HandlerType handler) {
  assert(n > 0);
  GoAsync(n == 1 ? handler : [n, handler] {
    for (std::int32_t i = 0; i < n; ++i) {
      GoAsync(handler);
    }
  });
}

cppecho::core::StateGuard::StateGuard() {
  // disableEvents();
}

cppecho::core::StateGuard::~StateGuard() {
  // enableEvents();
}
