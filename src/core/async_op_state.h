// Copyright [2016] <Malinovsky Rodion>

#pragma once

#include <memory>
#include <stdexcept>
#include "util/logger.h"

namespace cppecho {
namespace core {

enum class AsyncOpStatus { Normal, Cancelled, Timedout };

class AsyncOpStatusException : public std::runtime_error {
 public:
  explicit AsyncOpStatusException(AsyncOpStatus status);

  AsyncOpStatus GetStatus();

 private:
  AsyncOpStatus status_;
};

class AsyncOpState {
 public:
  AsyncOpState();

  AsyncOpStatus Reset();

  bool Cancel();

  bool Timedout();

  AsyncOpStatus GetStatus() const;

 private:
  DECLARE_GET_LOGGER("Core.Async.AsyncOpState")

  struct State {
    State() : status(AsyncOpStatus::Normal) {}
    AsyncOpStatus status;
  };

  bool SetStatus(AsyncOpStatus status);

  State& GetState();

  std::shared_ptr<State> state_;
};

}  // namespace core
}  // namespace cppecho
