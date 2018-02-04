// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include <atomic>
#include <boost/signals2/connection.hpp>
#include <memory>
#include "core/iengine.h"
#include "util/logger.h"

namespace rms {
namespace core {

class IEngineConfig;
class SequentialScheduler;

}  // namespace core
}  // namespace rms

namespace rms {
namespace net {

class TcpServer;

}  // namespace net
}  // namespace rms

namespace rms {
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

  boost::signals2::connection SubscribeOnStarted(const OnStartedSubsriberType& subscriber) override;

  boost::signals2::connection SubscribeOnStopped(const OnStoppedSubsriberType& subscriber) override;

 private:
  DECLARE_GET_LOGGER("Core.Engine")

  bool initiated_ = false;

  std::unique_ptr<core::IEngineConfig> engine_config_;

  std::unique_ptr<core::SequentialScheduler> main_sequential_scheduler_;

  std::unique_ptr<net::TcpServer> tcp_server_;

  std::atomic_bool stopped_{false};

  OnStartedType on_started_;

  OnStoppedType on_stopped_;
};

}  // namespace core
}  // namespace rms
