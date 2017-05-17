// Copyright [2016] <Malinovsky Rodion>

#include "core/coro_helper.h"
#include <utility>
#include "util/smartptr_util.h"

namespace {

using cppecho::core::CoroHelper;

thread_local CoroHelper* thrd_ptr_coro_helper = nullptr;

class Guard {
 public:
  explicit Guard(CoroHelper& ptr_co_helper);
  ~Guard();
  DECLARE_GET_LOGGER("Core.CoroHelper.Guard")
};

Guard::Guard(CoroHelper& ptr_coro_helper) {
  // ASAN doesn't like the logging below
  LOG_TRACE(
      "Saving coro pointer in thread local storage: " << &ptr_coro_helper);
  thrd_ptr_coro_helper = &ptr_coro_helper;
}

Guard::~Guard() {
  // ASAN doesn't like the logging below
  LOG_TRACE("Clearing coro pointer in thread local storage: "
            << thrd_ptr_coro_helper);
  thrd_ptr_coro_helper = nullptr;
}

struct CoroInterruptedOnYield {};

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
    : handler_(), ptr_yield_(nullptr), coro_() {}

cppecho::core::CoroHelper::~CoroHelper() {
  LOG_AUTO_TRACE();
  ptr_yield_ = nullptr;
}

cppecho::core::CoroHelper::CoroHelper(HandlerType handler)
    : handler_(std::move(handler))
    , ptr_yield_(nullptr)
    , coro_(MakeCoroAndAutoStart()) {}

void cppecho::core::CoroHelper::Start(HandlerType handler) {
  LOG_AUTO_TRACE();
  handler_ = std::move(handler);
  ptr_yield_ = nullptr;
  coro_.reset();
  coro_ = MakeCoroAndAutoStart();
}

void cppecho::core::CoroHelper::Yield() {
  LOG_AUTO_TRACE();
  if (ptr_yield_) {
    LOG_TRACE("Before actual yield");
    (*ptr_yield_)();
    LOG_TRACE("After actual yield");
    // Yield has finished. There are two options:
    // 1. Pointer is still valid and we just continue exection of co-routine
    // handler
    // 2. We're destroying co-routine thus we must interrupt handler
    // cppcheck doesn't "understand", that there was an aync call, thus here is
    // no warning (if exist add : cppcheck-suppress oppositeInnerCondition)
    if (!ptr_yield_) {
      LOG_TRACE("Interrupting coroutine");
      throw CoroInterruptedOnYield{};
    }
  }
}

void cppecho::core::CoroHelper::Resume() {
  LOG_AUTO_TRACE();
  if (*this) {
    auto* old = this;
    std::swap(old, thrd_ptr_coro_helper);
    (*coro_)();
    std::swap(old, thrd_ptr_coro_helper);
  }
}

cppecho::core::CoroHelper::CoroPullType
cppecho::core::CoroHelper::MakeCoroAndAutoStart() {
  // ASAN doesn't like the logging below
  LOG_AUTO_TRACE();
  // CTor fires coro
  return util::make_unique<CoroType::pull_type>(
      [this](CoroType::push_type& yield) {
        // ASAN doesn't like the logging below
        LOG_AUTO_TRACE();
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

cppecho::core::CoroHelper::operator bool() const {
  return coro_ && (*coro_);
}
