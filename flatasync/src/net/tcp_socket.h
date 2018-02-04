// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include <memory>
#include <unordered_map>
#include <utility>

#include <boost/signals2.hpp>

#include <stdint.h>
#include <array>
#include <string>  // IWYU pragma: keep
#include "net/alias.h"
#include "util/enum_util.h"
#include "util/logger.h"

namespace rms {
namespace core {

class IScheduler;

}  // namespace core
}  // namespace rms

namespace rms {
namespace net {

class TcpSocket : public std::enable_shared_from_this<TcpSocket> {
 private:
  struct PrivateKey {};

 public:
  enum class SocketOpt { NoDelay };

  explicit TcpSocket(const PrivateKey& /*unused*/);

  TcpSocket(const PrivateKey& /*unused*/, AsioTcpSocketType socket);

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

  using SocketOptsMap = std::unordered_map<SocketOpt, bool, rms::util::enum_util::EnumClassHash>;
  SocketOptsMap GetSocketOpts() const;

  using OnDataType = boost::signals2::signal<void(TcpSocket& socket, const BufferType& data)>;
  using OnDataSubsriberType = OnDataType::slot_type;

  boost::signals2::connection SubscribeOnData(const OnDataSubsriberType& subscriber);

  using OnDisconnectedType = boost::signals2::signal<void(TcpSocket& socket)>;
  using OnDisconnectedSubsriberType = OnDisconnectedType::slot_type;

  boost::signals2::connection SubscribeOnDisconnected(const OnDisconnectedSubsriberType& subscriber);

 private:
  DECLARE_GET_LOGGER("Net.Socket")

  AsioTcpSocketType socket_;

  constexpr static const int MAX_BUFFER = 1024;
  std::array<uint8_t, MAX_BUFFER> buffer_in_ = {};

  OnDataType on_data_;

  OnDisconnectedType on_disconnected_;

  TcpServerIdType id_ = 0u;

  rms::core::IScheduler& scheduler_;

  bool stopped_ = false;
};

}  // namespace net
}  // namespace rms
