// Copyright [2016] <Malinovsky Rodion>

#pragma once

#include <boost/system/error_code.hpp>
#include <functional>
#include <string>

namespace cppecho {
namespace net {

class Socket;

using BufferType = std::string;
using SocketHandlerType = std::function<void(Socket&)>;

using ErrorType = boost::system::error_code;
using IoHandlerType = std::function<void(const ErrorType&)>;
using BufferIoHandlerType = std::function<void(const ErrorType&, std::size_t)>;
using CallbackIoHandlerType = std::function<void(IoHandlerType)>;

}  // namespace net
}  // namespace cppecho
