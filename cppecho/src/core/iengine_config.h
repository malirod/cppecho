// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include <string>
#include "core/alias.h"

namespace rms {
namespace core {

/**
 * Interface for Engine configuration.
 */
class IEngineConfig {
 public:
  /**
   * Destructor.
   */
  virtual ~IEngineConfig() = default;

  /**
   * Get server address as string.
   * @return Server address as string.
   */
  virtual const std::string& GetServerAddress() const = 0;

  /**
   * Get server port.
   * @return Port
   */
  virtual PortType GetServerPort() const = 0;

  /**
   * Set server address string.
   * @param value Server address as string.
   */
  virtual void SetServerAddress(const std::string& value) = 0;

  /**
   * Set server port.
   * @param value Port.
   */
  virtual void SetServerPort(PortType value) = 0;
};

}  // namespace core
}  // namespace rms
