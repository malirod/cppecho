// Copyright [2016] <Malinovsky Rodion>

#include "net/acceptor.h"

#include <memory>
#include <utility>

#include "core/async.h"
#include "net/util.h"
#include "util/smartptr_util.h"

using cppecho::util::make_unique;

cppecho::net::Acceptor::Acceptor(const boost::asio::ip::tcp::endpoint& endpoint)
    : acceptor_(GetNetworkServiceAccessorInstance().GetRef().GetAsioService()) {
  acceptor_.open(endpoint.protocol());
  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  acceptor_.set_option(boost::asio::ip::tcp::no_delay(true));
  acceptor_.bind(endpoint);
  acceptor_.listen();
}

cppecho::net::Acceptor::Acceptor(int port)
    : Acceptor(
          boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)) {}

cppecho::net::Acceptor::Acceptor(const std::string& ip, int port)
    : Acceptor(boost::asio::ip::tcp::endpoint(
          boost::asio::ip::address::from_string(ip), port)) {}

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
  auto socket_holder = make_unique<Socket>(std::move(Accept()));
  assert(socket_holder);
  Socket* socket_ptr = socket_holder.get();

  LOG_DEBUG("Accepted connection");

  cppecho::core::RunAsync([socket_ptr, handler] {
    std::unique_ptr<Socket> socket(socket_ptr);
    handler(std::move(socket));
  });
  // Socket ptr was captured by lambda. Prevent double desctruction.
  socket_holder.release();
}

void cppecho::net::Acceptor::Stop() {
  acceptor_.close();
}
