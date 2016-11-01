// Copyright [2016] <Malinovsky Rodion>

#pragma once

#include "core/alias.h"

namespace cppecho {
namespace core {

class IScheduler {
 public:
  virtual ~IScheduler() = default;

  virtual void Schedule(HandlerType handler) = 0;

  virtual const char* Name() const;
};

inline const char* IScheduler::Name() const {
  return "<unknown>";
}

}  // namespace core
}  // namespace cppecho
