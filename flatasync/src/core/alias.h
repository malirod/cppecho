// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include <boost/asio.hpp>
#include <cstdint>
#include <functional>

namespace rms {
namespace core {

using AsioServiceType = boost::asio::io_service;
using AsioServiceWorkType = boost::asio::io_service::work;
using AsioServiceStrandType = boost::asio::io_service::strand;

using HandlerType = std::function<void()>;
using ProceedHandlerType = std::function<void(HandlerType)>;

using PortType = uint32_t;

}  // namespace core
}  // namespace rms
