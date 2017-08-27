// Copyright [2017] <Malinovsky Rodion>

#include <deque>
#include <memory>

#include "core/async.h"
#include "core/default_scheduler_accessor.h"
#include "core/helper.h"
#include "core/thread_pool.h"
#include "net/acceptor.h"
#include "net/util.h"
#include "util/logger.h"
#include "util/smartptr_util.h"

#include "gtest/gtest.h"

DECLARE_GLOBAL_GET_LOGGER("Test.Net.TcpSocket");

namespace {

using cppecho::core::ThreadPool;
using cppecho::core::RunAsync;
using cppecho::core::WaitAll;
using cppecho::net::Acceptor;
using cppecho::net::BufferType;
using cppecho::net::TcpSocket;
using cppecho::util::make_unique;
using cppecho::net::GetNetworkSchedulerAccessorInstance;
using cppecho::core::SchedulersInitiator;

const int SERVER_PORT = 10123;

const char SERVER_ECHO_PREFIX[] = "echo: ";

const char GREETING[] = "Hello World!!!";

}  // namespace

TEST(TestTcpSocket, SocketEchoTest) {
  LOG_AUTO_TRACE();

  auto schedulers_initiator = make_unique<SchedulersInitiator>();
  std::atomic_int execution_step{0};

  RunAsync(
      [&] {
        ++execution_step;
        Acceptor acceptor(SERVER_PORT);
        LOG_DEBUG("Accepting connection on port " << SERVER_PORT);
        ++execution_step;

        RunAsync([&]() {
          acceptor.DoAccept([&](std::shared_ptr<TcpSocket> accepted_socket) {
            ASSERT_TRUE(accepted_socket);

            // Ensure that new connection has disbled Nagle's algorythm
            const auto socket_opts = accepted_socket->GetSocketOpts();
            ASSERT_TRUE(socket_opts.at(TcpSocket::SocketOpt::NoDelay));

            LOG_DEBUG("Accepted socket");
            ++execution_step;
            const auto rcv_buffer =
                accepted_socket->ReadExact(sizeof(GREETING) - 1);
            LOG_DEBUG("Server: received data: " << rcv_buffer);
            ++execution_step;
            BufferType snd_buffer{SERVER_ECHO_PREFIX};
            snd_buffer += rcv_buffer;
            LOG_DEBUG("Server: sending data: " << snd_buffer);
            accepted_socket->Write(snd_buffer);
            ++execution_step;
          });
        });

        auto socket = TcpSocket::Create();
        socket->Connect("127.0.0.1", SERVER_PORT);
        BufferType snd_buffer{GREETING};
        LOG_DEBUG("Client: sending data: " << snd_buffer);
        socket->Write(snd_buffer);
        ++execution_step;
        LOG_DEBUG("Client: reading reply from server");
        const auto rcv_buffer = socket->ReadExact(sizeof(SERVER_ECHO_PREFIX) -
                                                  1 + snd_buffer.length());
        LOG_DEBUG("Client: received data: " << rcv_buffer);
        ASSERT_EQ(SERVER_ECHO_PREFIX + snd_buffer, rcv_buffer);
        ++execution_step;
      },
      GetNetworkSchedulerAccessorInstance().GetRef());

  LOG_DEBUG("Waiting all async tasks");
  WaitAll();
  LOG_DEBUG("Waited all async tasks");

  ASSERT_EQ(7, execution_step);
}

TEST(TestTcpSocket, SocketEchoTestReadPartial) {
  LOG_AUTO_TRACE();

  auto schedulers_initiator = make_unique<SchedulersInitiator>();
  std::atomic_int execution_step{0};

  RunAsync(
      [&] {
        ++execution_step;
        Acceptor acceptor(SERVER_PORT);
        LOG_DEBUG("Accepting connection on port " << SERVER_PORT);
        ++execution_step;

        RunAsync([&]() {
          acceptor.DoAccept([&](std::shared_ptr<TcpSocket> accepted_socket) {
            ASSERT_TRUE(accepted_socket);

            LOG_DEBUG("Accepted socket");
            ++execution_step;
            const auto rcv_buffer = accepted_socket->ReadPartial();
            LOG_DEBUG("Server: received data: " << rcv_buffer.first);
            ++execution_step;
            BufferType snd_buffer{SERVER_ECHO_PREFIX};
            snd_buffer += rcv_buffer.first;
            LOG_DEBUG("Server: sending data: " << snd_buffer);
            accepted_socket->Write(snd_buffer);
            ++execution_step;
          });
        });

        auto socket = TcpSocket::Create();
        socket->Connect("127.0.0.1", SERVER_PORT);
        BufferType snd_buffer{GREETING};
        LOG_DEBUG("Client: sending data: " << snd_buffer);
        socket->Write(snd_buffer);
        ++execution_step;
        LOG_DEBUG("Client: reading reply from server");
        const auto rcv_buffer = socket->ReadPartial();
        LOG_DEBUG("Client: received data: " << rcv_buffer.first);
        ASSERT_EQ(SERVER_ECHO_PREFIX + snd_buffer, rcv_buffer.first);
        ++execution_step;
      },
      GetNetworkSchedulerAccessorInstance().GetRef());

  LOG_DEBUG("Waiting all async tasks");
  WaitAll();
  LOG_DEBUG("Waited all async tasks");

  ASSERT_EQ(7, execution_step);
}
