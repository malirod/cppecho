// Copyright [2016] <Malinovsky Rodion>

#include "core/coro_helper.h"
#include <utility>
// #include "util/smartptr_util.h"

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
    : handler_()
    , ptr_yield_(nullptr)
    // , coro_()
    , coro_(MakeCoro()) {
  LOG_AUTO_TRACE();
}

cppecho::core::CoroHelper::CoroHelper(HandlerType handler)
    : handler_(std::move(handler)), ptr_yield_(nullptr), coro_(MakeCoro()) {
  LOG_AUTO_TRACE();
}

void cppecho::core::CoroHelper::Start(HandlerType handler) {
  // using std::swap;
  LOG_AUTO_TRACE();
  handler_ = std::move(handler);
  ptr_yield_ = nullptr;
  // LOG_INFO("!!!! 1 ptr_yield_="<<ptr_yield_);
  // auto new_coro = MakeCoro();
  // auto new_coro =
  // coro_ = MakeCoro();
  // coro_.reset();
  coro_ = MakeCoro();
  // swap(coro_, new_coro);
  // LOG_INFO("!!!! 2 ptr_yield_="<<ptr_yield_);
}

void cppecho::core::CoroHelper::Yield() {
  LOG_AUTO_TRACE();
  if (ptr_yield_) {
    (*ptr_yield_)();
  }
}

void cppecho::core::CoroHelper::Resume() {
  LOG_AUTO_TRACE();
  // if (coro_ && *coro_) {
  if (coro_) {
    coro_();
    // (*coro_)();
  }
}

cppecho::core::CoroHelper::CoroType::pull_type
cppecho::core::CoroHelper::MakeCoro() {
  // std::unique_ptr<cppecho::core::CoroHelper::CoroType::pull_type>
  // cppecho::core::CoroHelper::MakeCoro() {

  // CTor fires coro
  return CoroType::pull_type{[this](CoroType::push_type& yield) {
    // return util::make_unique<CoroType::pull_type>([this](CoroType::push_type&
    // yield) {
    LOG_INFO("!!!! 3 ptr_yield_=" << &yield);

    ptr_yield_ = &yield;
    Guard guard(*this);
    if (handler_) {
      handler_();
      // }});
    }
  }};
}

cppecho::core::CoroHelper::Guard::Guard(CoroHelper& ptr_coro_helper)
    : ptr_co_helper_(&ptr_coro_helper) {
  thrd_ptr_coro_helper = ptr_co_helper_;
}

cppecho::core::CoroHelper::Guard::~Guard() {
  thrd_ptr_coro_helper->ptr_yield_ = nullptr;
  thrd_ptr_coro_helper = nullptr;
}
