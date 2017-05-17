// Copyright [2016] <Malinovsky Rodion>

#include "core/sequential_scheduler.h"

#include <utility>

cppecho::core::SequentialScheduler::SequentialScheduler(IIoService& service,
                                                        const char* name)
    : strand_(service.GetAsioService()), strand_name_(name) {}

void cppecho::core::SequentialScheduler::Schedule(HandlerType handler) {
  strand_.post(std::move(handler));
}

const char* cppecho::core::SequentialScheduler::GetName() const {
  return strand_name_;
}
