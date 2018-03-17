// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include "core/alias.h"

namespace rms {
namespace core {

/**
 * Interface of the scheduler for async tasks execution.
 */
class IScheduler {
 public:
  /**
   * Destructor of the scheduler.
   */
  virtual ~IScheduler() = default;

  /**
   * Schedule the task to be executed within context of this scheduler.
   * @param handler Task to be executed.
   */
  virtual void Schedule(HandlerType handler) = 0;

  /**
   * Get name of the scheduler.
   * @return Name of the scheduler.
   */
  virtual const char* GetName() const;
};

inline const char* IScheduler::GetName() const {
  return "<unknown>";
}

}  // namespace core
}  // namespace rms
