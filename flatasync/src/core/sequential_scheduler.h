// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include "core/alias.h"
#include "core/ischeduler.h"

namespace rms {
namespace core {
class IIoService;
}
}  // namespace rms

namespace rms {
namespace core {

/**
 * Scheduler which guarantees that all scheduled tasks will not run in parallel.
 */
class SequentialScheduler : public IScheduler {
 public:
  /**
   * Deletes sequential scheduler.
   */
  ~SequentialScheduler() = default;

  /**
   * Create scheduler with specific io service and name.
   * @param service IO service to be used for scheduling.
   * @param name Name of the scheduler.
   */
  explicit SequentialScheduler(IIoService& service, const char* name = "Sequential");

  /**
   * Schedule the task to be executed within context of this scheduler.
   * @param handler Task to be executed.
   */
  void Schedule(HandlerType handler) override;

  /**
   * Get name of the scheduler.
   * @return Name of the scheduler.
   */
  const char* GetName() const override;

 private:
  AsioServiceStrandType strand_;

  const char* strand_name_;
};

}  // namespace core
}  // namespace rms
