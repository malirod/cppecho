// Copyright [2016] <Malinovsky Rodion>

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "boost/asio.hpp"
#include "boost/signals2.hpp"

#include "net/alias.h"
#include "util/enum_util.h"
#include "util/logger.h"

namespace cppecho {
namespace core {

class IScheduler;

}  // namespace core
}  // namespace cppecho

namespace cppecho {
namespace net {

class TcpServer;
class Acceptor;

class TcpSocket : public std::enable_shared_from_this<TcpSocket> {
 private:
  struct PrivateKey {};

 public:
  enum class SocketOpt { NoDelay };

  explicit TcpSocket(const PrivateKey& private_key);

  TcpSocket(const PrivateKey& private_key, AsioTcpSocketType socket);

  static std::shared_ptr<TcpSocket> Create();

  static std::shared_ptr<TcpSocket> Create(AsioTcpSocketType socket);

  ~TcpSocket();

  BufferType ReadExact(std::size_t size);

  std::pair<BufferType, ErrorType> ReadPartial();

  BufferType ReadUntil(const std::string& delimeter);

  void Write(const BufferType& buffer);

  void Connect(const std::string& ip, int port);

  void Connect(const EndPointType& end_point);

  void Start();

  void Stop();

  TcpServerIdType GetId() const;

  void SetId(TcpServerIdType id);

  using SocketOptsMap =
      std::unordered_map<SocketOpt,
                         bool,
                         cppecho::util::enum_util::EnumClassHash>;
  SocketOptsMap GetSocketOpts() const;

  using OnDataType =
      boost::signals2::signal<void(TcpSocket& socket, const BufferType& data)>;
  using OnDataSubsriberType = OnDataType::slot_type;

  boost::signals2::connection SubscribeOnData(
      const OnDataSubsriberType& subscriber);

  using OnDisconnectedType = boost::signals2::signal<void(TcpSocket& socket)>;
  using OnDisconnectedSubsriberType = OnDisconnectedType::slot_type;

  boost::signals2::connection SubscribeOnDisconnected(
      const OnDisconnectedSubsriberType& subscriber);

 private:
  DECLARE_GET_LOGGER("Net.Socket")

  AsioTcpSocketType socket_;

  constexpr static const int MAX_BUFFER = 1024;
  std::array<uint8_t, MAX_BUFFER> buffer_in_;

  OnDataType on_data_;

  OnDisconnectedType on_disconnected_;

  TcpServerIdType id_ = 0u;

  cppecho::core::IScheduler& scheduler_;

  bool stopped_ = false;
};

}  // namespace net
}  // namespace cppecho
