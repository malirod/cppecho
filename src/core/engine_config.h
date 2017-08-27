// Copyright [2016] <Malinovsky Rodion>

#pragma once

#include <string>
#include "core/iengine_config.h"

namespace cppecho {
namespace core {

class EngineConfig : public IEngineConfig {
 public:
  EngineConfig();

  const std::string& GetServerAddress() const override;

  PortType GetServerPort() const override;

  void SetServerAddress(const std::string& value) override;

  void SetServerPort(PortType value) override;

 private:
  std::string server_address_;

  PortType server_port_ = 0u;
};

}  // namespace core
}  // namespace cppecho
