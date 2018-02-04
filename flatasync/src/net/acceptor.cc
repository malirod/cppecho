// Copyright [2018] <Malinovsky Rodion>

#include "net/acceptor.h"

#include <memory>
#include <utility>

#include "core/async.h"
#include "net/util.h"

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <cassert>
#include "core/iioservice.h"
#include "net/tcp_socket.h"

using rms::core::GetCurrentThreadIoService;
using rms::net::TcpSocket;

rms::net::Acceptor::Acceptor(const EndPointType& endpoint) : acceptor_(GetCurrentThreadIoService().GetAsioService()) {
  // TODO(malirod): move this logic to Start out of CTor
  LOG_DEBUG("Opening socket for listening");
  boost::system::error_code error;
  acceptor_.open(endpoint.protocol(), error);
  if (error.value() != boost::system::errc::success) {
    // TODO(malirod): raise OnError event
    LOG_DEBUG("Error during open: " << error.message());
  }
  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  acceptor_.set_option(boost::asio::ip::tcp::no_delay(true));
  acceptor_.bind(endpoint, error);
  if (error.value() != boost::system::errc::success) {
    // TODO(malirod): raise OnError event
    LOG_DEBUG("Error during bind: " << error.message());
  }
  acceptor_.listen(boost::asio::socket_base::max_connections, error);
  if (error.value() != boost::system::errc::success) {
    // TODO(malirod): raise OnError event
    LOG_DEBUG("Error during listen: " << error.message());
  }
  LOG_DEBUG("Listening");
}

rms::net::Acceptor::Acceptor(int port) : Acceptor(EndPointType(boost::asio::ip::tcp::v4(), port)) {}

rms::net::Acceptor::Acceptor(const std::string& ip, int port)
    : Acceptor(EndPointType(boost::asio::ip::address::from_string(ip), port)) {}

std::shared_ptr<TcpSocket> rms::net::Acceptor::Accept() {
  LOG_AUTO_TRACE();
  AsioTcpSocketType asio_socket(GetCurrentThreadIoService().GetAsioService());
  DeferIo([this, &asio_socket](IoHandlerType proceed) {
    LOG_DEBUG("Calling acceptor_.async_accept");
    acceptor_.async_accept(asio_socket, proceed);
    LOG_DEBUG("End of call acceptor_.async_accept");
  });
  return TcpSocket::Create(std::move(asio_socket));
}

void rms::net::Acceptor::DoAccept(const SocketHandlerType& handler) {
  LOG_AUTO_TRACE();
  auto socket = Accept();
  assert(socket);

  if (!acceptor_.is_open()) {
    LOG_DEBUG("Acceptor is closed after Accept has finished.");
    return;
  }
  LOG_DEBUG("Accepted connection");

  rms::core::RunAsync([socket, handler] { handler(socket); });
}

void rms::net::Acceptor::Stop() {
  LOG_DEBUG("Closing acceptor");
  boost::system::error_code error;
  acceptor_.cancel(error);
  if (error.value() != boost::system::errc::success)
    LOG_DEBUG("Acceptor canceling error: " << error.message());
  acceptor_.close(error);
  if (error.value() != boost::system::errc::success)
    LOG_DEBUG("Acceptor closing error: " << error.message());
}
