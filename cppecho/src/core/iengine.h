// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include <boost/signals2.hpp>

namespace rms {
namespace core {

/**
 * Interface for Engine. Holds all and runs all business logic.
 */
class IEngine {
 public:
  /**
   * Destructor.
   */
  virtual ~IEngine() = default;

  /**
   * Start Engine. Non-blocking call. Actual startup will be performed asynchronously.
   * @return True if started. False otherwise.
   */
  virtual bool Start() = 0;

  /**
   * Trigger stop sequence. Non-blocking.
   * @return True if stop sequence has been started. False otherwise.
   */
  virtual bool Stop() = 0;

  /**
   * Init Engine. Blocking call.
   * @return True if initiated and ready to go. False otherwise.
   */
  virtual bool Init() = 0;

  using OnStartedType = boost::signals2::signal<void()>;
  using OnStartedSubsriberType = OnStartedType::slot_type;
  /**
   * Subscribe for event when Engine has been stated and become fully operable.
   * @param subscriber Observer to trigger when event occurs.
   * @return Observer connection.
   */
  virtual boost::signals2::connection SubscribeOnStarted(const OnStartedSubsriberType& subscriber) = 0;

  using OnStoppedType = boost::signals2::signal<void()>;
  using OnStoppedSubsriberType = OnStoppedType::slot_type;
  /**
   * Subscribe for event when Engine has stopped.
   * @param subscriber Observer to trigger when event occurs.
   * @return Observer connection.
   */
  virtual boost::signals2::connection SubscribeOnStopped(const OnStoppedSubsriberType& subscriber) = 0;
};

}  // namespace core
}  // namespace rms
