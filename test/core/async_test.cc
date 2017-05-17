// Copyright [2016] <Malinovsky Rodion>

#include "core/async.h"
#include "core/async_runner.h"
#include "core/default_scheduler_accessor.h"
#include "core/sequential_scheduler.h"
#include "core/thread_pool.h"
#include "gtest/gtest.h"
#include "util/logger.h"
#include "util/thread_util.h"

#include "boost/thread/barrier.hpp"

using cppecho::core::ThreadPool;
using cppecho::core::AsyncRunner;
using cppecho::core::AsyncOpStatus;
using cppecho::core::RunAsync;
using cppecho::core::WaitAll;
using cppecho::core::GetDefaultSchedulerAccessorInstance;
using cppecho::core::GetTimeoutServiceAccessorInstance;
using cppecho::core::SequentialScheduler;
using cppecho::core::HandleEvents;
using cppecho::core::Timeout;
using cppecho::util::ThreadUtil;
using cppecho::util::SleepFor;

DECLARE_GLOBAL_GET_LOGGER("Core.AsyncRunnerTest")

namespace {

auto GetAsioService(cppecho::core::IIoService& io_service)
    -> decltype(io_service.GetAsioService()) {
  return io_service.GetAsioService();
}

}  // namespace

TEST(TestAsync, RunAsyncOnce) {
  ThreadPool thread_pool_main{1u, "main"};
  GetDefaultSchedulerAccessorInstance().Attach(thread_pool_main);
  const auto main_thread_name = thread_pool_main.GetName();

  std::atomic<int> counter{0};
  boost::barrier barrier{2};

  RunAsync([&]() {
    ASSERT_EQ(main_thread_name, ThreadUtil::GetCurrentThreadName());
    barrier.wait();
    ++counter;
  });
  ASSERT_EQ(0, counter);
  barrier.wait();

  WaitAll();
  ASSERT_EQ(1, counter);
}

TEST(TestAsync, RunAsyncAndSwitch) {
  ThreadPool thread_pool_main{1u, "main"};
  ThreadPool thread_pool_net{1u, "net"};
  GetDefaultSchedulerAccessorInstance().Attach(thread_pool_main);
  const auto main_thread_name = thread_pool_main.GetName();
  const auto net_thread_name = thread_pool_net.GetName();

  std::atomic<int> counter{0};
  boost::barrier barrier{2};
  RunAsync([&]() {
    ASSERT_EQ(main_thread_name, ThreadUtil::GetCurrentThreadName());
    barrier.wait();
    ++counter;

    SwitchTo(thread_pool_net);

    ASSERT_EQ(net_thread_name, ThreadUtil::GetCurrentThreadName());
    ++counter;

    SwitchTo(thread_pool_main);
    ASSERT_EQ(main_thread_name, ThreadUtil::GetCurrentThreadName());
    ++counter;

    SwitchTo(thread_pool_main);
    ASSERT_EQ(main_thread_name, ThreadUtil::GetCurrentThreadName());
    ++counter;
  });

  ASSERT_EQ(0, counter);
  barrier.wait();
  WaitAll();
  ASSERT_EQ(4, counter);
}

TEST(TestAsync, RunAsyncAndException) {
  ThreadPool thread_pool_main{1u, "main"};
  ThreadPool thread_pool_net{1u, "net"};
  GetDefaultSchedulerAccessorInstance().Attach(thread_pool_main);
  const auto main_thread_name = thread_pool_main.GetName();
  const auto net_thread_name = thread_pool_net.GetName();

  std::atomic<int> counter{0};
  boost::barrier barrier{2};
  RunAsync([&]() {
    ASSERT_EQ(main_thread_name, ThreadUtil::GetCurrentThreadName());
    barrier.wait();
    ++counter;

    try {
      throw std::runtime_error("Test");
    } catch (std::exception& e) {
      SwitchTo(thread_pool_net);
      ASSERT_EQ(net_thread_name, ThreadUtil::GetCurrentThreadName());
    }
    SwitchTo(thread_pool_main);

    ASSERT_EQ(main_thread_name, ThreadUtil::GetCurrentThreadName());
    ++counter;
  });

  ASSERT_EQ(0, counter);
  barrier.wait();
  WaitAll();
  ASSERT_EQ(2, counter);
}

TEST(TestAsync, RunAsyncWait) {
  ThreadPool thread_pool_main{1u, "main"};
  GetDefaultSchedulerAccessorInstance().Attach(thread_pool_main);
  const auto main_thread_name = thread_pool_main.GetName();

  std::atomic<int> counter{0};
  boost::barrier barrier{2};

  RunAsync([&]() {
    ASSERT_EQ(main_thread_name, ThreadUtil::GetCurrentThreadName());
    barrier.wait();
    ++counter;
    cppecho::core::RunAsyncWait(
        {[&] {
           ASSERT_EQ(main_thread_name, ThreadUtil::GetCurrentThreadName());
           SleepFor(1);
           ++counter;
         },
         [&] {
           ASSERT_EQ(main_thread_name, ThreadUtil::GetCurrentThreadName());
           SleepFor(2);
           ++counter;
         }});
    ASSERT_EQ(3, counter);
    ++counter;
  });

  ASSERT_EQ(0, counter);
  barrier.wait();

  WaitAll();
  ASSERT_EQ(4, counter);
}

