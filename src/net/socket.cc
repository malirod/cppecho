// Copyright [2016] <Malinovsky Rodion>

#include "net/socket.h"

#include <utility>

#include "core/async.h"
#include "net/util.h"

using cppecho::net::TcpServerIdType;
using cppecho::core::RunAsync;

cppecho::net::Socket::Socket()
    : socket_(GetNetworkServiceAccessorInstance().GetRef().GetAsioService()) {}

cppecho::net::BufferType cppecho::net::Socket::ReadExact(std::size_t size) {
  BufferType buffer(size, 0);
  DeferIo([&](IoHandlerType proceed) {
    boost::asio::async_read(socket_,
                            boost::asio::buffer(&buffer[0], buffer.size()),
                            BufferIoHandler(buffer, std::move(proceed)));
  });
  return buffer;
}

cppecho::net::BufferType cppecho::net::Socket::ReadPartial() {
  BufferType buffer;
  DeferIo([&](IoHandlerType proceed) {
    socket_.async_read_some(boost::asio::buffer(&buffer[0], buffer.size()),
                            BufferIoHandler(buffer, std::move(proceed)));
  });
  return buffer;
}

cppecho::net::BufferType cppecho::net::Socket::ReadUntil(
    const std::string& delimeter) {
  boost::asio::streambuf buffer;
  DeferIo([&](IoHandlerType proceed) {
    boost::asio::async_read_until(
        socket_, buffer, delimeter, BufferIoHandler(std::move(proceed)));
  });
  return ToBuffer(buffer);
}

void cppecho::net::Socket::Write(const BufferType& buffer) {
  DeferIo([&](IoHandlerType proceed) {
    boost::asio::async_write(socket_,
                             boost::asio::buffer(&buffer[0], buffer.size()),
                             BufferIoHandler(std::move(proceed)));
  });
}

void cppecho::net::Socket::Connect(const std::string& ip, int port) {
  DeferIo([&](IoHandlerType proceed) {
    socket_.async_connect(boost::asio::ip::tcp::endpoint(
                              boost::asio::ip::address::from_string(ip), port),
                          proceed);
  });
}

void cppecho::net::Socket::Connect(const EndPointType& end_point) {
  DeferIo([&](IoHandlerType proceed) {
    socket_.async_connect(end_point, proceed);
  });
}

void cppecho::net::Socket::Close() {
  socket_.close();
}

cppecho::net::Socket::SocketOptsMap cppecho::net::Socket::GetSocketOpts()
    const {
  boost::asio::ip::tcp::no_delay no_delay_option;
  socket_.get_option(no_delay_option);
  return {{SocketOpt::NoDelay, no_delay_option.value()}};
}

boost::signals2::connection cppecho::net::Socket::SubscribeOnData(
    const OnDataSubsriberType& subscriber) {
  LOG_AUTO_TRACE();
  RegisterToReceive();
  return on_data_.connect(subscriber);
}

boost::signals2::connection cppecho::net::Socket::SubscribeOnDisconnected(
    const OnDisconnectedSubsriberType& subscriber) {
  LOG_AUTO_TRACE();
  RegisterToReceive();
  return on_disconnected_.connect(subscriber);
}

void cppecho::net::Socket::OnSocketReceived(
    const boost::system::error_code& error, std::size_t bytes_transferred) {
  LOG_AUTO_TRACE();
  RunAsync([error, bytes_transferred, this]() {
    if ((error == boost::asio::error::eof) ||
        (error == boost::asio::error::connection_reset)) {
      LOG_ERROR("OnSocketReceived disconnected");
      on_disconnected_(*this);
      return;
    }

    if (error) {
      LOG_ERROR("OnSocketReceived error: " << error.value() << ", message: "
                                           << error.message());
      socket_.close();
      on_disconnected_(*this);
      return;
    }

    LOG_ERROR("OnSocketReceived raising OnData. bytes_transferred: "
              << bytes_transferred);
    auto end_iter = buffer_in_.begin();
    std::advance(end_iter, bytes_transferred);
    on_data_(*this, {buffer_in_.begin(), end_iter});

    is_waiting_receive_ = false;
    RegisterToReceive();
  });
}

void cppecho::net::Socket::RegisterToReceive() {
  LOG_AUTO_TRACE();

  if (is_waiting_receive_)
    return;

  socket_.async_receive(boost::asio::buffer(&buffer_in_[0], buffer_in_.size()),
                        std::bind(&Socket::OnSocketReceived,
                                  this,
                                  std::placeholders::_1,
                                  std::placeholders::_2));
  is_waiting_receive_ = true;
}

boost::optional<TcpServerIdType> cppecho::net::Socket::GetId() const {
  return id_;
}

void cppecho::net::Socket::SetId(boost::optional<TcpServerIdType> id) {
  id_ = std::move(id);
}
