// Copyright [2016] <Malinovsky Rodion>

#pragma once

#include <memory>
#include <string>

#include "boost/asio.hpp"
#include "net/alias.h"
#include "net/tcp_socket.h"
#include "util/logger.h"

namespace cppecho {
namespace net {

class Acceptor {
 public:
  explicit Acceptor(const boost::asio::ip::tcp::endpoint& endpoint);

  explicit Acceptor(int port);

  Acceptor(const std::string& ip, int port);

  void DoAccept(SocketHandlerType);

  void Stop();

 private:
  DECLARE_GET_LOGGER("Net.Acceptor")

  std::shared_ptr<TcpSocket> Accept();

  boost::asio::ip::tcp::acceptor acceptor_;
};

}  // namespace net
}  // namespace cppecho
