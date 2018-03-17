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

/**
 * Implementation of Engine. Holds all and runs all business logic.
 */
class Engine : public IEngine {
 public:
  /**
   * Creates instance of Engine initiated with IEngineConfig.
   * @param engine_config Engine configuration.
   */
  explicit Engine(std::unique_ptr<core::IEngineConfig> engine_config);

  Engine& operator=(const Engine&) = delete;
  Engine(const Engine&) = delete;

  /**
   * Destroy Engine instance. Mark Engine as stopped. Don't perform actual shutdown.
   */
  ~Engine() override;

  /**
   * Start Engine. Non-blocking call. Actual startup will be performed asynchronously.
   * @return True if started. False otherwise.
   */
  bool Start() override;

  /**
   * Trigger stop sequence. Non-blocking.
   * @return True if stop sequence has been started. False otherwise.
   */
  bool Stop() override;

  /**
   * Init Engine. Blocking call.
   * @return True if initiated and ready to go. False otherwise.
   */
  bool Init() override;

  /**
   * Subscribe for event when Engine has been stated and become fully operable.
   * @param subscriber Observer to trigger when event occurs.
   * @return Observer connection.
   */
  boost::signals2::connection SubscribeOnStarted(const OnStartedSubsriberType& subscriber) override;

  /**
   * Subscribe for event when Engine has stopped.
   * @param subscriber Observer to trigger when event occurs.
   * @return Observer connection.
   */
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
