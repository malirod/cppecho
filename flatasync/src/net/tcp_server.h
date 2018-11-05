// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <boost/optional.hpp>
#include <boost/signals2.hpp>

#include "net/alias.h"
#include "util/logger.h"

namespace rms {
namespace net {

class Acceptor;
class TcpSocket;

/**
 * Tcp Server. Allows to listen for incoming connection and communicate with them. Works asynchronously. Should be used
 * within async tasks.
 */
class TcpServer {
 public:
  /**
   * Desctructor. Proper shutdown should be done explicitly.
   */
  ~TcpServer();

  TcpServer& operator=(const TcpServer&) = delete;
  TcpServer(const TcpServer&) = delete;

  /**
   * Create Tcp server and limit amount of connections.
   * @param max_connections Maximum count of connection server can hold. If max count is reached new connections will be
   * rejected.
   */
  explicit TcpServer(int max_connections);

  /**
   * Start listening on specified address.
   * @param endpoint Address to listen on.
   */
  void Start(const EndPointType& endpoint);

  /**
   * Start listening on specified port and default address.
   * @param port Port to listen on.
   */
  void Start(int port);

  /**
   * Start listening on specific address and port.
   * @param ip Local address to listen on.
   * @param port Port to listen on.
   */
  void Start(const std::string& ip, int port);

  /**
   * Initiate shutdown sequence.
   */
  void Stop();

  /**
   * Disconnect specific client connection.
   * @param id Identifier of the client to deal with.
   */
  void StopClient(TcpServerIdType id);

  /**
   * Read exact count of bytes from specific client.
   * @param id Identifier of the client to deal with.
   * @param size Amount of bytes to read.
   * @return Buffer which holds result.
   */
  boost::optional<BufferType> ReadExact(TcpServerIdType id, std::size_t size);

  /**
   * Read all available data at the moment.
   * @param id Identifier of the client to deal with.
   * @return Buffer which holds result.
   */
  boost::optional<std::pair<BufferType, ErrorType>> ReadPartial(TcpServerIdType id);

  /**
   * Read data from specific socket until special delimiter is received. Buffer might contain data after delimiter.
   * @param id Identifier of the client to deal with.
   * @param delimiter String to wait for.
   * @return Buffer which holds result.
   */
  boost::optional<BufferType> ReadUntil(TcpServerIdType id, const std::string& delimiter);

  /**
   * Sends data to specific client. Makes async
   * @param id Identifier of the client to deal with.
   * @param buffer Buffer which holds data to send.
   */
  void Write(TcpServerIdType id, const BufferType& buffer);

  using OnListeningType = boost::signals2::signal<void()>;
  using OnListeningSubscriberType = OnListeningType::slot_type;

  /**
   * Register to OnListening which will be fired when server socket is up and running. Server should be started first.
   * @param subscriber Slot which will be fired when server socket is ready and listening.
   * @return Connection of the signal to slot.
   */
  boost::signals2::connection SubscribeOnListening(const OnListeningSubscriberType& subscriber);

  using OnConnectedType = boost::signals2::signal<void(TcpServerIdType id)>;
  using OnConnectedSubscriberType = OnConnectedType::slot_type;

  /**
   * Register to OnConnected which will be fired when new client is connected. Server should be started first.
   * @param subscriber Slot which will be fired when new client is connected.
   * @return Connection of the signal to slot.
   */
  boost::signals2::connection SubscribeOnConnected(const OnConnectedSubscriberType& subscriber);

  using OnDataType = boost::signals2::signal<void(TcpServerIdType id, const BufferType& data)>;
  using OnDataSubscriberType = OnDataType::slot_type;

  /**
   * Register to OnData which will be fired when data is received from client. Server should be started first.
   * @param subscriber Slot which will be fired when data is received from client.
   * @return Connection of the signal to slot.
   */
  boost::signals2::connection SubscribeOnData(const OnDataSubscriberType& subscriber);

  using OnDisconnectedType = boost::signals2::signal<void(TcpServerIdType id)>;
  using OnDisconnectedSubscriberType = OnDisconnectedType::slot_type;

  /**
   * Register to OnDisconnected which will be fired when client has disconnected. Server should be started first.
   * @param subscriber Slot which will be fired when client has disconnected.
   * @return Connection of the signal to slot.
   */
  boost::signals2::connection SubscribeOnDisconnected(const OnDisconnectedSubscriberType& subscriber);

  using OnStoppedType = boost::signals2::signal<void()>;
  using OnStoppedSubscriberType = OnStoppedType::slot_type;

  /**
   * Register to OnStopped which will be fired when server has stopped. Server should be started first.
   * @param subscriber Slot which will be fired when server has stopped.
   * @return Connection of the signal to slot.
   */
  boost::signals2::connection SubscribeOnStopped(const OnStoppedSubscriberType& subscriber);

 private:
  DECLARE_GET_LOGGER("Net.TcpServer")

  void OnAccepted(std::shared_ptr<TcpSocket> accepted_socket);

  boost::optional<TcpServerIdType> GetNewConnectionIndex() const;

  std::shared_ptr<TcpSocket> GetSocket(TcpServerIdType id);

  void RaiseOnClosed();

  std::size_t GetConnectedCount() const;

  bool is_running_ = false;

  OnConnectedType on_connected_;

  OnListeningType on_listening_;

  OnDataType on_data_;

  OnDisconnectedType on_disconnected_;

  OnStoppedType on_stopped_;

  std::unique_ptr<rms::net::Acceptor> acceptor_;

  using ClientConnectionItemType = std::shared_ptr<rms::net::TcpSocket>;
  std::vector<ClientConnectionItemType> client_connections_;
};

}  // namespace net
}  // namespace rms
