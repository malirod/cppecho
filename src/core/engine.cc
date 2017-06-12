// Copyright [2016] <Malinovsky Rodion>

#include "core/engine.h"
#include <cassert>
#include <utility>
#include "core/async.h"
#include "core/default_scheduler_accessor.h"
#include "core/iengine_config.h"
#include "core/ischeduler.h"
#include "net/util.h"

using cppecho::net::GetNetworkSchedulerAccessorInstance;

cppecho::core::Engine::Engine(
    std::unique_ptr<core::IEngineConfig> engine_config)
    : engine_config_(std::move(engine_config)) {
  LOG_AUTO_TRACE();
  assert(engine_config_ != nullptr && "Config is not set");
  LOG_INFO("Engine has been created.");
}

cppecho::core::Engine::~Engine() {
  LOG_AUTO_TRACE();

  LOG_INFO("Engine has been destroyed.");
}

bool cppecho::core::Engine::Launch() {
  LOG_AUTO_TRACE();
  LOG_INFO("Launching engine. Listenig on "
           << engine_config_->GetServerAddress()
           << ":"
           << engine_config_->GetServerPort());
  assert(initiated_);

  RunAsync(
      [&]() {
        LOG_INFO("Accepting client connections on "
                 << engine_config_->GetServerAddress()
                 << ":"
                 << engine_config_->GetServerPort());
      },
      GetNetworkSchedulerAccessorInstance());

  LOG_INFO("Engine has been launched.");
  return true;
}

bool cppecho::core::Engine::Init() {
  LOG_AUTO_TRACE();
  assert(!initiated_);

  initiated_ = true;
  return initiated_;
}
