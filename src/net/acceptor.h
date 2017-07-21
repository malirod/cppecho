// Copyright [2016] <Malinovsky Rodion>

#pragma once

#include <boost/asio.hpp>
#include "net/alias.h"
#include "net/socket.h"
#include "util/logger.h"

namespace cppecho {
namespace net {

class Acceptor {
 public:
  explicit Acceptor(int port);

  void DoAccept(SocketHandlerType);

 private:
  DECLARE_GET_LOGGER("Net.Acceptor")

  Socket Accept();

  boost::asio::ip::tcp::acceptor acceptor_;
};

}  // namespace net
}  // namespace cppecho
