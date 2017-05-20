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
  Socket socket;
  DeferIo([this, &socket](IoHandlerType proceed) {
    acceptor_.async_accept(socket.socket_, proceed);
  });
  return socket;
}

void cppecho::net::Acceptor::DoAccept(SocketHandlerType handler) {
  auto holder = cppecho::util::make_unique<Socket>(std::move(Accept()));
  auto* socket_ptr = holder.get();
  cppecho::core::RunAsync([socket_ptr, handler] {
    std::unique_ptr<Socket> async_holder(socket_ptr);
    handler(*async_holder);
  });
  holder.release();
}
