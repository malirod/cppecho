// Copyright [2016] <Malinovsky Rodion>

#include "core/engine.h"
#include <cassert>
#include <utility>

#include "boost/algorithm/string.hpp"

#include "core/async.h"
#include "core/default_scheduler_accessor.h"
#include "core/iengine_config.h"
#include "core/ischeduler.h"
#include "net/acceptor.h"
#include "net/util.h"
#include "util/smartptr_util.h"

using cppecho::net::GetNetworkSchedulerAccessorInstance;
using cppecho::net::GetNetworkServiceAccessorInstance;
using cppecho::net::Acceptor;
using cppecho::net::BufferType;
using cppecho::net::Socket;
using cppecho::util::make_unique;

cppecho::core::Engine::Engine(
    std::unique_ptr<core::IEngineConfig> engine_config)
    : engine_config_(std::move(engine_config)) {
  LOG_AUTO_TRACE();
  assert(engine_config_ != nullptr && "Config is not set");
  LOG_INFO("Engine has been created.");
}

cppecho::core::Engine::~Engine() {
  LOG_AUTO_TRACE();

  stopped_ = true;

  LOG_INFO("Engine has been destroyed.");
}

bool cppecho::core::Engine::Start() {
  LOG_AUTO_TRACE();
  LOG_INFO("Starting engine. Listenig on " << engine_config_->GetServerAddress()
                                           << ":"
                                           << engine_config_->GetServerPort());
  assert(initiated_);

  RunAsync(
      [&]() {
        acceptor_ = make_unique<Acceptor>(engine_config_->GetServerAddress(),
                                          engine_config_->GetServerPort());
        LOG_INFO("Accepting client connections on "
                 << engine_config_->GetServerAddress()
                 << ":"
                 << engine_config_->GetServerPort());

        on_started_();

        while (!stopped_) {
          acceptor_->DoAccept(
              std::bind(&Engine::OnAccepted, this, std::placeholders::_1));
        }
      },
      GetNetworkSchedulerAccessorInstance());

  LOG_INFO("Engine has been launched.");
  return true;
}

bool cppecho::core::Engine::Stop() {
  LOG_AUTO_TRACE();
  LOG_INFO("Stopping engine");
  assert(initiated_);

  stopped_ = true;

  RunAsync([&]() { acceptor_->Stop(); }, GetNetworkSchedulerAccessorInstance());

  return true;
}

bool cppecho::core::Engine::Init() {
  LOG_AUTO_TRACE();
  assert(!initiated_);

  initiated_ = true;
  return initiated_;
}

void cppecho::core::Engine::OnAccepted(
    std::unique_ptr<Socket> accepted_socket) {
  LOG_DEBUG("Accepted socket");
  assert(accepted_socket);
  client_connections_.emplace_back(std::move(accepted_socket));
  auto& socket = *(client_connections_.back());
  const auto rcv_buffer = boost::algorithm::trim_copy(socket.ReadUntil("\n"));
  LOG_DEBUG("Read from socket: " << rcv_buffer);
  socket.Write("echo: " + rcv_buffer + '\n');
}

boost::signals2::connection cppecho::core::Engine::SubscribeOnStarted(
    const OnStartedSubsriberType& subscriber) {
  LOG_AUTO_TRACE();
  return on_started_.connect(subscriber);
}
