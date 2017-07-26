// Copyright [2016] <Malinovsky Rodion>

#pragma once

#include <string>
#include <unordered_map>

#include "boost/asio.hpp"
#include "boost/signals2.hpp"

#include "net/alias.h"
#include "util/enum_util.h"
#include "util/logger.h"

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

  void Write(const BufferType& buffer);

  void Connect(const std::string& ip, int port);

  void Connect(const EndPointType& end_point);

  void Close();

  boost::optional<TcpServerIdType> GetId() const;

  void SetId(boost::optional<TcpServerIdType> id);

  using SocketOptsMap =
      std::unordered_map<SocketOpt,
                         bool,
                         cppecho::util::enum_util::EnumClassHash>;
  SocketOptsMap GetSocketOpts() const;

  using OnDataType =
      boost::signals2::signal<void(Socket& socket, const BufferType& data)>;
  using OnDataSubsriberType = OnDataType::slot_type;

  boost::signals2::connection SubscribeOnData(
      const OnDataSubsriberType& subscriber);

  using OnDisconnectedType = boost::signals2::signal<void(Socket& socket)>;
  using OnDisconnectedSubsriberType = OnDisconnectedType::slot_type;

  boost::signals2::connection SubscribeOnDisconnected(
      const OnDisconnectedSubsriberType& subscriber);

 private:
  DECLARE_GET_LOGGER("Net.Socket")

  friend class Acceptor;

  void RegisterToReceive();

  void OnSocketReceived(const boost::system::error_code& error,
                        std::size_t bytes_transferred);

  bool is_waiting_receive_ = false;

  boost::asio::ip::tcp::socket socket_;

  constexpr static const int MAX_BUFFER = 1024;
  std::array<uint8_t, MAX_BUFFER> buffer_in_;

  OnDataType on_data_;

  OnDisconnectedType on_disconnected_;

  boost::optional<TcpServerIdType> id_;
};

}  // namespace net
}  // namespace cppecho
