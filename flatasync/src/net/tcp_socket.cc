// Copyright [2018] <Malinovsky Rodion>

#include "net/tcp_socket.h"

#include <utility>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include "core/async.h"
#include "core/iioservice.h"
#include "net/util.h"

using rms::core::GetCurrentThreadIoService;
using rms::core::RunAsync;
using rms::net::TcpServerIdType;

std::shared_ptr<rms::net::TcpSocket> rms::net::TcpSocket::Create() {
  return std::make_shared<TcpSocket>(TcpSocket::PrivateKey());
}

std::shared_ptr<rms::net::TcpSocket> rms::net::TcpSocket::Create(AsioTcpSocketType socket) {
  return std::make_shared<TcpSocket>(TcpSocket::PrivateKey(), std::move(socket));
}

rms::net::TcpSocket::TcpSocket(const PrivateKey& /*unused*/)
    : socket_(GetCurrentThreadIoService().GetAsioService()), scheduler_(rms::core::GetCurrentThreadScheduler()) {}

rms::net::TcpSocket::TcpSocket(const PrivateKey& /*unused*/, AsioTcpSocketType socket)
    : socket_(std::move(socket)), scheduler_(rms::core::GetCurrentThreadScheduler()) {}

rms::net::TcpSocket::~TcpSocket() {
  LOG_DEBUG("[" << GetId() << "] Destroying socket");
}

rms::net::BufferType rms::net::TcpSocket::ReadExact(std::size_t size) {
  auto self = shared_from_this();
  BufferType buffer(size, 0);
  if (!socket_.is_open()) {
    LOG_DEBUG("Skip ReadExact: not open");
    return buffer;
  }

  DeferIo([&, self](IoHandlerType proceed) {
    boost::asio::async_read(
        socket_, boost::asio::buffer(&buffer[0], buffer.size()), BufferIoHandler(buffer, std::move(proceed)));
  });

  if (!socket_.is_open() && !stopped_) {
    stopped_ = true;
    LOG_DEBUG("[" << GetId() << "] Closed after async operation (ReadExact): raise on_disconnect");
    RunAsync([&, self]() { on_disconnected_(*this); }, scheduler_);
  }

  return buffer;
}

std::pair<rms::net::BufferType, rms::net::ErrorType> rms::net::TcpSocket::ReadPartial() {
  constexpr static const int MAX_BUFFER = 1024;
  auto self = shared_from_this();
  BufferType buffer(MAX_BUFFER, 0);
  if (!socket_.is_open()) {
    LOG_DEBUG("Skip ReadPartial: not open");
    return std::make_pair(buffer, ErrorType());
  }

  const auto error = DeferIo([&, self](IoHandlerType proceed) {
    socket_.async_read_some(boost::asio::buffer(&buffer[0], buffer.size()),
                            BufferIoHandler(buffer, std::move(proceed)));
  });

  if (!socket_.is_open() && !stopped_) {
    stopped_ = true;
    LOG_DEBUG("[" << GetId() << "] Closed after async operation (ReadPartial): raise on_disconnect");
    RunAsync([&, self]() { on_disconnected_(*this); }, scheduler_);
  }

  return std::make_pair(buffer, error);
}

rms::net::BufferType rms::net::TcpSocket::ReadUntil(const std::string& delimeter) {
  auto self = shared_from_this();
  boost::asio::streambuf buffer;

  if (!socket_.is_open()) {
    LOG_DEBUG("Skip ReadUntil: not open");
    return ToBuffer(buffer);
  }

  DeferIo([&, self](IoHandlerType proceed) {
    boost::asio::async_read_until(socket_, buffer, delimeter, BufferIoHandler(std::move(proceed)));
  });

  if (!socket_.is_open() && !stopped_) {
    stopped_ = true;
    LOG_DEBUG("[" << GetId() << "] Closed after async operation (ReadUntil): raise on_disconnect");
    RunAsync([&, self]() { on_disconnected_(*this); }, scheduler_);
  }

  return ToBuffer(buffer);
}

void rms::net::TcpSocket::Write(const BufferType& buffer) {
  auto self = shared_from_this();
  DeferIo([&, self](IoHandlerType proceed) {
    boost::asio::async_write(
        socket_, boost::asio::buffer(&buffer[0], buffer.size()), BufferIoHandler(std::move(proceed)));
  });

  if (!socket_.is_open() && !stopped_) {
    stopped_ = true;
    LOG_DEBUG("[" << GetId() << "] Closed after async operation (Write): raise on_disconnect");
    RunAsync([&, self]() { on_disconnected_(*this); }, scheduler_);
  }
}

