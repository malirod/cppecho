// Copyright [2017] <Malinovsky Rodion>

#include "core/async_proxy.h"
#include "core/async_runner.h"
#include "core/default_scheduler_accessor.h"
#include "core/thread_pool.h"
#include "gtest/gtest.h"
#include "util/logger.h"
#include "util/thread_util.h"

#include "boost/thread/barrier.hpp"

using cppecho::core::ThreadPool;
using cppecho::core::AsyncRunner;
using cppecho::core::RunAsync;
using cppecho::core::WaitAll;
using cppecho::core::GetDefaultSchedulerAccessorInstance;
using cppecho::core::AsyncProxy;
using cppecho::util::ThreadUtil;

namespace {

class Foo {
 public:
  template <typename T>
  void RunCallback(T callback) {
    callback(val_);
  }

 private:
  int val_ = 10;
};

}  // namespace

TEST(TestAsyncProxy, RunObjMethodsInAnotherContext) {
  ThreadPool thread_pool_main{1u, "main"};
  ThreadPool thread_pool_net{1u, "net"};
  GetDefaultSchedulerAccessorInstance().Attach(thread_pool_main);
  const auto main_thread_name = thread_pool_main.GetName();
  const auto net_thread_name = thread_pool_net.GetName();

  std::atomic_int value{0};

  AsyncProxy<Foo>().Attach(thread_pool_net);

  boost::barrier barrier{2};
  RunAsync([&]() {
    ASSERT_EQ(main_thread_name, ThreadUtil::GetCurrentThreadName());
    barrier.wait();

    AsyncProxy<Foo>()->RunCallback([&](int& val) {
      ASSERT_EQ(val, 10);
      ASSERT_EQ(net_thread_name, ThreadUtil::GetCurrentThreadName());
      ++val;
    });

    AsyncProxy<Foo>()->RunCallback([&](int& val) {
      ASSERT_EQ(val, 11);
      ASSERT_EQ(net_thread_name, ThreadUtil::GetCurrentThreadName());
    });
  });

  barrier.wait();
  WaitAll();
}
