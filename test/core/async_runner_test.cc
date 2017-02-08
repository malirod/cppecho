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

  int counter = 0;

  RunAsync([&]() {
    ASSERT_EQ(main_thread_name, ThreadUtil::GetCurrentThreadName());
    ++counter;
  });
  ASSERT_EQ(0, counter);

  WaitAll();
  ASSERT_EQ(1, counter);
}

TEST(TestAsyncRunner, RunAsyncAndSwitch) {
  ThreadPool thread_pool_main{1u, "main"};
  ThreadPool thread_pool_net{1u, "net"};
  GetDefaultSchedulerAccessorInstance().Attach(thread_pool_main);
  const auto main_thread_name = thread_pool_main.GetName();
  const auto net_thread_name = thread_pool_net.GetName();

  int counter = 0;
  RunAsync([&]() {
    ASSERT_EQ(main_thread_name, ThreadUtil::GetCurrentThreadName());
    ++counter;

    SwitchTo(thread_pool_net);

    ASSERT_EQ(net_thread_name, ThreadUtil::GetCurrentThreadName());
    ++counter;
  });

  ASSERT_EQ(0, counter);
  WaitAll();
  ASSERT_EQ(2, counter);
}
