// Copyright [2016] <Malinovsky Rodion>

#include "core/default_scheduler_accessor.h"

cppecho::core::IoServiceAccessor&
cppecho::core::GetDefaultIoServiceAccessorInstance() {
  return cppecho::util::single<IoServiceAccessor>();
}

cppecho::core::TimeoutServiceAccessor&
cppecho::core::GetTimeoutServiceAccessorInstance() {
  return cppecho::util::single<TimeoutServiceAccessor>();
}

cppecho::core::IoSchedulerAccessor&
cppecho::core::GetDefaultSchedulerAccessorInstance() {
  return cppecho::util::single<IoSchedulerAccessor>();
}
