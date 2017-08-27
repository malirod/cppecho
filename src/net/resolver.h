// Copyright [2016] <Malinovsky Rodion>

#pragma once

#include <boost/asio.hpp>
#include <string>

namespace cppecho {
namespace net {

class Resolver {
 public:
  Resolver();

  using EndPointsType = boost::asio::ip::tcp::resolver::iterator;

  EndPointsType Resolve(const std::string& hostname, int port);

 private:
  boost::asio::ip::tcp::resolver resolver_;
};

}  // namespace net
}  // namespace cppecho
