// Copyright [2016] <Malinovsky Rodion>

#pragma once

#include <string>
#include "core/iengine.h"
#include "util/logger.h"

namespace cppecho {
namespace core {

class Engine : public IEngine {
 public:
  Engine() = delete;
  Engine(const std::string& address, std::uint32_t port);

  Engine& operator=(const Engine&) = delete;
  Engine(const Engine&) = delete;

  ~Engine() override;

  bool Start() override;

  void Stop() override;

  bool Init() override;

 private:
  DECLARE_GET_LOGGER("Core.Engine")

  std::string address_;

  std::uint32_t port_;
};

}  // namespace core
}  // namespace cppecho
