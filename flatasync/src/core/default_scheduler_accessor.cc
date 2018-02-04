// Copyright [2018] <Malinovsky Rodion>

#include "core/default_scheduler_accessor.h"

rms::core::IoServiceAccessor& rms::core::GetDefaultIoServiceAccessorInstance() {
  return rms::util::single<IoServiceAccessor>();
}

rms::core::TimeoutServiceAccessor& rms::core::GetTimeoutServiceAccessorInstance() {
  return rms::util::single<TimeoutServiceAccessor>();
}

rms::core::IoSchedulerAccessor& rms::core::GetDefaultSchedulerAccessorInstance() {
  return rms::util::single<IoSchedulerAccessor>();
}
