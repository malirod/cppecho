// Copyright [2017] <Malinovsky Rodion>

#pragma once

#include <memory>

namespace cppecho {
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
}  // namespace cppecho
