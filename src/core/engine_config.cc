// Copyright [2016] <Malinovsky Rodion>

#include "core/engine_config.h"

namespace {

const char* const kDefaultListenAddress = "0.0.0.0";
const cppecho::core::PortType kDefaultListenPort = 8088u;

}  // namespace

cppecho::core::EngineConfig::EngineConfig()
    : server_address_(kDefaultListenAddress)
    , server_port_(kDefaultListenPort) {}

const std::string& cppecho::core::EngineConfig::GetServerAddress() const {
  return server_address_;
}

cppecho::core::PortType cppecho::core::EngineConfig::GetServerPort() const {
  return server_port_;
}

void cppecho::core::EngineConfig::SetServerAddress(const std::string& value) {
  server_address_ = value;
}

void cppecho::core::EngineConfig::SetServerPort(PortType value) {
  server_port_ = value;
}
