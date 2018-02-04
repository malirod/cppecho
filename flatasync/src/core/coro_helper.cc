// Copyright [2018] <Malinovsky Rodion>

#include "core/coro_helper.h"
#include <boost/context/detail/exception.hpp>
#include <cassert>
#include <functional>
#include <utility>

namespace {

using rms::core::CoroHelper;

thread_local CoroHelper* thrd_ptr_coro_helper = nullptr;

class Guard {
 public:
  explicit Guard(CoroHelper& ptr_coro_helper);
  ~Guard();
  DECLARE_GET_LOGGER("Core.CoroHelper.Guard")
};

Guard::Guard(CoroHelper& ptr_coro_helper) {
  // ASAN doesn't like the logging below
  LOG_TRACE("Saving coro pointer in thread local storage: " << &ptr_coro_helper);
  thrd_ptr_coro_helper = &ptr_coro_helper;
}

Guard::~Guard() {
  // ASAN doesn't like the logging below
  LOG_TRACE("Clearing coro pointer in thread local storage: " << thrd_ptr_coro_helper);
  thrd_ptr_coro_helper = nullptr;
}

struct CoroInterruptedOnYield {};

}  // namespace

void rms::core::Yield() {
  assert(IsInsideCoroutine() && "Yield() must be called inside coroutine call only.");
  thrd_ptr_coro_helper->Yield();
}

bool rms::core::IsInsideCoroutine() {
  return thrd_ptr_coro_helper != nullptr;
}

rms::core::CoroHelper::CoroHelper() : ptr_yield_(nullptr) {}

rms::core::CoroHelper::~CoroHelper() {
  LOG_AUTO_TRACE();
  ptr_yield_ = nullptr;
}

rms::core::CoroHelper::CoroHelper(HandlerType handler)
    : handler_(std::move(handler)), ptr_yield_(nullptr), coro_(MakeCoroAndAutoStart()) {}

void rms::core::CoroHelper::Start(HandlerType handler) {
  LOG_AUTO_TRACE();
  handler_ = std::move(handler);
  ptr_yield_ = nullptr;
  coro_.reset();
  coro_ = MakeCoroAndAutoStart();
}

void rms::core::CoroHelper::Yield() {
  LOG_AUTO_TRACE();
  if (ptr_yield_ != nullptr) {
    LOG_TRACE("Before actual yield");
    (*ptr_yield_)();
    LOG_TRACE("After actual yield");
    // Yield has finished. There are two options:
    // 1. Pointer is still valid and we just continue exection of co-routine
    // handler
    // 2. We're destroying co-routine thus we must interrupt handler
    // cppcheck doesn't "understand", that there was an aync call, thus here is
    // no warning (if exist add : cppcheck-suppress oppositeInnerCondition)
    if (ptr_yield_ == nullptr) {
      LOG_TRACE("Interrupting coroutine");
      throw CoroInterruptedOnYield{};
    }
  }
}

void rms::core::CoroHelper::Resume() {
  LOG_AUTO_TRACE();
  if (*this) {
    auto* old = this;
    std::swap(old, thrd_ptr_coro_helper);
    (*coro_)();
    std::swap(old, thrd_ptr_coro_helper);
  }
}

rms::core::CoroHelper::CoroPullType rms::core::CoroHelper::MakeCoroAndAutoStart() {
  // ASAN doesn't like the logging below
  LOG_AUTO_TRACE();
  // CTor fires coro
  return std::make_unique<CoroType::pull_type>([this](CoroType::push_type& yield) {
    // ASAN doesn't like the logging below
    ptr_yield_ = &yield;
    LOG_TRACE("Creating guard for this coro");
    Guard guard(*this);
    if (handler_) {
      try {
        LOG_TRACE("Executing handler in coro");
        handler_();
        LOG_TRACE("Finished execution of the coro handler");
      } catch (const CoroInterruptedOnYield&) {
        // ASAN doesn't like the logging below
        LOG_TRACE("Coroutine has been interrupted.");
      }
    }
  });
}

rms::core::CoroHelper::operator bool() const {
  return coro_ && (*coro_);
}
