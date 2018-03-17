// Copyright [2018] <Malinovsky Rodion>

#include <algorithm>
#include <exception>
#include <memory>
#include <system_error>
#include "core/engine_launcher.h"
#include "core/general_error.h"
#include "core/startup_config.h"
#include "core/version.h"
#include "util/logger.h"

DECLARE_GLOBAL_GET_LOGGER("Main")

/**
 * Main entry point of the application.
 * @param argc Count of command line arguments.
 * @param argv Command line arguments.
 * @return Error code.
 */
int main(int argc, char** argv) {
  using rms::core::EngineLauncher;
  using rms::core::GeneralError;
  using rms::core::StartupConfig;

  INIT_LOGGER("logger.cfg");

  LOG_INFO("Starting cppecho server " << rms::core::version::GetVersion());

  try {
    auto startup_config = std::make_unique<StartupConfig>();

    if (!startup_config->Parse(argc, argv)) {
      const auto error_code = make_error_condition(GeneralError::WrongCommandLine);
      return error_code.value();
    }
    EngineLauncher engine_launcher(std::move(startup_config));

    const auto error_code = engine_launcher.Run();
    LOG_INFO("Cppecho has finished with exit code '" << error_code.message() << "'");
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
