// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include <string>
#include "core/alias.h"
#include "core/iengine_config.h"

namespace rms {
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
}  // namespace rms
