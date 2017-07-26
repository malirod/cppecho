// Copyright [2017] <Malinovsky Rodion>

#include "net/tcp_server.h"

#include <utility>

#include "core/async.h"
#include "net/acceptor.h"
#include "net/util.h"
#include "util/smartptr_util.h"

using cppecho::util::make_unique;
using cppecho::core::RunAsync;
using cppecho::net::Socket;
using cppecho::net::BufferType;
using cppecho::net::TcpServerIdType;

cppecho::net::TcpServer::TcpServer(int max_connections)
    : max_connections_(max_connections), client_connections_(max_connections) {}

cppecho::net::TcpServer::~TcpServer() = default;

void cppecho::net::TcpServer::Start(
    const boost::asio::ip::tcp::endpoint& endpoint) {
  RunAsync([&]() {
    acceptor_ = make_unique<Acceptor>(endpoint);
    LOG_INFO("Accepting client connections on " << endpoint.address() << ":"
                                                << endpoint.port());

    on_listening_();

    while (!stopped_) {
      acceptor_->DoAccept(
          std::bind(&TcpServer::OnAccepted, this, std::placeholders::_1));
    }
  });
}

void cppecho::net::TcpServer::OnAccepted(
    std::unique_ptr<Socket> accepted_socket) {
  LOG_INFO("Accepted socket");
  assert(accepted_socket);

  const auto index = GetNewConnectionIndex();
  if (!index) {
    LOG_INFO("Max connections reached. Connection refused");
    accepted_socket->Close();
    return;
  }

  client_connections_[*index] = std::move(accepted_socket);

  auto& socket = *(client_connections_[*index]);
  // index is zero based, id it's 1-based
  const auto id = *index + 1;
  socket.SetId(id);
  LOG_INFO("Connected client. id: " << id);

  // Register to socket events

  socket.SubscribeOnData([&](Socket& socket, const BufferType& data) {
    LOG_DEBUG("Socket data in: " << data);
    assert(socket.GetId());
    on_data_(*socket.GetId(), data);
  });

  socket.SubscribeOnDisconnected([&](Socket& socket) {
    LOG_DEBUG("Socket disconnected");
    assert(socket.GetId());
    on_disconnected_(*socket.GetId());
  });

  on_connected_(*socket.GetId());
}

void cppecho::net::TcpServer::Start(int port) {
  Start(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
}

void cppecho::net::TcpServer::Start(const std::string& ip, int port) {
  Start(boost::asio::ip::tcp::endpoint(
      boost::asio::ip::address::from_string(ip), port));
}

void cppecho::net::TcpServer::Stop() {
  LOG_AUTO_TRACE();
  if (stopped_)
    return;
  stopped_ = true;

  RunAsync([&]() {
    acceptor_->Stop();
    for (auto&& item : client_connections_) {
      if (item)
        item->Close();
    }
  });
}

boost::optional<TcpServerIdType>
cppecho::net::TcpServer::GetNewConnectionIndex() const {
  const auto iter = std::find_if(client_connections_.begin(),
                                 client_connections_.end(),
                                 [](const std::unique_ptr<Socket>& item) {
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

bool cppecho::net::TcpServer::Close(TcpServerIdType id) {
  LOG_AUTO_TRACE();

  auto* socket_ptr = GetSocket(id);

  if (!socket_ptr)
    return false;

  socket_ptr->Close();

  return true;
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

Socket* cppecho::net::TcpServer::GetSocket(TcpServerIdType id) {
  LOG_AUTO_TRACE();
  if (id < 1) {
    LOG_DEBUG("Wrong id. Id is 1-based");
    return nullptr;
  }

  const auto index = id - 1;
  if (index >= client_connections_.size()) {
    LOG_DEBUG("Wrong id. Id is greater than size");
    return nullptr;
  }

  auto& socket = client_connections_[index];

  if (!socket) {
    LOG_DEBUG("Wrong id. No such connection.");
    return nullptr;
  }

  return socket.get();
}

boost::optional<BufferType> cppecho::net::TcpServer::ReadExact(
    TcpServerIdType id, std::size_t size) {
  LOG_AUTO_TRACE();
  auto* socket_ptr = GetSocket(id);

  if (!socket_ptr)
    return boost::none;

  return socket_ptr->ReadExact(size);
}

boost::optional<BufferType> cppecho::net::TcpServer::ReadPartial(
    TcpServerIdType id) {
  LOG_AUTO_TRACE();
  auto* socket_ptr = GetSocket(id);

  if (!socket_ptr)
    return boost::none;

  return socket_ptr->ReadPartial();
}

boost::optional<BufferType> cppecho::net::TcpServer::ReadUntil(
    TcpServerIdType id, const std::string& delimeter) {
  LOG_AUTO_TRACE();
  auto* socket_ptr = GetSocket(id);

  if (!socket_ptr)
    return boost::none;

  return socket_ptr->ReadUntil(delimeter);
}

void cppecho::net::TcpServer::Write(TcpServerIdType id,
                                    const BufferType& buffer) {
  LOG_AUTO_TRACE();
  auto* socket_ptr = GetSocket(id);

  if (!socket_ptr)
    return;

  socket_ptr->Write(buffer);
}
