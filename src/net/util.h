// Copyright [2016] <Malinovsky Rodion>

#pragma once

#include <boost/asio.hpp>
#include "core/iioservice.h"
#include "net/alias.h"
#include "util/singleton.h"

using cppecho::util::SingleAccessor;
using cppecho::core::IIoService;

namespace cppecho {
namespace net {

struct NetworkTag;
using NetworkServiceAccessor = SingleAccessor<IIoService, NetworkTag>;
NetworkServiceAccessor& GetNetworkServiceAccessorInstance();

void DeferIo(CallbackIoHandlerType callback);

BufferIoHandlerType BufferIoHandler(BufferType& buffer, IoHandlerType proceed);

BufferIoHandlerType BufferIoHandler(IoHandlerType proceed);

}  // namespace net
}  // namespace cppecho
