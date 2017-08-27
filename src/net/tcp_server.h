// Copyright [2017] <Malinovsky Rodion>

#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "boost/asio.hpp"
#include "boost/optional.hpp"
#include "boost/signals2.hpp"

#include "net/alias.h"
#include "util/logger.h"

namespace cppecho {
namespace net {

class Acceptor;

class TcpServer {
 public:
  ~TcpServer();

  TcpServer& operator=(const TcpServer&) = delete;
  TcpServer(const TcpServer&) = delete;

  explicit TcpServer(int max_connections);

  void Start(const EndPointType& endpoint);

  void Start(int port);

  void Start(const std::string& ip, int port);

  void Stop();

  void StopClient(TcpServerIdType id);

  boost::optional<BufferType> ReadExact(TcpServerIdType id, std::size_t size);

  boost::optional<std::pair<BufferType, ErrorType>> ReadPartial(
      TcpServerIdType id);

  boost::optional<BufferType> ReadUntil(TcpServerIdType id,
                                        const std::string& delimeter);

  void Write(TcpServerIdType id, const BufferType& buffer);

  using OnListeningType = boost::signals2::signal<void()>;
  using OnListeningSubsriberType = OnListeningType::slot_type;

  boost::signals2::connection SubscribeOnListening(
      const OnListeningSubsriberType& subscriber);

  using OnConnectedType = boost::signals2::signal<void(TcpServerIdType id)>;
  using OnConnectedSubsriberType = OnConnectedType::slot_type;

  boost::signals2::connection SubscribeOnConnected(
      const OnConnectedSubsriberType& subscriber);

  using OnDataType =
      boost::signals2::signal<void(TcpServerIdType id, const BufferType& data)>;
  using OnDataSubsriberType = OnDataType::slot_type;

  boost::signals2::connection SubscribeOnData(
      const OnDataSubsriberType& subscriber);

  using OnDisconnectedType = boost::signals2::signal<void(TcpServerIdType id)>;
  using OnDisconnectedSubsriberType = OnDisconnectedType::slot_type;

  boost::signals2::connection SubscribeOnDisconnected(
      const OnDisconnectedSubsriberType& subscriber);

  using OnStoppedType = boost::signals2::signal<void()>;
  using OnStoppedSubsriberType = OnStoppedType::slot_type;

  boost::signals2::connection SubscribeOnStopped(
      const OnStoppedSubsriberType& subscriber);

 private:
  DECLARE_GET_LOGGER("Net.TcpServer")

  void OnAccepted(std::shared_ptr<TcpSocket> accepted_socket);

  boost::optional<TcpServerIdType> GetNewConnectionIndex() const;

  std::shared_ptr<TcpSocket> GetSocket(TcpServerIdType id);

  void RaiseOnClosed();

  std::size_t GetConnectedCount() const;

  const int max_connections_;

  bool is_running_ = false;

  OnConnectedType on_connected_;

  OnListeningType on_listening_;

  OnDataType on_data_;

  OnDisconnectedType on_disconnected_;

  OnStoppedType on_stopped_;

  std::unique_ptr<cppecho::net::Acceptor> acceptor_;

  using ClientConnectionItemType = std::shared_ptr<cppecho::net::TcpSocket>;
  std::vector<ClientConnectionItemType> client_connections_;
};

}  // namespace net
}  // namespace cppecho
