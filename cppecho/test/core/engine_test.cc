// Copyright [2018] <Malinovsky Rodion>

#include "core/engine.h"
#include <gtest/gtest.h>
#include "core/async.h"
#include "core/engine_config.h"
#include "core/helper.h"
#include "net/tcp_socket.h"
#include "util/logger.h"

#include <algorithm>
#include <atomic>
#include <memory>
#include "net/alias.h"

DECLARE_GLOBAL_GET_LOGGER("Test.Core.Engine");

namespace {

using rms::core::Engine;
using rms::core::EngineConfig;
using rms::core::SchedulersInitiator;
using rms::core::WaitAll;
using rms::net::BufferType;
using rms::net::TcpSocket;

const char SERVER_ADDRESS[] = "127.0.0.1";

const int SERVER_PORT = 10124;

const char GREETING[] = "Hello World!!!\n";

}  // namespace

TEST(TestEngine, EngineEchoTest) {
  LOG_AUTO_TRACE();

  auto schedulers_initiator = std::make_unique<SchedulersInitiator>();
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
