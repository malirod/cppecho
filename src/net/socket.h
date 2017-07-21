// Copyright [2016] <Malinovsky Rodion>

#pragma once

#include <boost/asio.hpp>
#include <string>
#include "net/alias.h"

namespace cppecho {
namespace net {

class Acceptor;

class Socket {
 public:
  using EndPointType = boost::asio::ip::tcp::endpoint;

  Socket();

  Socket(Socket&&) = default;

  Socket& operator=(Socket&&) = default;

  void Read(BufferType&);

  void PartialRead(BufferType&);

  void Write(const BufferType&);

  void Connect(const std::string& ip, int port);

  void Connect(const EndPointType& end_point);

  void Close();

 private:
  friend class Acceptor;

  boost::asio::ip::tcp::socket socket_;
};

}  // namespace net
}  // namespace cppecho
