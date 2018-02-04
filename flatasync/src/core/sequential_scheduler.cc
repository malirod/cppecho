// Copyright [2018] <Malinovsky Rodion>

#include "core/sequential_scheduler.h"

#include <type_traits>
#include <utility>
#include "core/iioservice.h"

rms::core::SequentialScheduler::SequentialScheduler(IIoService& service, const char* name)
    : strand_(service.GetAsioService()), strand_name_(name) {}

void rms::core::SequentialScheduler::Schedule(HandlerType handler) {
  strand_.post(std::move(handler));
}

const char* rms::core::SequentialScheduler::GetName() const {
  return strand_name_;
}
