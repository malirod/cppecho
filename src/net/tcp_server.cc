// Copyright [2017] <Malinovsky Rodion>

#include "net/tcp_server.h"

#include <utility>

#include "core/async.h"
#include "net/acceptor.h"
#include "net/util.h"
#include "util/smartptr_util.h"

using cppecho::core::RunAsync;
using cppecho::util::make_unique;
using cppecho::net::TcpSocket;
using cppecho::net::BufferType;
using cppecho::net::ErrorType;
using cppecho::net::TcpServerIdType;

namespace {

std::size_t GetSocketIndex(TcpServerIdType id) {
  if (id < 1) {
    // Wrong id. Id is 1-based
    return 0;
  }
  return id - 1;
}

}  // namespace

cppecho::net::TcpServer::TcpServer(int max_connections)
    : max_connections_(max_connections), client_connections_(max_connections) {}

cppecho::net::TcpServer::~TcpServer() = default;

void cppecho::net::TcpServer::OnAccepted(
    std::shared_ptr<TcpSocket> accepted_socket) {
  LOG_INFO("Accepted socket");
  assert(accepted_socket);

  if (!is_running_) {
    LOG_INFO("Server is not running. Skipping acceptance.");
    return;
  }

  const auto index = GetNewConnectionIndex();
  if (!index) {
    LOG_INFO("Max connections reached. Connection refused");
    accepted_socket->Stop();
    return;
  }

  client_connections_[*index] = std::move(accepted_socket);

  auto& socket = *(client_connections_[*index]);
  // index is zero based, id it's 1-based
  const auto id = *index + 1;
  socket.SetId(id);
  LOG_INFO("Connected client. id: " << id);

  // Register to socket events

  socket.SubscribeOnData([&](TcpSocket& socket, const BufferType& data) {
    LOG_DEBUG("Socket data in: " << data);
    assert(socket.GetId());
    on_data_(socket.GetId(), data);
  });

  socket.SubscribeOnDisconnected([&](TcpSocket& socket) {
    LOG_DEBUG("Socket disconnected id: " << socket.GetId());
    assert(socket.GetId());
    RunAsync([&]() {
      const auto id = socket.GetId();
      on_disconnected_(id);
      LOG_DEBUG("Releasing socket id=" << id);
      client_connections_[GetSocketIndex(id)].reset();
      if (!is_running_) {
        RaiseOnClosed();
      }
    });
  });

  RunAsync([&]() { on_connected_(socket.GetId()); });

  socket.Start();

  RunAsync([&]() {
    if (is_running_) {
      acceptor_->DoAccept(
          std::bind(&TcpServer::OnAccepted, this, std::placeholders::_1));
    }
  });
}

void cppecho::net::TcpServer::Start(const EndPointType& endpoint) {
  LOG_AUTO_TRACE();
  if (is_running_) {
    LOG_DEBUG("Server is running. Stop first.");
    return;
  }
  is_running_ = true;

  // Capture endpoint by value, since it's temp and might be destroyed before
  // async code will execute
  RunAsync([&, endpoint]() {
    if (!is_running_) {
      LOG_DEBUG("Skip start listening: not running.");
      return;
    }
    LOG_INFO("Accepting client connections on " << endpoint.address() << ":"
                                                << endpoint.port());
    acceptor_ = make_unique<Acceptor>(endpoint);

    RunAsync([&]() {
      LOG_DEBUG("Start listening");

      on_listening_();
    });

    acceptor_->DoAccept(
        std::bind(&TcpServer::OnAccepted, this, std::placeholders::_1));
  });
}

void cppecho::net::TcpServer::Start(int port) {
  Start(EndPointType(boost::asio::ip::tcp::v4(), port));
}

void cppecho::net::TcpServer::Start(const std::string& ip, int port) {
  Start(EndPointType(boost::asio::ip::address::from_string(ip), port));
}

void cppecho::net::TcpServer::RaiseOnClosed() {
  const auto count = GetConnectedCount();
  if (count == 0) {
    LOG_INFO("All connections has been closed");
    on_stopped_();
  } else {
    LOG_DEBUG(
        "Waiting for client connections are closed. Active connections count "
        "is "
        << count);
  }
}

