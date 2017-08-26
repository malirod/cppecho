// Copyright [2017] <Malinovsky Rodion>

#include "core/helper.h"
#include "core/thread_pool.h"
#include "net/util.h"
#include "util/smartptr_util.h"

#include "core/default_scheduler_accessor.h"

using cppecho::util::make_unique;
using cppecho::net::GetNetworkServiceAccessorInstance;
using cppecho::net::GetNetworkSchedulerAccessorInstance;
using cppecho::core::GetDefaultIoServiceAccessorInstance;
using cppecho::core::GetDefaultSchedulerAccessorInstance;
using cppecho::core::ThreadPool;

cppecho::core::SchedulersInitiator::SchedulersInitiator() {
  const auto hardware_threads_count = std::thread::hardware_concurrency();
  const int thread_pool_size = hardware_threads_count * 2;

  thread_pool_net_ = make_unique<ThreadPool>(thread_pool_size, "net");

  thread_pool_main_ = make_unique<ThreadPool>(thread_pool_size, "main");

  GetDefaultIoServiceAccessorInstance().Attach(*thread_pool_main_);
  GetDefaultSchedulerAccessorInstance().Attach(*thread_pool_main_);

  GetNetworkServiceAccessorInstance().Attach(*thread_pool_net_);
  GetNetworkSchedulerAccessorInstance().Attach(*thread_pool_net_);
}

cppecho::core::SchedulersInitiator::~SchedulersInitiator() {
  GetNetworkServiceAccessorInstance().Detach();
  GetNetworkSchedulerAccessorInstance().Detach();

  GetDefaultIoServiceAccessorInstance().Detach();
  GetDefaultSchedulerAccessorInstance().Detach();

  thread_pool_net_.reset();
  thread_pool_main_.reset();
}
