// Copyright [2016] <Malinovsky Rodion>

#include <iostream>

#include "core/engine.h"
#include "core/startup_config.h"
#include "core/version.h"
#include "model/general_error.h"
#include "util/logger.h"
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

  std::error_code launch();

 private:
  DECLARE_GET_LOGGER("Core.EngineLauncher")

  std::error_code Init();

  std::error_code DoLaunch();

  std::unique_ptr<StartupConfig> startup_config_;

  std::unique_ptr<IEngine> engine_;
};

EngineLauncher::EngineLauncher(std::unique_ptr<StartupConfig> startup_config)
    : startup_config_(std::move(startup_config)) {}

std::error_code EngineLauncher::Init() {
  LOG_AUTO_TRACE();
  engine_ = util::make_unique<Engine>(startup_config_->GetAddress(),
                                      startup_config_->GetPort());
  return std::error_code();
}

std::error_code EngineLauncher::DoLaunch() {
  LOG_AUTO_TRACE();
  if (!engine_->Start()) {
    LOG_ERROR("Failed to start Engine");
    return make_error_code(model::GeneralError::StartupFailed);
  }

  LOG_INFO("Waiting for termination request");

  LOG_INFO("Termination request received");

  engine_.reset();
  return std::error_code();
}

std::error_code EngineLauncher::launch() {
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
  return error_code ? error_code : DoLaunch();
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

    const auto error_code = engine_launcher.launch();
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
