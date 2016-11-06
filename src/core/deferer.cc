// Copyright [2016] <Malinovsky Rodion>

#include "core/deferer.h"
#include <utility>

cppecho::core::Deferer::Deferer(HandlerType handler)
    : handler_(std::move(handler)), coro_(MakeCoro()) {}

void cppecho::core::Deferer::Start(HandlerType handler) {
  handler_ = std::move(handler);
  coro_ = MakeCoro();
}

void cppecho::core::Deferer::Resume() {
  if (coro_) {
    if (coro_) {
      coro_();
    }
  }
}

cppecho::core::Deferer::CoroType::pull_type cppecho::core::Deferer::MakeCoro() {
  // CTor fires coro
  return CoroType::pull_type{[this](CoroType::push_type& yield) {
    yield();
    if (handler_) {
      handler_();
    }
  }};
}
