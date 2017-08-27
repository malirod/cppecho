// Copyright [2016] <Malinovsky Rodion>

#pragma once

#include <boost/signals2.hpp>

namespace cppecho {
namespace core {

class IEngine {
 public:
  virtual ~IEngine() = default;

  virtual bool Start() = 0;

  virtual bool Stop() = 0;

  virtual bool Init() = 0;

  using OnStartedType = boost::signals2::signal<void()>;
  using OnStartedSubsriberType = OnStartedType::slot_type;
  virtual boost::signals2::connection SubscribeOnStarted(
      const OnStartedSubsriberType& subscriber) = 0;

  using OnStoppedType = boost::signals2::signal<void()>;
  using OnStoppedSubsriberType = OnStoppedType::slot_type;
  virtual boost::signals2::connection SubscribeOnStopped(
      const OnStoppedSubsriberType& subscriber) = 0;
};

}  // namespace core
}  // namespace cppecho
