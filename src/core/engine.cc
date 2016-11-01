// Copyright [2016] <Malinovsky Rodion>

#include "core/engine.h"
#include <cassert>
#include <utility>
#include "core/async.h"
#include "core/ischeduler.h"
#include "core/thread_pool.h"
#include "model/iengine_config.h"

cppecho::core::Engine::Engine(
    std::unique_ptr<ThreadPool> thread_pool,
    std::unique_ptr<model::IEngineConfig> engine_config)
    : thread_pool_(std::move(thread_pool))
    , engine_config_(std::move(engine_config)) {
  LOG_AUTO_TRACE();
  assert(thread_pool_ != nullptr && "Thread pool is not set");
  assert(engine_config_ != nullptr && "Config is not set");
  LOG_INFO("Engine has been created.");
}

cppecho::core::Engine::~Engine() {
  LOG_AUTO_TRACE();
  GetDefaultIoServiceAccessorInstance().Detach();
  GetDefaultSchedulerAccessorInstance().Detach();
  LOG_INFO("Engine has been destroyed.");
}

bool cppecho::core::Engine::Launch() {
  LOG_AUTO_TRACE();
  LOG_INFO("Launching engine. Listenig on "
           << engine_config_->GetServerAddress()
           << ":"
           << engine_config_->GetServerPort());
  assert(initiated_);

  LOG_INFO("Engine has been launched.");
  return true;
}

bool cppecho::core::Engine::Init() {
  LOG_AUTO_TRACE();
  assert(!initiated_);

  GetDefaultIoServiceAccessorInstance().Attach(*thread_pool_);
  GetDefaultSchedulerAccessorInstance().Attach(*thread_pool_);

  initiated_ = true;
  return initiated_;
}
