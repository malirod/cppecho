// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include <memory>

namespace rms {
namespace core {

class ThreadPool;

class SchedulersInitiator {
 public:
  SchedulersInitiator();

  ~SchedulersInitiator();

 private:
  std::unique_ptr<ThreadPool> thread_pool_net_;

  std::unique_ptr<ThreadPool> thread_pool_main_;
};

}  // namespace core
}  // namespace rms
