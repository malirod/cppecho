// Copyright [2018] <Malinovsky Rodion>

#include "core/engine.h"
#include <gtest/gtest.h>
#include "core/async.h"
#include "core/default_scheduler_accessor.h"
#include "core/engine_config.h"
#include "core/thread_pool.h"
#include "net/tcp_socket.h"
#include "net/util.h"
#include "util/logger.h"

#include <algorithm>
#include <atomic>
#include <memory>
#include "net/alias.h"

DECLARE_GLOBAL_GET_LOGGER("Test.Core.Engine")

namespace {

using rms::core::Engine;
using rms::core::EngineConfig;
using rms::core::GetDefaultIoServiceAccessorInstance;
using rms::core::GetDefaultSchedulerAccessorInstance;
using rms::core::ThreadPool;
using rms::core::WaitAll;
using rms::net::BufferType;
using rms::net::GetNetworkSchedulerAccessorInstance;
using rms::net::GetNetworkServiceAccessorInstance;
using rms::net::TcpSocket;

const char SERVER_ADDRESS[] = "127.0.0.1";

const int SERVER_PORT = 10124;

const char GREETING[] = "Hello World!!!\n";

}  // namespace

TEST(TestEngine, EngineEchoTest) {
  LOG_AUTO_TRACE();

  const auto hardware_threads_count = std::thread::hardware_concurrency();
  const int thread_pool_size = hardware_threads_count * 2;

  ThreadPool thread_pool_net(thread_pool_size, "net");
  ThreadPool thread_pool_main(thread_pool_size, "main");

  GetDefaultIoServiceAccessorInstance().Attach(thread_pool_main);
  GetDefaultSchedulerAccessorInstance().Attach(thread_pool_main);

  GetNetworkServiceAccessorInstance().Attach(thread_pool_net);
  GetNetworkSchedulerAccessorInstance().Attach(thread_pool_net);

  std::atomic_int execution_step{0};

  auto engine_config = std::make_unique<EngineConfig>();
  engine_config->SetServerAddress(SERVER_ADDRESS);
  engine_config->SetServerPort(SERVER_PORT);

  auto engine = std::make_unique<Engine>(std::move(engine_config));

  const auto initiated = engine->Init();
  EXPECT_TRUE(initiated);

  engine->SubscribeOnStarted([&]() {
    auto socket = TcpSocket::Create();
    socket->Connect(SERVER_ADDRESS, SERVER_PORT);
    ++execution_step;

    BufferType snd_buffer{GREETING};
    socket->Write(snd_buffer);
    ++execution_step;

    const auto rcv_buffer = socket->ReadUntil("\n");
    ASSERT_EQ("echo: " + snd_buffer, rcv_buffer);
    ++execution_step;

    engine->Stop();
    ++execution_step;
  });

  const auto launched = engine->Start();
  ASSERT_TRUE(launched);

  WaitAll();

  ASSERT_EQ(4, execution_step);
}
