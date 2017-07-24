// Copyright [2017] <Malinovsky Rodion>

#include <deque>

#include "core/async.h"
#include "core/default_scheduler_accessor.h"
#include "core/engine.h"
#include "core/engine_config.h"
#include "core/thread_pool.h"
#include "gtest/gtest.h"
#include "net/socket.h"
#include "net/util.h"
#include "util/logger.h"
#include "util/smartptr_util.h"

DECLARE_GLOBAL_GET_LOGGER("Test.Core.Engine");

namespace {

using cppecho::core::ThreadPool;
using cppecho::core::RunAsync;
using cppecho::core::IEngine;
using cppecho::core::Engine;
using cppecho::util::make_unique;
using cppecho::net::GetNetworkServiceAccessorInstance;
using cppecho::net::GetNetworkSchedulerAccessorInstance;
using cppecho::core::GetDefaultIoServiceAccessorInstance;
using cppecho::core::GetDefaultSchedulerAccessorInstance;
using cppecho::core::EngineConfig;
using cppecho::core::WaitAll;
using cppecho::net::Socket;
using cppecho::net::BufferType;

const char SERVER_ADDRESS[] = "127.0.0.1";

const int SERVER_PORT = 10124;

const char GREETING[] = "Hello World!!!\n";

class TestEngine : public ::testing::Test {
 public:
  TestEngine();

  ~TestEngine() override;

 protected:
  void LaunchEngine();

  void LaunchClient();

  std::unique_ptr<ThreadPool> thread_pool_net_;

  std::unique_ptr<ThreadPool> thread_pool_main_;

  std::unique_ptr<IEngine> engine_;

  std::atomic_int execution_step_{0};

  std::deque<std::unique_ptr<Socket>> client_connections_;
};

TestEngine::TestEngine() {
  LOG_DEBUG("Setup");

  const auto hardware_threads_count = std::thread::hardware_concurrency();
  const int thread_pool_size =
      hardware_threads_count >= 2 ? hardware_threads_count : 2;

  thread_pool_net_ = make_unique<ThreadPool>(thread_pool_size, "net");

  thread_pool_main_ = make_unique<ThreadPool>(thread_pool_size, "main");

  GetDefaultIoServiceAccessorInstance().Attach(*thread_pool_main_);
  GetDefaultSchedulerAccessorInstance().Attach(*thread_pool_main_);

  GetNetworkServiceAccessorInstance().Attach(*thread_pool_net_);
  GetNetworkSchedulerAccessorInstance().Attach(*thread_pool_net_);

  auto engine_config = make_unique<EngineConfig>();
  engine_config->SetServerAddress(SERVER_ADDRESS);
  engine_config->SetServerPort(SERVER_PORT);

  engine_ = make_unique<Engine>(std::move(engine_config));

  const auto initiated = engine_->Init();
  EXPECT_TRUE(initiated);
}

TestEngine::~TestEngine() {
  LOG_DEBUG("Teardown");

  GetNetworkServiceAccessorInstance().Detach();
  GetNetworkSchedulerAccessorInstance().Detach();

  GetDefaultIoServiceAccessorInstance().Detach();
  GetDefaultSchedulerAccessorInstance().Detach();

  thread_pool_net_.reset();
  thread_pool_main_.reset();
}

void TestEngine::LaunchEngine() {
  const auto launched = engine_->Start();
  ASSERT_TRUE(launched);
}

void TestEngine::LaunchClient() {
  RunAsync(
      [&] {
        Socket socket;
        socket.Connect(SERVER_ADDRESS, SERVER_PORT);
        execution_step_++;

        BufferType snd_buffer{GREETING};
        socket.Write(snd_buffer);
        execution_step_++;

        const auto rcv_buffer = socket.ReadUntil("\n");
        ASSERT_EQ("echo: " + snd_buffer, rcv_buffer);
        execution_step_++;

        engine_->Stop();
        execution_step_++;
      },
      *thread_pool_net_);
}

}  // namespace

TEST_F(TestEngine, EngineEchoTest) {
  LOG_AUTO_TRACE();

  engine_->SubscribeOnStarted([&]() {
    LaunchClient();
    execution_step_++;
  });

  LaunchEngine();

  WaitAll();

  ASSERT_EQ(5, execution_step_);
}
