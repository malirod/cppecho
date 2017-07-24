// Copyright [2016] <Malinovsky Rodion>

#pragma once

#include <string>
#include <unordered_map>

#include "boost/asio.hpp"

#include "net/alias.h"
#include "util/enum_util.h"

namespace cppecho {
namespace net {

class Acceptor;

class Socket {
 public:
  using EndPointType = boost::asio::ip::tcp::endpoint;

  enum class SocketOpt { NoDelay };

  Socket();

  Socket(Socket&&) = default;

  Socket& operator=(Socket&&) = default;

  BufferType ReadExact(std::size_t size);

  BufferType ReadPartial();

  BufferType ReadUntil(const std::string& delimeter);

  void Write(const BufferType&);

  void Connect(const std::string& ip, int port);

  void Connect(const EndPointType& end_point);

  void Close();

  using SocketOptsMap =
      std::unordered_map<SocketOpt,
                         bool,
                         cppecho::util::enum_util::EnumClassHash>;
  SocketOptsMap GetSocketOpts() const;

 private:
  friend class Acceptor;

  boost::asio::ip::tcp::socket socket_;
};

}  // namespace net
}  // namespace cppecho
