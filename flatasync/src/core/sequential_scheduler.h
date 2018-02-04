// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include "core/alias.h"
#include "core/ischeduler.h"

namespace rms {
namespace core {
class IIoService;
}
}  // namespace rms

namespace rms {
namespace core {

class SequentialScheduler : public IScheduler {
 public:
  ~SequentialScheduler() = default;

  explicit SequentialScheduler(IIoService& service, const char* name = "Sequential");

  void Schedule(HandlerType handler) override;

  const char* GetName() const override;

 private:
  AsioServiceStrandType strand_;

  const char* strand_name_;
};

}  // namespace core
}  // namespace rms
