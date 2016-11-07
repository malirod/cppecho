// Copyright [2016] <Malinovsky Rodion>

#include "core/async.h"
#include "core/async_runner.h"
#include "core/default_scheduler_accessor.h"
#include "core/thread_pool.h"
#include "gtest/gtest.h"
#include "util/logger.h"
#include "util/thread_util.h"

using cppecho::core::ThreadPool;
using cppecho::core::AsyncRunner;
using cppecho::core::AsyncOpStatus;
using cppecho::core::RunAsync;
using cppecho::core::WaitAll;
using cppecho::core::GetDefaultSchedulerAccessorInstance;
using cppecho::util::ThreadUtil;

DECLARE_GLOBAL_GET_LOGGER("Core.AsyncRunnerTest")

namespace {

auto GetAsioService(cppecho::core::IIoService& io_service)
    -> decltype(io_service.GetAsioService()) {
  return io_service.GetAsioService();
}

}  // namespace

TEST(TestAsyncRunner, RunAsyncOnce) {
  ThreadPool thread_pool_main{1u, "main"};
  ThreadPool thread_pool_net{1u, "net"};
  GetDefaultSchedulerAccessorInstance().Attach(thread_pool_main);
  const auto main_thread_name = thread_pool_main.GetName();

  RunAsync([&]() {
    ASSERT_EQ(main_thread_name, ThreadUtil::GetCurrentThreadName());
  });

  WaitAll();
}

TEST(TestAsyncRunner, RunAsyncAndSwitch) {
  LOG_INFO("!!! Test ----- Init");

  // setup
  ThreadPool thread_pool_main{1u, "main"};
  const auto main_thread_name = thread_pool_main.GetName();
  ThreadPool thread_pool_net{1u, "net"};
  GetDefaultSchedulerAccessorInstance().Attach(thread_pool_main);

  LOG_INFO("!!! ----- Test Start");
  RunAsync([&]() {
    LOG_INFO("!!! 1.1 Enter: " << ThreadUtil::GetCurrentThreadId());
    ASSERT_EQ(main_thread_name, ThreadUtil::GetCurrentThreadName());

    LOG_INFO("!!! Switching to thread_pool_net");
    SwitchTo(thread_pool_net);
    LOG_INFO("Switched to thread_pool_net");

    LOG_INFO("!!! 1.1 Leave: " << ThreadUtil::GetCurrentThreadId());
  });

  LOG_INFO("!!! Waiting ----- all");
  WaitAll();
  LOG_INFO("!!! Test ----- End");
}
