// Copyright [2016] <Malinovsky Rodion>

#include "core/engine_launcher.h"
#include <iostream>
#include <utility>
#include "core/async.h"
#include "core/default_scheduler_accessor.h"
#include "core/startup_config.h"
#include "core/version.h"
#include "model/engine_config.h"
#include "model/general_error.h"
#include "util/logger.h"
#include "util/scope_guard.h"
#include "util/smartptr_util.h"

cppecho::core::EngineLauncher::EngineLauncher(
    std::unique_ptr<StartupConfig> startup_config)
    : startup_config_(std::move(startup_config)) {}

std::error_code cppecho::core::EngineLauncher::Init() {
  LOG_AUTO_TRACE();

  const int thread_pool_size = std::thread::hardware_concurrency();
  thread_pool_ = util::make_unique<ThreadPool>(thread_pool_size, "main");

  GetDefaultIoServiceAccessorInstance().Attach(*thread_pool_);
  GetDefaultSchedulerAccessorInstance().Attach(*thread_pool_);

  auto engine_config = util::make_unique<model::EngineConfig>();
  if (!startup_config_->GetAddress().empty()) {
    engine_config->SetServerAddress(startup_config_->GetAddress());
  }
  if (startup_config_->GetPort() != 0) {
    engine_config->SetServerPort(startup_config_->GetPort());
  }

  engine_ = util::make_unique<Engine>(std::move(engine_config));

  const auto initiated = engine_->Init();
  return initiated ? std::error_code()
                   : make_error_code(model::GeneralError::StartupFailed);
}

void cppecho::core::EngineLauncher::DeInit() {
  LOG_AUTO_TRACE();

  engine_.reset();

  GetDefaultIoServiceAccessorInstance().Detach();
  GetDefaultSchedulerAccessorInstance().Detach();

  thread_pool_.reset();
}

std::error_code cppecho::core::EngineLauncher::DoRun() {
  LOG_AUTO_TRACE();
  if (!engine_->Launch()) {
    LOG_ERROR("Failed to start Engine");
    return make_error_code(model::GeneralError::StartupFailed);
  }
  auto& asio_service =
      GetDefaultIoServiceAccessorInstance().GetRef().GetAsioService();

  boost::asio::signal_set signals(asio_service, SIGINT, SIGTERM);
  signals.async_wait([&asio_service, this](
      const boost::system::error_code& error, int signal_number) {
    if (!error) {
      LOG_INFO("Termination request received: " << signal_number
                                                << ". Stopping");
      asio_service.stop();
    } else {
      LOG_ERROR("Error in signals handler: " << error.message());
    }
  });

  LOG_INFO("Waiting for termination request");
  asio_service.run();

  return std::error_code();
}

std::error_code cppecho::core::EngineLauncher::Run() {
  LOG_AUTO_TRACE();

  if (startup_config_->GetIsShowHelp()) {
    std::cout << startup_config_->GetHelp();
    return std::error_code();
  }

  if (startup_config_->GetIsShowVersion()) {
    std::cout << version::GetVersion() << std::endl;
    return std::error_code();
  }

  const auto error_code = Init();
  auto scope_guard = util::MakeScopeGuard([&]() { DeInit(); });
  return error_code ? error_code : DoRun();
}
