// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include <boost/asio.hpp>
#include "net/alias.h"
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

using rms::core::IIoService;
using rms::core::IScheduler;
using rms::util::SingleAccessor;

namespace rms {
namespace net {

struct NetworkTag;
using NetworkServiceAccessor = SingleAccessor<IIoService, NetworkTag>;
NetworkServiceAccessor& GetNetworkServiceAccessorInstance();

using NetworkIoSchedulerAccessor = rms::util::SingleAccessor<IScheduler, NetworkTag>;
NetworkIoSchedulerAccessor& GetNetworkSchedulerAccessorInstance();

ErrorType DeferIo(CallbackIoHandlerType callback);

BufferIoHandlerType BufferIoHandler(BufferType& buffer, IoHandlerType proceed);

BufferIoHandlerType BufferIoHandler(IoHandlerType proceed);

BufferType ToBuffer(const boost::asio::streambuf& streambuf);

}  // namespace net
}  // namespace rms
