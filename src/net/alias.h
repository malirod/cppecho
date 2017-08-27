// Copyright [2016] <Malinovsky Rodion>

#pragma once

#include <functional>
#include <memory>
#include <string>

#include "boost/asio.hpp"
#include "boost/system/error_code.hpp"

namespace cppecho {
namespace net {

class TcpSocket;

using BufferType = std::string;
using TcpServerIdType = std::size_t;
using SocketHandlerType = std::function<void(std::shared_ptr<TcpSocket>)>;
using EndPointType = boost::asio::ip::tcp::endpoint;
using AsioTcpSocketType = boost::asio::ip::tcp::socket;

using ErrorType = boost::system::error_code;
using IoHandlerType = std::function<void(const ErrorType&)>;
using BufferIoHandlerType = std::function<void(const ErrorType&, std::size_t)>;
using CallbackIoHandlerType = std::function<void(IoHandlerType)>;

}  // namespace net
}  // namespace cppecho
