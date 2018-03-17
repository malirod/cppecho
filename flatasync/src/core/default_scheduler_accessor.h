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
/**
 * Gets reference to default asio io service (singleton).
 * @return Reference to asio io service.
 */
IoServiceAccessor& GetDefaultIoServiceAccessorInstance();

struct TimeoutTag;
using TimeoutServiceAccessor = rms::util::SingleAccessor<IIoService, TimeoutTag>;
/**
 * Gets reference to asio io service (singleton) which purpose is to track timeouts.
 * @return Reference to asio io service which tracks timeouts.
 */
TimeoutServiceAccessor& GetTimeoutServiceAccessorInstance();

using IoSchedulerAccessor = rms::util::SingleAccessor<IScheduler>;
/**
 * Gets reference to default scheduler (singleton).
 * @return Reference to scheduler.
 */
IoSchedulerAccessor& GetDefaultSchedulerAccessorInstance();

}  // namespace core
}  // namespace rms
