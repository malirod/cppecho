// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include <boost/asio.hpp>
#include <string>

namespace rms {
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
}  // namespace rms