TEST(TestAsync, RunAsyncWaitWithWaiter) {
  ThreadPool thread_pool_main{1u, "main"};
  GetDefaultSchedulerAccessorInstance().Attach(thread_pool_main);
  const auto main_thread_name = thread_pool_main.GetName();

  std::atomic<int> counter{0};
  boost::barrier barrier{2};

  RunAsync([&]() {
    ASSERT_EQ(main_thread_name, ThreadUtil::GetCurrentThreadName());
    barrier.wait();
    ++counter;

    cppecho::core::Waiter waiter;
    waiter.Wait();
    waiter
        .RunAsync([&] {
          ASSERT_EQ(main_thread_name, ThreadUtil::GetCurrentThreadName());
          SleepFor(1);
          ++counter;
        })
        .RunAsync([&] {
          ASSERT_EQ(main_thread_name, ThreadUtil::GetCurrentThreadName());
          SleepFor(2);
          ++counter;
        });
    ASSERT_EQ(1, counter);

    waiter.Wait();
    ASSERT_EQ(3, counter);
    ++counter;
  });

  ASSERT_EQ(0, counter);
  barrier.wait();

  WaitAll();
  ASSERT_EQ(4, counter);
}

TEST(TestAsync, RunAsyncAnyWait) {
  ThreadPool thread_pool_main{3u, "main"};
  GetDefaultSchedulerAccessorInstance().Attach(thread_pool_main);
  const auto main_thread_name = thread_pool_main.GetName();

  std::atomic<int> counter{0};
  boost::barrier barrier{2};

  RunAsync([&]() {
    ASSERT_EQ(main_thread_name, ThreadUtil::GetCurrentThreadName());
    barrier.wait();
    ++counter;

    std::size_t index = cppecho::core::RunAsyncAnyWait(
        {[&] {
           ASSERT_EQ(main_thread_name, ThreadUtil::GetCurrentThreadName());
           SleepFor(200);
         },
         [&] {
           ASSERT_EQ(main_thread_name, ThreadUtil::GetCurrentThreadName());
           SleepFor(1);
           ++counter;
         }});
    ASSERT_EQ(1u, index);
  });

  ASSERT_EQ(0, counter);
  barrier.wait();

  WaitAll();
  ASSERT_EQ(2, counter);
}

TEST(TestAsync, RunAsyncAnyResult_RunInParallelWithMultipleThreads) {
  ThreadPool thread_pool_main{3u, "main"};
  GetDefaultSchedulerAccessorInstance().Attach(thread_pool_main);
  const auto main_thread_name = thread_pool_main.GetName();

  std::atomic<int> counter{0};
  boost::barrier barrier{2};

  RunAsync([&]() {
    ASSERT_EQ(main_thread_name, ThreadUtil::GetCurrentThreadName());
    barrier.wait();
    ++counter;

    const auto result = cppecho::core::RunAsyncAnyResult<int>(
        {[&] {
           SleepFor(200);
           return boost::optional<int>(200);
         },
         [&] {
           SleepFor(1);
           ++counter;
           return boost::optional<int>(100);
         }});
    ASSERT_EQ(100, result);
  });

  ASSERT_EQ(0, counter);
  barrier.wait();

  WaitAll();
  ASSERT_EQ(2, counter);
}

TEST(TestAsync, RunAsyncAnyResult_RunSequentiallyWithSingleThread) {
  ThreadPool thread_pool_main{1u, "main"};
  GetDefaultSchedulerAccessorInstance().Attach(thread_pool_main);
  const auto main_thread_name = thread_pool_main.GetName();

  std::atomic<int> counter{0};
  boost::barrier barrier{2};

  RunAsync([&]() {
    ASSERT_EQ(main_thread_name, ThreadUtil::GetCurrentThreadName());
    barrier.wait();
    ++counter;

    const auto result = cppecho::core::RunAsyncAnyResult<int>(
        {[&] {
           SleepFor(200);
           ++counter;
           return boost::optional<int>(200);
         },
         [&] {
           SleepFor(1);
           return boost::optional<int>(100);
         }});
    ASSERT_EQ(200, result);
  });

  ASSERT_EQ(0, counter);
  barrier.wait();

  WaitAll();
  ASSERT_EQ(2, counter);
}

TEST(TestAsync, RunAsyncAnyResult_RunInParallelImplicitConversion) {
  ThreadPool thread_pool_main{3u, "main"};
  GetDefaultSchedulerAccessorInstance().Attach(thread_pool_main);
  const auto main_thread_name = thread_pool_main.GetName();

  std::atomic<int> counter{0};
  boost::barrier barrier{2};

  RunAsync([&]() {
    ASSERT_EQ(main_thread_name, ThreadUtil::GetCurrentThreadName());
    barrier.wait();
    ++counter;

    const auto result = cppecho::core::RunAsyncAnyResult<int>(
        {[&] {
           SleepFor(200);
           return boost::optional<int>(200);
         },
         [&] {
           SleepFor(1);
           ++counter;
           return 100;
         }});
    ASSERT_EQ(100, result);
  });

  ASSERT_EQ(0, counter);
  barrier.wait();

  WaitAll();
  ASSERT_EQ(2, counter);
}

