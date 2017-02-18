// Copyright [2016] <Malinovsky Rodion>

#include "core/coro_helper.h"
#include <utility>

namespace {

thread_local cppecho::core::CoroHelper* thrd_ptr_coro_helper = nullptr;

}  // namespace

void cppecho::core::Yield() {
  assert(IsInsideCoroutine() &&
         "Yield() must be called inside coroutine call only.");
  thrd_ptr_coro_helper->Yield();
}

bool cppecho::core::IsInsideCoroutine() {
  return thrd_ptr_coro_helper != nullptr;
}

cppecho::core::CoroHelper::CoroHelper()
    : handler_(), ptr_yield_(nullptr), coro_(MakeCoro()) {}

cppecho::core::CoroHelper::CoroHelper(HandlerType handler)
    : handler_(std::move(handler)), ptr_yield_(nullptr), coro_(MakeCoro()) {}

void cppecho::core::CoroHelper::Start(HandlerType handler) {
  handler_ = std::move(handler);
  ptr_yield_ = nullptr;
  coro_ = MakeCoro();
}

void cppecho::core::CoroHelper::Yield() {
  if (ptr_yield_) {
    (*ptr_yield_)();
  }
}

void cppecho::core::CoroHelper::Resume() {
  if (coro_) {
    coro_();
  }
}

cppecho::core::CoroHelper::CoroType::pull_type
cppecho::core::CoroHelper::MakeCoro() {
  // CTor fires coro
  return CoroType::pull_type{[this](CoroType::push_type& yield) {
    ptr_yield_ = &yield;
    Guard guard(*this);
    if (handler_) {
      handler_();
    }
  }};
}

cppecho::core::CoroHelper::Guard::Guard(CoroHelper& ptr_coro_helper)
    : ptr_co_helper_(&ptr_coro_helper) {
  LOG_AUTO_TRACE();
  thrd_ptr_coro_helper = ptr_co_helper_;
}

cppecho::core::CoroHelper::Guard::~Guard() {
  LOG_AUTO_TRACE();
  if (!thrd_ptr_coro_helper)
    return;
  thrd_ptr_coro_helper->ptr_yield_ = nullptr;
  thrd_ptr_coro_helper = nullptr;
}
