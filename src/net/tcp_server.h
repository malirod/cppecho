// Copyright [2017] <Malinovsky Rodion>

#pragma once

#include <memory>
#include <string>
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

  void Start(const boost::asio::ip::tcp::endpoint& endpoint);

  void Start(int port);

  void Start(const std::string& ip, int port);

  void Stop();

  bool Close(TcpServerIdType id);

  boost::optional<BufferType> ReadExact(TcpServerIdType id, std::size_t size);

  boost::optional<BufferType> ReadPartial(TcpServerIdType id);

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

 private:
  DECLARE_GET_LOGGER("Net.TcpServer")

  void OnAccepted(std::unique_ptr<Socket> accepted_socket);

  boost::optional<TcpServerIdType> GetNewConnectionIndex() const;

  Socket* GetSocket(TcpServerIdType id);

  const int max_connections_;

  std::atomic_bool stopped_{false};

  OnConnectedType on_connected_;

  OnListeningType on_listening_;

  OnDataType on_data_;

  OnDisconnectedType on_disconnected_;

  std::unique_ptr<cppecho::net::Acceptor> acceptor_;

  std::vector<std::unique_ptr<cppecho::net::Socket>> client_connections_;
};

}  // namespace net
}  // namespace cppecho
