// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include <memory>
#include <stdexcept>
#include "util/logger.h"

namespace rms {
namespace core {

/**
 * Enumeration which holds async operation possible statuses.
 */
enum class AsyncOpStatus { Normal, Cancelled, Timedout };

/**
 * Exception used to break exception of the async operation.
 */
class AsyncOpStatusException : public std::runtime_error {
 public:
  /**
   * Construct exception with initial value
   * @param status Initial status
   */
  explicit AsyncOpStatusException(AsyncOpStatus status);

  /**
   * Gets assigned async operation status.
   * @return Async operation status.
   */
  AsyncOpStatus GetStatus();

 private:
  AsyncOpStatus status_;
};

/**
 * Class allows to control async operation: Cancel, Timeout etc.
 */
class AsyncOpState {
 public:
  AsyncOpState();

  /**
   * Set async operation status to Normal.
   * @return Old status
   */
  AsyncOpStatus Reset();

  /**
   * Set async operation status to Canceled
   * @return True if status has been changed, false otherwise.
   */
  bool Cancel();

  /**
   * Set async operation status to Timedout
   * @return True if status has been changed, false otherwise.
   */
  bool Timedout();

  /**
   * Get current status
   * @return Current status.
   */
  AsyncOpStatus GetStatus() const;

 private:
  DECLARE_GET_LOGGER("Core.Async.AsyncOpState")

  struct State {
    AsyncOpStatus status = AsyncOpStatus::Normal;
  };

  bool SetStatus(AsyncOpStatus status);

  State& GetState();

  std::shared_ptr<State> state_;
};

}  // namespace core
}  // namespace rms
