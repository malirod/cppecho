// Copyright [2018] <Malinovsky Rodion>

#include "core/engine_config.h"

namespace {

const char* const kDefaultListenAddress = "0.0.0.0";
const rms::core::PortType kDefaultListenPort = 8088u;

}  // namespace

rms::core::EngineConfig::EngineConfig() : server_address_(kDefaultListenAddress), server_port_(kDefaultListenPort) {}

const std::string& rms::core::EngineConfig::GetServerAddress() const {
  return server_address_;
}

rms::core::PortType rms::core::EngineConfig::GetServerPort() const {
  return server_port_;
}

void rms::core::EngineConfig::SetServerAddress(const std::string& value) {
  server_address_ = value;
}

void rms::core::EngineConfig::SetServerPort(PortType value) {
  server_port_ = value;
}
