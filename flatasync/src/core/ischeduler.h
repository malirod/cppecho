// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include "core/alias.h"

namespace rms {
namespace core {

class IScheduler {
 public:
  virtual ~IScheduler() = default;

  virtual void Schedule(HandlerType handler) = 0;

  virtual const char* GetName() const;
};

inline const char* IScheduler::GetName() const {
  return "<unknown>";
}

}  // namespace core
}  // namespace rms
