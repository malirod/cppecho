// Copyright [2018] <Malinovsky Rodion>

#include "core/async_op_state.h"
#include <cassert>
#include "util/enum_util.h"

using rms::core::AsyncOpStatus;
using rms::util::enum_util::EnumToString;

template <>
rms::util::enum_util::EnumStrings<AsyncOpStatus>::DataType rms::util::enum_util::EnumStrings<AsyncOpStatus>::data = {
    "Normal", "Cancelled", "Timedout"};

rms::core::AsyncOpStatusException::AsyncOpStatusException(AsyncOpStatus status)
    : std::runtime_error("Event received: " + util::enum_util::EnumToString(status)), status_(status) {}

AsyncOpStatus rms::core::AsyncOpStatusException::GetStatus() {
  return status_;
}

rms::core::AsyncOpState::AsyncOpState() : state_(std::make_shared<State>()) {}

AsyncOpStatus rms::core::AsyncOpState::Reset() {
  const auto status = GetState().status;
  GetState().status = AsyncOpStatus::Normal;
  return status;
}

bool rms::core::AsyncOpState::Cancel() {
  return SetStatus(AsyncOpStatus::Cancelled);
}

bool rms::core::AsyncOpState::Timedout() {
  return SetStatus(AsyncOpStatus::Timedout);
}

bool rms::core::AsyncOpState::SetStatus(AsyncOpStatus status) {
  auto& current_status = GetState().status;
  if (current_status != AsyncOpStatus::Normal) {
    LOG_TRACE("Skipping changing status since alredy not Normal: " << EnumToString(current_status));
    return false;
  }
  LOG_TRACE("Changing status from " << EnumToString(current_status) << " to " << EnumToString(status));
  current_status = status;
  return true;
}

rms::core::AsyncOpState::State& rms::core::AsyncOpState::GetState() {
  assert(state_ != nullptr && "Internal state is null");
  return *state_;
}

rms::core::AsyncOpStatus rms::core::AsyncOpState::GetStatus() const {
  assert(state_ != nullptr && "Internal state is null");
  return state_->status;
}
