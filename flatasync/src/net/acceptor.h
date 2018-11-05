// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include <memory>

#include <boost/asio.hpp>
#include <string>
#include "net/alias.h"
#include "util/logger.h"

namespace rms {
namespace net {

class TcpSocket;

/**
 * Waits and handles incoming connections.
 */
class Acceptor {
 public:
  /**
   * Constructs acceptor basing on address and port.
   * @param endpoint Address and port to listen for new connections.
   */
  explicit Acceptor(const boost::asio::ip::tcp::endpoint& endpoint);

  /**
   * Constructs acceptor basing on port. Default address is 0.0.0.0.
   * @param port Port to listen for new connections.
   */
  explicit Acceptor(int port);

  /**
   * Constructs acceptor basing on address and port.
   * @param ip Address to listen for new connections.
   * @param port Port to listen for new connections.
   */
  Acceptor(const std::string& ip, int port);

  /**
   * Run async task to accept new connections.
   * @param handler Fired when new connection is accepted.
   */
  void DoAccept(const SocketHandlerType& handler);

  /**
   * Stop listening incoming connections.
   */
  void Stop();

 private:
  DECLARE_GET_LOGGER("Net.Acceptor")

  std::shared_ptr<TcpSocket> Accept();

  boost::asio::ip::tcp::acceptor acceptor_;
};

}  // namespace net
}  // namespace rms
