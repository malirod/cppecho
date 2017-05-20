// Copyright [2016] <Malinovsky Rodion>

#pragma once

#include <string>
#include "core/alias.h"

namespace cppecho {
namespace core {

class IEngineConfig {
 public:
  virtual ~IEngineConfig() = default;

  virtual const std::string& GetServerAddress() const = 0;

  virtual PortType GetServerPort() const = 0;

  virtual void SetServerAddress(const std::string& value) = 0;

  virtual void SetServerPort(PortType value) = 0;
};

}  // namespace core
}  // namespace cppecho
