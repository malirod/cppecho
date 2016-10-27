// Copyright [2016] <Malinovsky Rodion>

#include "core/engine.h"

namespace {

const char* kDefaultListenAddress = "0.0.0.0";
const std::uint32_t kDefaultListenPort = 8088;

}  // namespace

cppecho::core::Engine::Engine(const std::string& address, std::uint32_t port)
    : address_(address), port_(port) {
  LOG_AUTO_TRACE();
  if (address_.empty()) {
    address_ = kDefaultListenAddress;
  }
  if (port_ == 0) {
    port_ = kDefaultListenPort;
  }
  LOG_INFO("Engine has been created.");
}

cppecho::core::Engine::~Engine() {
  LOG_AUTO_TRACE();
  Stop();
  LOG_INFO("Engine has been destroyed.");
}

bool cppecho::core::Engine::Start() {
  LOG_AUTO_TRACE();
  LOG_INFO("Starting engine. Listenig on " << address_ << ":" << port_);

  LOG_INFO("Engine has been started.");
  return true;
}

void cppecho::core::Engine::Stop() {
  LOG_AUTO_TRACE();
  LOG_INFO("Engine has been stopped.");
}

bool cppecho::core::Engine::Init() {
  LOG_AUTO_TRACE();
  return true;
}
