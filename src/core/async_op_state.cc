// Copyright [2016] <Malinovsky Rodion>

#include "core/async_op_state.h"
#include <cassert>
#include "util/enum_util.h"

using cppecho::core::AsyncOpStatus;
using cppecho::util::enum_util::EnumToString;

template <>
cppecho::util::enum_util::EnumStrings<AsyncOpStatus>::DataType
    cppecho::util::enum_util::EnumStrings<AsyncOpStatus>::data = {
        "Normal", "Cancelled", "Timedout"};

cppecho::core::AsyncOpStatusException::AsyncOpStatusException(
    AsyncOpStatus status)
    : std::runtime_error("Event received: " +
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
    LOG_TRACE("Skipping changing status since alredy not Normal: "
              << EnumToString(current_status));
    return false;
  }
  LOG_TRACE("Changing status from " << EnumToString(current_status) << " to "
                                    << EnumToString(status));
  current_status = status;
  return true;
}

cppecho::core::AsyncOpState::State& cppecho::core::AsyncOpState::GetState() {
  assert(state_ != nullptr && "Internal state is null");
  return *state_;
}

cppecho::core::AsyncOpStatus cppecho::core::AsyncOpState::GetStatus() const {
  assert(state_ != nullptr && "Internal state is null");
  return state_->status;
}
