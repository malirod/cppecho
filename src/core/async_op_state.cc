// Copyright [2016] <Malinovsky Rodion>

#include "core/async_op_state.h"
#include <cassert>
#include "util/enum_util.h"

using cppecho::core::AsyncOpStatus;

template <>
cppecho::util::enum_util::EnumStrings<AsyncOpStatus>::DataType
    cppecho::util::enum_util::EnumStrings<AsyncOpStatus>::data = {
        "Normal", "Cancelled", "Timedout"};

cppecho::core::AsyncOpStatusException::AsyncOpStatusException(
    AsyncOpStatus status)
    : std::runtime_error("Journey event received: " +
                         util::enum_util::EnumToString(status))
    , status_(status) {}

AsyncOpStatus cppecho::core::AsyncOpStatusException::GetStatus() {
  return status_;
}

cppecho::core::AsyncOpState::AsyncOpState()
    : state_(std::make_shared<State>()) {}

AsyncOpStatus cppecho::core::AsyncOpState::Reset() {
  const auto status = GetState().status;
  GetState().status = AsyncOpStatus::Normal;
  return status;
}

bool cppecho::core::AsyncOpState::Cancel() {
  return SetStatus(AsyncOpStatus::Cancelled);
}

bool cppecho::core::AsyncOpState::Timedout() {
  return SetStatus(AsyncOpStatus::Timedout);
}

bool cppecho::core::AsyncOpState::SetStatus(AsyncOpStatus status) {
  auto& current_status = GetState().status;
  if (current_status != AsyncOpStatus::Normal) {
    return false;
  }
  current_status = status;
  return true;
}

cppecho::core::AsyncOpState::State& cppecho::core::AsyncOpState::GetState() {
  assert(state_ != nullptr && "Internal state is null");
  return *state_;
}
