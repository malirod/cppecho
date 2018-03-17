// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include <boost/asio.hpp>
#include <string>

namespace rms {
namespace net {

/**
 * Resolves hostname into EndPoint suitable for making connection.
 */
class Resolver {
 public:
  /**
   * Creates resolver ready for work.
   */
  Resolver();

  using EndPointsType = boost::asio::ip::tcp::resolver::iterator;

  /**
   * Resolves name and port to endpoints list. Must be executed within async task. Suspends execution of the current
   * task until name is resolved.
   * @param hostname Host name to resolve.
   * @param port Port to resolve.
   * @return List of endpoints suitable for connection.
   */
  EndPointsType Resolve(const std::string& hostname, int port);

 private:
  boost::asio::ip::tcp::resolver resolver_;
};

}  // namespace net
}  // namespace rms
