// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include <string>
#include "core/alias.h"
#include "core/iengine_config.h"

namespace rms {
namespace core {

/**
 * Implementation of IEngineConfig. Engine configuration.
 */
class EngineConfig : public IEngineConfig {
 public:
  /**
   * Create EngineConfig default instance.
   */
  EngineConfig();

  /**
   * Get server address stored in configuration.
   * @return Address as string.
   */
  const std::string& GetServerAddress() const override;

  /**
   * Get server port stored in configuration.
   * @return Server port.
   */
  PortType GetServerPort() const override;

  /**
   * Set server address for configuration.
   * @param value Server address to set.
   */
  void SetServerAddress(const std::string& value) override;

  /**
   * Set server port for configuration.
   * @param value Server port to set.
   */
  void SetServerPort(PortType value) override;

 private:
  std::string server_address_;

  PortType server_port_ = 0u;
};

}  // namespace core
}  // namespace rms
