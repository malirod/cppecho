// Copyright [2018] <Malinovsky Rodion>

#include "core/helper.h"
#include <thread>
#include "core/thread_pool.h"
#include "net/util.h"

#include "core/default_scheduler_accessor.h"

using rms::core::GetDefaultIoServiceAccessorInstance;
using rms::core::GetDefaultSchedulerAccessorInstance;
using rms::core::ThreadPool;
using rms::net::GetNetworkSchedulerAccessorInstance;
using rms::net::GetNetworkServiceAccessorInstance;

rms::core::SchedulersInitiator::SchedulersInitiator() {
  const auto hardware_threads_count = std::thread::hardware_concurrency();
  const int thread_pool_size = hardware_threads_count * 2;

  thread_pool_net_ = std::make_unique<ThreadPool>(thread_pool_size, "net");

  thread_pool_main_ = std::make_unique<ThreadPool>(thread_pool_size, "main");

  GetDefaultIoServiceAccessorInstance().Attach(*thread_pool_main_);
  GetDefaultSchedulerAccessorInstance().Attach(*thread_pool_main_);

  GetNetworkServiceAccessorInstance().Attach(*thread_pool_net_);
  GetNetworkSchedulerAccessorInstance().Attach(*thread_pool_net_);
}

rms::core::SchedulersInitiator::~SchedulersInitiator() {
  GetNetworkServiceAccessorInstance().Detach();
  GetNetworkSchedulerAccessorInstance().Detach();

  GetDefaultIoServiceAccessorInstance().Detach();
  GetDefaultSchedulerAccessorInstance().Detach();

  thread_pool_net_.reset();
  thread_pool_main_.reset();
}