TEST(TestAsync, RunAsyncAnyResult_RunInParallelNoneValueIgnored) {
  ThreadPool thread_pool_main{3u, "main"};
  GetDefaultSchedulerAccessorInstance().Attach(thread_pool_main);
  const auto main_thread_name = thread_pool_main.GetName();

  std::atomic<int> counter{0};
  boost::barrier barrier{2};

  RunAsync([&]() {
    ASSERT_EQ(main_thread_name, ThreadUtil::GetCurrentThreadName());
    barrier.wait();
    ++counter;

    const auto result = cppecho::core::RunAsyncAnyResult<int>(
        {[&] {
           SleepFor(200);
           return boost::optional<int>(200);
         },
         [&] {
           SleepFor(1);
           ++counter;
           return boost::optional<int>();
         }});
    ASSERT_EQ(200, result);
  });

  ASSERT_EQ(0, counter);
  barrier.wait();

  WaitAll();
  ASSERT_EQ(2, counter);
}

TEST(TestAsync, RunAsyncSequential) {
  ThreadPool thread_pool_main{3u, "main"};
  SequentialScheduler sequential_scheduler(thread_pool_main);
  const auto main_thread_name = thread_pool_main.GetName();

  std::atomic<int> counter{0};
  boost::barrier barrier{2};

  RunAsync(
      [&]() {
        ASSERT_EQ(main_thread_name, ThreadUtil::GetCurrentThreadName());
        barrier.wait();
        counter = 1;

        RunAsync(
            [&] {
              ASSERT_LT(counter, 10);
              counter += 10;
              SleepFor(100);
              counter += 10;
              ASSERT_LT(counter, 30);
              ASSERT_GT(counter, 20);
            },
            sequential_scheduler);

        ++counter;
        RunAsync(
            [&] {
              ASSERT_LT(counter, 100);
              counter += 100;
              SleepFor(100);
              counter += 100;
              ASSERT_LT(counter, 300);
              ASSERT_GT(counter, 200);
            },
            sequential_scheduler);

        ++counter;
        SwitchTo(sequential_scheduler);
        ASSERT_LT(counter, 300);
        ASSERT_GT(counter, 200);
        counter += 1000;
      },
      thread_pool_main);

  ASSERT_EQ(0, counter);
  barrier.wait();

  WaitAll();
  ASSERT_EQ(1223, counter);
}

TEST(TestAsync, RunAsyncTimeout) {
  ThreadPool thread_pool_main{3u, "main"};
  GetDefaultSchedulerAccessorInstance().Attach(thread_pool_main);
  GetTimeoutServiceAccessorInstance().Attach(thread_pool_main);

  const auto main_thread_name = thread_pool_main.GetName();

  std::atomic<int> counter{0};

  const auto normal_state = RunAsync([&]() {
    // This async task should not timeout
    ASSERT_EQ(main_thread_name, ThreadUtil::GetCurrentThreadName());
    ++counter;
    Timeout timeout(100);
    ++counter;
    HandleEvents();
    ++counter;
    SleepFor(50);
    ++counter;
    HandleEvents();
    ++counter;
  });

  const auto timedout_state = RunAsync([&]() {
    // This async task should timeout and break after last  HandleEvents
    ASSERT_EQ(main_thread_name, ThreadUtil::GetCurrentThreadName());
    ++counter;
    Timeout timeout(50);
    ++counter;
    HandleEvents();
    ++counter;
    SleepFor(100);
    ++counter;
    HandleEvents();  // This line should catch timeout and break execution
    ++counter;       // this inc should not happen
  });

  WaitAll();
  ASSERT_EQ(AsyncOpStatus::Normal, normal_state.GetStatus());
  ASSERT_EQ(AsyncOpStatus::Timedout, timedout_state.GetStatus());
  ASSERT_EQ(9, counter);
}

TEST(TestAsync, RunAsyncTimeoutThrowBeforeHandling) {
  ThreadPool thread_pool_main{3u, "main"};
  GetDefaultSchedulerAccessorInstance().Attach(thread_pool_main);
  GetTimeoutServiceAccessorInstance().Attach(thread_pool_main);

  const auto main_thread_name = thread_pool_main.GetName();

  std::atomic<int> counter{0};

  const auto timedout_state = RunAsync([&]() {
    ASSERT_EQ(main_thread_name, ThreadUtil::GetCurrentThreadName());
    ++counter;
    Timeout timeout(50);
    ++counter;
    HandleEvents();
    ++counter;
    SleepFor(100);
    ++counter;
    throw std::runtime_error("TEST");
    HandleEvents();
    ++counter;
  });

  WaitAll();
  ASSERT_EQ(AsyncOpStatus::Timedout, timedout_state.GetStatus());
  ASSERT_EQ(4, counter);
}
