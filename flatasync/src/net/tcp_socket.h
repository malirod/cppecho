// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include <memory>
#include <unordered_map>
#include <utility>

#include <boost/signals2.hpp>

#include <stdint.h>
#include <array>
#include <string>
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

/**
 * Tcp Socket based on boost asio.
 */
class TcpSocket : public std::enable_shared_from_this<TcpSocket> {
 private:
  /**
   * Helper struct which allows to prevent construction via constructors. Force to use factory. Used Passkey idiom.
   * Simple friendship doesn't work since factory returns shared_ptr.
   */
  struct PrivateKey {};

 public:
  /**
   * Socket option(s).
   */
  enum class SocketOpt { NoDelay };

  /**
   * Private constructor (Passkey idiom)
   */
  explicit TcpSocket(const PrivateKey& /*unused*/);

  /**
   * Private constructor (Passkey idiom)
   * @param socket boost asio socket to be used by this object. Takes ownership.
   */
  TcpSocket(const PrivateKey& /*unused*/, AsioTcpSocketType socket);

  /**
   * Factory which creates default TcpSocket.
   * @return Tcp socket.
   */
  static std::shared_ptr<TcpSocket> Create();

  /**
   * Factory which creates TcpSocket based on specific boost asio socket.
   * @param socket boost asio socket to be used by this object. Takes ownership.
   * @return Tcp socket.
   */
  static std::shared_ptr<TcpSocket> Create(AsioTcpSocketType socket);

  /**
   * Destroy socket.
   */
  ~TcpSocket();

  /**
   * Reads exact amount of bytes from socket. Should be called within async task. Suspends execution until result is
   * received.
   * @param size Amount of bytes to read from socket.
   * @return Buffer with received data.
   */
  BufferType ReadExact(std::size_t size);

  /**
   * Read some data available in socket. Should be called within async task. Suspends execution until result is
   * received.
   * @return Pair of buffer and error code (result of async operation)
   */
  std::pair<BufferType, ErrorType> ReadPartial();

  /**
   * Read from socket until delimiter reached. Buffer can contain data after delimiter. Should be called within async
   * task. Suspends execution until result is received.
   * @param delimeter String to wait for.
   * @return Buffer with received data.
   */
  BufferType ReadUntil(const std::string& delimiter);

  /**
   * Write buffer to socket. Should be called within async task. Suspends execution until result is received.
   * @param buffer Data to be sent.
   */
  void Write(const BufferType& buffer);

  /**
   * Establish connection to remote peer. Should be called within async task. Suspends execution until result is
   * received.
   * @param ip Address to connect.
   * @param port Port to connect.
   */
  void Connect(const std::string& ip, int port);

  /**
   * Establish connection to remote peer. Should be called within async task. Suspends execution until result is
   * received.
   * @param end_point Address to connect.
   */
  void Connect(const EndPointType& end_point);

  /**
   * Start listening for incoming data. Doesn't block. Fires OnData signal when data received and keep listening.
   */
  void Start();

  /**
   * Close socket and cleanup.
   */
  void Stop();

  /**
   * Get socket id.
   * @return Socket id.
   */
  TcpServerIdType GetId() const;

  /**
   * Set socket id.
   * @param id Socket id.
   */
  void SetId(TcpServerIdType id);

  using SocketOptsMap = std::unordered_map<SocketOpt, bool, rms::util::enum_util::EnumClassHash>;

  /**
   * Get socket options.
   * @return Socket options.
   */
  SocketOptsMap GetSocketOpts() const;

  using OnDataType = boost::signals2::signal<void(TcpSocket& socket, const BufferType& data)>;
  using OnDataSubscriberType = OnDataType::slot_type;

  /**
   * Register to OnData event which will be fired when data is received.
   * @param subscriber Slot which will be fired when data is received.
   * @return Connection of the signal to slot.
   */
  boost::signals2::connection SubscribeOnData(const OnDataSubscriberType& subscriber);

  using OnDisconnectedType = boost::signals2::signal<void(TcpSocket& socket)>;
  using OnDisconnectedSubscriberType = OnDisconnectedType::slot_type;

  /**
   * Register to OnDisconnected which will be fired when socket is disconnected.
   * @param subscriber Slot which will be fired when socket is disconnected.
   * @return Connection of the signal to slot.
   */
  boost::signals2::connection SubscribeOnDisconnected(const OnDisconnectedSubscriberType& subscriber);

 private:
  DECLARE_GET_LOGGER("Net.Socket")

  AsioTcpSocketType socket_;

  OnDataType on_data_;

  OnDisconnectedType on_disconnected_;

  TcpServerIdType id_ = 0u;

  rms::core::IScheduler& scheduler_;

  bool stopped_ = false;
};

}  // namespace net
}  // namespace rms