void rms::net::TcpSocket::Connect(const std::string& ip, int port) {
  Connect(EndPointType(boost::asio::ip::address::from_string(ip), port));
}

void rms::net::TcpSocket::Connect(const EndPointType& end_point) {
  if (socket_.is_open()) {
    LOG_DEBUG("[" << GetId() << "] Socket is open. Close it first.");
    return;
  }

  auto self = shared_from_this();
  stopped_ = false;
  LOG_DEBUG("[" << GetId() << "] Connecting");
  DeferIo([&, self](IoHandlerType proceed) { socket_.async_connect(end_point, proceed); });

  if (!socket_.is_open() && !stopped_) {
    stopped_ = true;
    LOG_DEBUG("[" << GetId() << "] Closed after async operation (Connect): raise on_disconnect");
    RunAsync([&, self]() { on_disconnected_(*this); }, scheduler_);
  }
}

void rms::net::TcpSocket::Stop() {
  LOG_AUTO_TRACE();
  auto self = shared_from_this();
  LOG_DEBUG("[" << GetId() << "] Start stopping socket");

  if (!socket_.is_open()) {
    LOG_DEBUG("[" << GetId() << "] Socket is already closed. Skip processing.");
    return;
  }

  RunAsync([&, self]() {
    LOG_DEBUG("[" << GetId() << "] Stopping socket");
    boost::system::error_code error;

    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
    if (error.value() != boost::system::errc::success) {
      LOG_DEBUG("[" << GetId() << "] Error during shutdown: " << error.message());
    }

    socket_.close(error);
    if (error.value() != boost::system::errc::success) {
      LOG_DEBUG("[" << GetId() << "] Error during close: " << error.message());
    }
    LOG_DEBUG("[" << GetId()
                  << "] Stop has been finished. Socket will be "
                     "stopped after all async handlers will "
                     "finish.");
  });
}

rms::net::TcpSocket::SocketOptsMap rms::net::TcpSocket::GetSocketOpts() const {
  boost::asio::ip::tcp::no_delay no_delay_option;
  socket_.get_option(no_delay_option);
  return {{SocketOpt::NoDelay, no_delay_option.value()}};
}

boost::signals2::connection rms::net::TcpSocket::SubscribeOnData(const OnDataSubsriberType& subscriber) {
  LOG_AUTO_TRACE();
  return on_data_.connect(subscriber);
}

boost::signals2::connection rms::net::TcpSocket::SubscribeOnDisconnected(
    const OnDisconnectedSubsriberType& subscriber) {
  LOG_AUTO_TRACE();
  return on_disconnected_.connect(subscriber);
}

void rms::net::TcpSocket::Start() {
  LOG_AUTO_TRACE();
  auto self = shared_from_this();

  if (!socket_.is_open()) {
    LOG_DEBUG("[" << GetId() << "] Skip Start: socket is not open");
    return;
  }

  RunAsync([&, self]() {
    LOG_DEBUG("[" << GetId() << "] Calling async_receive");

    const auto read_result = ReadPartial();

    const auto& error = read_result.second;

    if ((error == boost::asio::error::eof) || (error == boost::asio::error::connection_reset) ||
        (error == boost::asio::error::operation_aborted)) {
      LOG_DEBUG("[" << GetId() << "] Start: disconnected");

      // RunAsync([&, self]() { on_disconnected_(*this); }, scheduler_);
      if (!stopped_) {
        LOG_DEBUG("[" << GetId() << "] Start: disconnected");
        on_disconnected_(*this);
      } else {
        LOG_DEBUG("[" << GetId()
                      << "] Start: disconnected. Skip disconnection "
                         "event, elready stopped.");
      }
      return;
    }

    if (error.value() != boost::system::errc::success) {
      LOG_DEBUG("[" << GetId() << "] Start error: " << error.value() << ", message: " << error.message());
      Stop();

      // RunAsync([&, self]() { on_disconnected_(*this); }, scheduler_);

      if (!stopped_) {
        on_disconnected_(*this);

      } else {
        LOG_DEBUG("[" << GetId() << "] Start: skip disconnection event, elready stopped.");
      }

      return;
    }

    LOG_DEBUG("[" << GetId() << "] Start: raising OnData. bytes_transferred: " << read_result.first.size());

    // Run directly here instead of inside async op to avoid buffer copy
    on_data_(*this, read_result.first);

    RunAsync([&, self]() { Start(); });
  });
}

TcpServerIdType rms::net::TcpSocket::GetId() const {
  return id_;
}

void rms::net::TcpSocket::SetId(TcpServerIdType id) {
  LOG_DEBUG("Changing Id from " << id_ << " to " << id);
  id_ = id;
}
