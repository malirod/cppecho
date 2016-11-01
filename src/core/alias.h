// Copyright [2016] <Malinovsky Rodion>

#pragma once

#include <boost/asio.hpp>

namespace cppecho {
namespace core {

using AsioServiceType = boost::asio::io_service;
using AsioServiceWorkType = boost::asio::io_service::work;

using HandlerType = std::function<void()>;

}  // namespace core
}  // namespace cppecho
