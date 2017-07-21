// Copyright [2016] <Malinovsky Rodion>

#include "net/acceptor.h"
#include <memory>
#include <utility>
#include "core/async.h"
#include "net/util.h"
#include "util/smartptr_util.h"

cppecho::net::Acceptor::Acceptor(int port)
    : acceptor_(
          GetNetworkServiceAccessorInstance().GetRef().GetAsioService(),
          boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)) {}

cppecho::net::Socket cppecho::net::Acceptor::Accept() {
  LOG_AUTO_TRACE();
  Socket socket;
  DeferIo([this, &socket](IoHandlerType proceed) {
    LOG_DEBUG("Calling acceptor_.async_accept");
    acceptor_.async_accept(socket.socket_, proceed);
  });
  return socket;
}

void cppecho::net::Acceptor::DoAccept(SocketHandlerType handler) {
  LOG_AUTO_TRACE();
  // Socket should live after this method will finish due to async call
  // Async framework requires copyable lambdas thus movig accepted socket
  // to lambda will not work.
  auto socket_holder = std::make_shared<Socket>(std::move(Accept()));
  assert(socket_holder);
  LOG_DEBUG("Accepted connection");

  cppecho::core::RunAsync(
      [socket_holder, handler] { handler(*socket_holder); });
}
