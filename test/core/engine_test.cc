// Copyright [2017] <Malinovsky Rodion>

#include <deque>

#include "core/async.h"
#include "core/default_scheduler_accessor.h"
#include "core/engine.h"
#include "core/engine_config.h"
#include "core/helper.h"
#include "core/sequential_scheduler.h"
#include "core/thread_pool.h"
#include "gtest/gtest.h"
#include "net/tcp_socket.h"
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
using cppecho::core::GetDefaultIoServiceAccessorInstance;
using cppecho::core::EngineConfig;
using cppecho::core::WaitAll;
using cppecho::net::TcpSocket;
using cppecho::net::BufferType;
using cppecho::core::SchedulersInitiator;
using cppecho::core::SequentialScheduler;

const char SERVER_ADDRESS[] = "127.0.0.1";

const int SERVER_PORT = 10124;

const char GREETING[] = "Hello World!!!\n";

}  // namespace

TEST(TestEngine, EngineEchoTest) {
  LOG_AUTO_TRACE();

  auto schedulers_initiator = make_unique<SchedulersInitiator>();
  std::atomic_int execution_step{0};

  auto engine_config = make_unique<EngineConfig>();
  engine_config->SetServerAddress(SERVER_ADDRESS);
  engine_config->SetServerPort(SERVER_PORT);

  auto engine = make_unique<Engine>(std::move(engine_config));

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
