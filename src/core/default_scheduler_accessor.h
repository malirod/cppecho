// Copyright [2016] <Malinovsky Rodion>

#pragma once

#include "core/iioservice.h"
#include "core/ischeduler.h"
#include "util/singleton.h"

namespace cppecho {
namespace core {

using IoServiceAccessor = cppecho::util::SingleAccessor<IIoService>;
IoServiceAccessor& GetDefaultIoServiceAccessorInstance();

using IoSchedulerAccessor = cppecho::util::SingleAccessor<IScheduler>;
IoSchedulerAccessor& GetDefaultSchedulerAccessorInstance();

}  // namespace core
}  // namespace cppecho
