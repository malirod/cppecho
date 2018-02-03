// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include <memory>

#include <boost/asio.hpp>
#include <string>  // IWYU pragma: keep
#include "net/alias.h"
#include "util/logger.h"

namespace rms {
namespace net {

class TcpSocket;

class Acceptor {
 public:
  explicit Acceptor(const boost::asio::ip::tcp::endpoint& endpoint);

  explicit Acceptor(int port);

  Acceptor(const std::string& ip, int port);

  void DoAccept(const SocketHandlerType& handler);

  void Stop();

 private:
  DECLARE_GET_LOGGER("Net.Acceptor")

  std::shared_ptr<TcpSocket> Accept();

  boost::asio::ip::tcp::acceptor acceptor_;
};

}  // namespace net
}  // namespace rms
