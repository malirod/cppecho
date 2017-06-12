// Copyright [2016] <Malinovsky Rodion>

#include "core/engine_launcher.h"
#include <iostream>
#include <utility>
#include "core/async.h"
#include "core/default_scheduler_accessor.h"
#include "core/engine_config.h"
#include "core/general_error.h"
#include "core/startup_config.h"
#include "core/version.h"
#include "net/util.h"
#include "util/logger.h"
#include "util/scope_guard.h"
#include "util/smartptr_util.h"

using cppecho::net::GetNetworkServiceAccessorInstance;
using cppecho::net::GetNetworkSchedulerAccessorInstance;

cppecho::core::EngineLauncher::EngineLauncher(
    std::unique_ptr<StartupConfig> startup_config)
    : startup_config_(std::move(startup_config)) {}

std::error_code cppecho::core::EngineLauncher::Init() {
  LOG_AUTO_TRACE();

  const auto hardware_threads_count = std::thread::hardware_concurrency();
  const int thread_pool_size =
      hardware_threads_count >= 2 ? hardware_threads_count : 2;

  thread_pool_main_ = util::make_unique<ThreadPool>(thread_pool_size, "main");
  thread_pool_net_ = util::make_unique<ThreadPool>(thread_pool_size, "net");

  GetDefaultIoServiceAccessorInstance().Attach(*thread_pool_main_);
  GetDefaultSchedulerAccessorInstance().Attach(*thread_pool_main_);
  GetNetworkServiceAccessorInstance().Attach(*thread_pool_net_);
  GetNetworkSchedulerAccessorInstance().Attach(*thread_pool_net_);

  auto engine_config = util::make_unique<core::EngineConfig>();
  if (!startup_config_->GetAddress().empty()) {
    engine_config->SetServerAddress(startup_config_->GetAddress());
  }
  if (startup_config_->GetPort() != 0) {
    engine_config->SetServerPort(startup_config_->GetPort());
  }

  engine_ = util::make_unique<Engine>(std::move(engine_config));

  const auto initiated = engine_->Init();
  return initiated ? std::error_code()
                   : make_error_code(core::GeneralError::StartupFailed);
}

void cppecho::core::EngineLauncher::DeInit() {
  LOG_AUTO_TRACE();

  engine_.reset();

  GetNetworkServiceAccessorInstance().Detach();
  GetDefaultIoServiceAccessorInstance().Detach();
  GetDefaultSchedulerAccessorInstance().Detach();
  GetNetworkServiceAccessorInstance().Detach();

  thread_pool_net_.reset();
  thread_pool_main_.reset();
}

std::error_code cppecho::core::EngineLauncher::DoRun() {
  LOG_AUTO_TRACE();
  if (!engine_->Launch()) {
    LOG_ERROR("Failed to start Engine");
    return make_error_code(core::GeneralError::StartupFailed);
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
