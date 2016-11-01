// Copyright [2016] <Malinovsky Rodion>

#include "model/engine_config.h"

namespace {

const char* const kDefaultListenAddress = "0.0.0.0";
const cppecho::model::PortType kDefaultListenPort = 8088u;

}  // namespace

cppecho::model::EngineConfig::EngineConfig()
    : server_address_(kDefaultListenAddress)
    , server_port_(kDefaultListenPort) {}

const std::string& cppecho::model::EngineConfig::GetServerAddress() const {
  return server_address_;
}

cppecho::model::PortType cppecho::model::EngineConfig::GetServerPort() const {
  return server_port_;
}

void cppecho::model::EngineConfig::SetServerAddress(const std::string& value) {
  server_address_ = value;
}

void cppecho::model::EngineConfig::SetServerPort(PortType value) {
  server_port_ = value;
}