void cppecho::net::TcpServer::Stop() {
  LOG_AUTO_TRACE();
  RunAsync([&] {
    if (!is_running_) {
      LOG_DEBUG("Server already stopped");
      return;
    }
    is_running_ = false;

    LOG_DEBUG("Stopping acceptor");
    acceptor_->Stop();
    LOG_DEBUG("Stopping all connected sockets");
    for (auto&& item : client_connections_) {
      if (item) {
        LOG_DEBUG("Stopping socket id: " << item->GetId());
        item->Stop();
      }
    }
    RaiseOnClosed();
  });
}

std::size_t cppecho::net::TcpServer::GetConnectedCount() const {
  std::size_t result = 0u;
  for (auto&& item : client_connections_) {
    if (item) {
      ++result;
    }
  }
  return result;
}

boost::optional<TcpServerIdType>
cppecho::net::TcpServer::GetNewConnectionIndex() const {
  const auto iter = std::find_if(client_connections_.begin(),
                                 client_connections_.end(),
                                 [](const ClientConnectionItemType& item) {
                                   if (item) {
                                     return false;
                                   }
                                   return true;
                                 });
  if (iter == client_connections_.end()) {
    return boost::none;
  }
  return static_cast<TcpServerIdType>(
      std::distance(client_connections_.begin(), iter));
}

void cppecho::net::TcpServer::StopClient(TcpServerIdType id) {
  LOG_AUTO_TRACE();
  RunAsync([&] {
    auto socket = GetSocket(id);

    if (!socket)
      return;

    socket->Stop();

    return;
  });
  return;
}

boost::signals2::connection cppecho::net::TcpServer::SubscribeOnListening(
    const OnListeningSubsriberType& subscriber) {
  LOG_AUTO_TRACE();
  return on_listening_.connect(subscriber);
}

boost::signals2::connection cppecho::net::TcpServer::SubscribeOnConnected(
    const OnConnectedSubsriberType& subscriber) {
  LOG_AUTO_TRACE();
  return on_connected_.connect(subscriber);
}

boost::signals2::connection cppecho::net::TcpServer::SubscribeOnData(
    const OnDataSubsriberType& subscriber) {
  LOG_AUTO_TRACE();
  return on_data_.connect(subscriber);
}

boost::signals2::connection cppecho::net::TcpServer::SubscribeOnDisconnected(
    const OnDisconnectedSubsriberType& subscriber) {
  LOG_AUTO_TRACE();
  return on_disconnected_.connect(subscriber);
}

boost::signals2::connection cppecho::net::TcpServer::SubscribeOnStopped(
    const OnStoppedSubsriberType& subscriber) {
  LOG_AUTO_TRACE();
  return on_stopped_.connect(subscriber);
}

std::shared_ptr<cppecho::net::TcpSocket> cppecho::net::TcpServer::GetSocket(
    TcpServerIdType id) {
  LOG_AUTO_TRACE();
  if (id < 1) {
    LOG_DEBUG("Wrong id. Id is 1-based");
    return nullptr;
  }

  const auto index = GetSocketIndex(id);
  if (index >= client_connections_.size()) {
    LOG_DEBUG("Wrong id. Id is greater than size");
    return nullptr;
  }

  auto socket = client_connections_[index];

  if (!socket) {
    LOG_DEBUG("Wrong id. No such connection.");
    return nullptr;
  }

  return socket;
}

boost::optional<BufferType> cppecho::net::TcpServer::ReadExact(
    TcpServerIdType id, std::size_t size) {
  LOG_AUTO_TRACE();
  auto socket = GetSocket(id);

  if (!socket)
    return boost::none;

  return socket->ReadExact(size);
}

boost::optional<std::pair<BufferType, ErrorType>>
cppecho::net::TcpServer::ReadPartial(TcpServerIdType id) {
  LOG_AUTO_TRACE();
  auto socket = GetSocket(id);

  if (!socket) {
    return boost::none;
  }

  return socket->ReadPartial();
}

boost::optional<BufferType> cppecho::net::TcpServer::ReadUntil(
    TcpServerIdType id, const std::string& delimeter) {
  LOG_AUTO_TRACE();
  auto socket = GetSocket(id);

  if (!socket)
    return boost::none;

  return socket->ReadUntil(delimeter);
}

void cppecho::net::TcpServer::Write(TcpServerIdType id,
                                    const BufferType& buffer) {
  LOG_AUTO_TRACE();
  LOG_DEBUG("Writing to id=" << id);
  auto socket = GetSocket(id);

  if (!socket)
    return;

  socket->Write(buffer);
}
