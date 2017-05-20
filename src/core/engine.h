// Copyright [2016] <Malinovsky Rodion>

#pragma once

#include <memory>
#include <string>
#include "core/iengine.h"
#include "util/logger.h"

namespace cppecho {
namespace core {

class IEngineConfig;

}  // namespace core
}  // namespace cppecho

namespace cppecho {
namespace core {

class Engine : public IEngine {
 public:
  explicit Engine(std::unique_ptr<core::IEngineConfig> engine_config);

  Engine& operator=(const Engine&) = delete;
  Engine(const Engine&) = delete;

  ~Engine() override;

  bool Launch() override;

  bool Init() override;

 private:
  DECLARE_GET_LOGGER("Core.Engine")

  bool initiated_ = false;

  std::unique_ptr<core::IEngineConfig> engine_config_;
};

}  // namespace core
}  // namespace cppecho
