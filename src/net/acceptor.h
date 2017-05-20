// Copyright [2016] <Malinovsky Rodion>

#pragma once

#include <boost/asio.hpp>
#include "net/alias.h"
#include "net/socket.h"

namespace cppecho {
namespace net {

class Acceptor {
 public:
  explicit Acceptor(int port);

  Socket Accept();

  void DoAccept(SocketHandlerType);

 private:
  boost::asio::ip::tcp::acceptor acceptor_;
};

}  // namespace net
}  // namespace cppecho
