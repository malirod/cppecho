// Copyright [2016] <Malinovsky Rodion>

#pragma once

#include "core/alias.h"
#include "core/iioservice.h"
#include "core/ischeduler.h"

namespace cppecho {
namespace core {

class SequentialScheduler : public IScheduler {
 public:
  ~SequentialScheduler() = default;

  explicit SequentialScheduler(IIoService& service,
                               const char* name = "Sequential");

  void Schedule(HandlerType handler) override;

  const char* GetName() const override;

 private:
  AsioServiceStrandType strand_;

  const char* strand_name_;
};

}  // namespace core
}  // namespace cppecho
