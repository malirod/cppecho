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

/**
 * Special tag class for Network service accessors.
 */
struct NetworkTag;
using NetworkServiceAccessor = SingleAccessor<IIoService, NetworkTag>;
/**
 * Helper to get Network IO service singleton.
 * @return Reference to Network IO service singleton
 */
NetworkServiceAccessor& GetNetworkServiceAccessorInstance();

using NetworkIoSchedulerAccessor = rms::util::SingleAccessor<IScheduler, NetworkTag>;
/**
 * Helper to get Network scheduler singleton.
 * @return Reference to Network schduler singleton
 */
NetworkIoSchedulerAccessor& GetNetworkSchedulerAccessorInstance();

/**
 * Helper function for asio net functions. Allows to set operations to run after asio async call will finish.
 * @param callback Operation to run after asio net operation will finish.
 * @return Error code.
 */
ErrorType DeferIo(CallbackIoHandlerType callback);

/**
 * Helper for asio net calls.
 * @param buffer Buffer for result.
 * @param proceed Continuation
 * @return Continuation
 */
BufferIoHandlerType BufferIoHandler(BufferType& buffer, IoHandlerType proceed);

/**
 * Helper for asio net calls.
 * @param proceed Continuation
 * @return Continuation
 */
BufferIoHandlerType BufferIoHandler(IoHandlerType proceed);

/**
 * Converts (copy) asio stream buffer to internal buffer.
 * @param streambuf Input asio buffer.
 * @return Copy of the input buffer.
 */
BufferType ToBuffer(const boost::asio::streambuf& streambuf);

}  // namespace net
}  // namespace rms
