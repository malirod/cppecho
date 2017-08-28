// Copyright [2016] <Malinovsky Rodion>

#include "core/engine.h"
#include <cassert>
#include <utility>

#include "boost/algorithm/string.hpp"

#include "core/async.h"
#include "core/default_scheduler_accessor.h"
#include "core/iengine_config.h"
#include "core/ischeduler.h"
#include "core/sequential_scheduler.h"
#include "net/acceptor.h"
#include "net/tcp_server.h"
#include "net/util.h"
#include "util/smartptr_util.h"

using cppecho::net::GetNetworkSchedulerAccessorInstance;
using cppecho::net::GetNetworkServiceAccessorInstance;
using cppecho::util::make_unique;
using cppecho::net::TcpServer;
using cppecho::net::TcpServerIdType;
using cppecho::net::BufferType;

namespace {

const int MAX_CONNECTIONS_COUNT = 100;
const char SERVER_ECHO_PREFIX[] = "echo: ";
const char main_sequential_scheduler_name[] = "main_sequential";

}  // namespace

cppecho::core::Engine::Engine(
    std::unique_ptr<core::IEngineConfig> engine_config)
    : engine_config_(std::move(engine_config))
    , main_sequential_scheduler_(make_unique<SequentialScheduler>(
          GetDefaultIoServiceAccessorInstance().GetRef(),
          main_sequential_scheduler_name)) {
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
  LOG_INFO("Starting engine");
  assert(initiated_);

  RunAsync(
      [&]() {
        tcp_server_ = make_unique<TcpServer>(MAX_CONNECTIONS_COUNT);

        tcp_server_->SubscribeOnListening([&]() {
          LOG_INFO("Listenig on " << engine_config_->GetServerAddress() << ":"
                                  << engine_config_->GetServerPort());
          on_started_();
        });

        tcp_server_->SubscribeOnConnected([&](TcpServerIdType id) {
          (void)id;
          LOG_DEBUG("Client Connected. Id: " << id);
        });

        tcp_server_->SubscribeOnData(
            [&](TcpServerIdType id, const BufferType& data) {
              LOG_DEBUG("Received data: " << data);
              BufferType snd_buffer{SERVER_ECHO_PREFIX};
              snd_buffer += data;
              LOG_DEBUG("Sending data: " << snd_buffer);
              tcp_server_->Write(id, snd_buffer);
            });

        tcp_server_->SubscribeOnDisconnected([&](TcpServerIdType id) {
          (void)id;
          LOG_DEBUG("Client DisConnected. Id: " << id);
        });

        tcp_server_->SubscribeOnStopped([&]() {
          LOG_DEBUG("TcpServer has been closed.");
          on_stopped_();
        });

        tcp_server_->Start(engine_config_->GetServerAddress(),
                           engine_config_->GetServerPort());
      },
      *main_sequential_scheduler_);

  LOG_INFO("Engine has been launched.");
  return true;
}

bool cppecho::core::Engine::Stop() {
  LOG_AUTO_TRACE();
  LOG_INFO("Stopping engine");
  assert(initiated_);

  if (stopped_) {
    LOG_INFO("Already stopped. Skip.");
    return true;
  }

  stopped_ = true;

  RunAsync([&]() { tcp_server_->Stop(); }, *main_sequential_scheduler_);

  return true;
}

bool cppecho::core::Engine::Init() {
  LOG_AUTO_TRACE();
  assert(!initiated_);

  initiated_ = true;
  return initiated_;
}

boost::signals2::connection cppecho::core::Engine::SubscribeOnStarted(
    const OnStartedSubsriberType& subscriber) {
  LOG_AUTO_TRACE();
  return on_started_.connect(subscriber);
}

boost::signals2::connection cppecho::core::Engine::SubscribeOnStopped(
    const OnStoppedSubsriberType& subscriber) {
  LOG_AUTO_TRACE();
  return on_stopped_.connect(subscriber);
}
