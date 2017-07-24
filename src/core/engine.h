// Copyright [2016] <Malinovsky Rodion>

#pragma once

#include <deque>
#include <memory>
#include <string>
#include "core/iengine.h"
#include "util/logger.h"

namespace cppecho {
namespace core {

class IEngineConfig;

}  // namespace core
}  // namespace cppecho

namespace cppecho {
namespace net {

class Socket;
class Acceptor;

}  // namespace net
}  // namespace cppecho

namespace cppecho {
namespace core {

class Engine : public IEngine {
 public:
  explicit Engine(std::unique_ptr<core::IEngineConfig> engine_config);

  Engine& operator=(const Engine&) = delete;
  Engine(const Engine&) = delete;

  ~Engine() override;

  bool Start() override;

  bool Stop() override;

  bool Init() override;

  boost::signals2::connection SubscribeOnStarted(
      const OnStartedSubsriberType& subscriber) override;

 private:
  DECLARE_GET_LOGGER("Core.Engine")

  void OnAccepted(std::unique_ptr<cppecho::net::Socket> accepted_socket);

  bool initiated_ = false;

  std::unique_ptr<core::IEngineConfig> engine_config_;

  std::atomic_bool stopped_{false};

  OnStartedType on_started_;

  std::unique_ptr<cppecho::net::Acceptor> acceptor_;

  std::deque<std::unique_ptr<cppecho::net::Socket>> client_connections_;
};

}  // namespace core
}  // namespace cppecho
