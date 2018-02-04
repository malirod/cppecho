// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include <memory>
#include <system_error>
#include "core/iengine.h"
#include "core/startup_config.h"
#include "core/thread_pool.h"
#include "util/logger.h"

namespace rms {
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

  std::unique_ptr<ThreadPool> thread_pool_main_;

  std::unique_ptr<ThreadPool> thread_pool_net_;
};

}  // namespace core
}  // namespace rms