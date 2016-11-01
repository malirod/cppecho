// Copyright [2016] <Malinovsky Rodion>

#pragma once

#include <memory>
#include <string>
#include "core/iengine.h"
#include "util/logger.h"

namespace cppecho {
namespace model {

class IEngineConfig;

}  // namespace model
}  // namespace cppecho

namespace cppecho {
namespace core {

class ThreadPool;

class Engine : public IEngine {
 public:
  Engine(std::unique_ptr<ThreadPool> thread_pool_,
         std::unique_ptr<model::IEngineConfig> engine_config);

  Engine& operator=(const Engine&) = delete;
  Engine(const Engine&) = delete;

  ~Engine() override;

  bool Launch() override;

  bool Init() override;

 private:
  DECLARE_GET_LOGGER("Core.Engine")

  std::unique_ptr<ThreadPool> thread_pool_;

  bool initiated_ = false;

  std::unique_ptr<model::IEngineConfig> engine_config_;
};

}  // namespace core
}  // namespace cppecho
