// Copyright [2016] <Malinovsky Rodion>

#include "boost/asio/signal_set.hpp"
#include "core/engine_launcher.h"
#include "core/general_error.h"
#include "core/startup_config.h"
#include "core/version.h"
#include "util/logger.h"
#include "util/smartptr_util.h"

DECLARE_GLOBAL_GET_LOGGER("Main")

int main(int argc, char** argv) {
  using cppecho::core::StartupConfig;
  using cppecho::core::EngineLauncher;
  using cppecho::core::GeneralError;
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
