// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include "util/singleton.h"

namespace rms {
namespace core {
class IIoService;
}
}  // namespace rms
namespace rms {
namespace core {
class IScheduler;
}
}  // namespace rms

namespace rms {
namespace core {

using IoServiceAccessor = rms::util::SingleAccessor<IIoService>;
IoServiceAccessor& GetDefaultIoServiceAccessorInstance();

struct TimeoutTag;
using TimeoutServiceAccessor = rms::util::SingleAccessor<IIoService, TimeoutTag>;
TimeoutServiceAccessor& GetTimeoutServiceAccessorInstance();

using IoSchedulerAccessor = rms::util::SingleAccessor<IScheduler>;
IoSchedulerAccessor& GetDefaultSchedulerAccessorInstance();

}  // namespace core
}  // namespace rms
