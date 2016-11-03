// Copyright [2016] <Malinovsky Rodion>

#include "core/engine.h"
#include <iostream>
#include "boost/asio/signal_set.hpp"
#include "core/async.h"
#include "core/default_scheduler_accessor.h"
#include "core/startup_config.h"
#include "core/thread_pool.h"
#include "core/version.h"
#include "model/engine_config.h"
#include "model/general_error.h"
#include "util/logger.h"
#include "util/scope_guard.h"
#include "util/smartptr_util.h"

////////////////////////////////////////////////////////////////////////////////
// class EngineLauncher
////////////////////////////////////////////////////////////////////////////////

namespace cppecho {
namespace core {

class EngineLauncher {
 public:
  explicit EngineLauncher(std::unique_ptr<StartupConfig> startup_config);

  EngineLauncher(const EngineLauncher&) = delete;
  EngineLauncher(const EngineLauncher&&) = delete;
  EngineLauncher& operator=(const EngineLauncher&) = delete;
  EngineLauncher& operator=(const EngineLauncher&&) = delete;

  std::error_code Run();

 private:
  DECLARE_GET_LOGGER("Core.EngineLauncher")

  std::error_code Init();

  void DeInit();

  std::error_code DoRun();

  std::unique_ptr<StartupConfig> startup_config_;

  std::unique_ptr<IEngine> engine_;

  std::unique_ptr<ThreadPool> thread_pool_;
};

EngineLauncher::EngineLauncher(std::unique_ptr<StartupConfig> startup_config)
    : startup_config_(std::move(startup_config)) {}

std::error_code EngineLauncher::Init() {
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

void EngineLauncher::DeInit() {
  LOG_AUTO_TRACE();

  engine_.reset();

  GetDefaultIoServiceAccessorInstance().Detach();
  GetDefaultSchedulerAccessorInstance().Detach();

  thread_pool_.reset();
}

std::error_code EngineLauncher::DoRun() {
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

std::error_code EngineLauncher::Run() {
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

}  // namespace core
}  // namespace cppecho

////////////////////////////////////////////////////////////////////////////////
// Entry point
////////////////////////////////////////////////////////////////////////////////

DECLARE_GLOBAL_GET_LOGGER("Main")

int main(int argc, char** argv) {
  using cppecho::core::StartupConfig;
  using cppecho::core::EngineLauncher;
  using cppecho::model::GeneralError;
  using cppecho::util::make_unique;
  INIT_LOGGER("logger.cfg");

  LOG_INFO("Starting cppecho server " << cppecho::core::version::GetVersion());

  try {
    auto startup_config = make_unique<StartupConfig>();

    if (!startup_config->Parse(argc, argv)) {
      const auto error_code =
          make_error_condition(GeneralError::WrongCommandLine);
      return error_code.value();
    }
    EngineLauncher engine_launcher(std::move(startup_config));

    const auto error_code = engine_launcher.Run();
    LOG_INFO("Cppecho has finished with exit code '" << error_code.message()
                                                     << "'");
    return error_code.value();
  } catch (std::exception& e) {
    LOG_FATAL("Exception has occured: " << e.what());
  } catch (...) {
    LOG_FATAL("Unknown exception has occured");
  }

  const auto error_code = make_error_condition(GeneralError::InternalError);
  LOG_FATAL("Unexpected exit: " << error_code.message());
  return error_code.value();
}
