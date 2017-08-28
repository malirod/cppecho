// Copyright [2017] <Malinovsky Rodion>

#include <memory>
#include <mutex>

#include "core/async.h"
#include "core/default_scheduler_accessor.h"
#include "core/helper.h"
#include "core/sequential_scheduler.h"
#include "core/thread_pool.h"
#include "net/tcp_server.h"
#include "net/tcp_socket.h"
#include "net/util.h"
#include "util/logger.h"
#include "util/smartptr_util.h"

#include "boost/algorithm/string.hpp"
#include "gtest/gtest.h"

DECLARE_GLOBAL_GET_LOGGER("Test.Net.TCPServer");

namespace {

using cppecho::core::ThreadPool;
using cppecho::core::RunAsync;
using cppecho::core::RunAsyncWait;
using cppecho::core::WaitAll;
using cppecho::net::BufferType;
using cppecho::net::TcpSocket;
using cppecho::net::TcpServer;
using cppecho::net::TcpServerIdType;
using cppecho::util::make_unique;
using cppecho::net::GetNetworkServiceAccessorInstance;
using cppecho::core::SequentialScheduler;
using cppecho::core::SchedulersInitiator;
using cppecho::core::DeferProceed;
using cppecho::core::HandlerType;

const int SERVER_PORT = 10125;

const char SERVER_ECHO_PREFIX[] = "echo: ";

const char GREETING[] = "Hello World!!!";

}  // namespace

TEST(TestTcpServer, EchoTest) {
  LOG_AUTO_TRACE();

  auto schedulers_initiator = make_unique<SchedulersInitiator>();

  static const std::string net_sequential_scheduler_name{"net_sequential"};
  std::atomic_int execution_step{0};

  SequentialScheduler net_sequential_scheduler(
      GetNetworkServiceAccessorInstance().GetRef(),
      net_sequential_scheduler_name.c_str());

  std::unique_ptr<TcpServer> tcp_server;

  std::mutex mutex;
  std::condition_variable waiter;
  std::atomic_bool server_stopped{false};

  RunAsync(
      [&] {
        tcp_server = make_unique<TcpServer>(2);

        tcp_server->SubscribeOnListening([&]() {
          ++execution_step;
          // After all data transfer will be done this socket should
          // start disconnection
          auto socket = TcpSocket::Create();

          socket->SubscribeOnDisconnected([&](TcpSocket&) {
            ASSERT_TRUE(false) << "Should not fire since start wasn't called";
          });

          socket->Connect("127.0.0.1", SERVER_PORT);

          for (int i = 1; i <= 3; ++i) {
            BufferType snd_buffer{GREETING};
            snd_buffer += " #" + std::to_string(i);
            LOG_DEBUG("Client: sending data: " << snd_buffer);
            socket->Write(snd_buffer + "\n");
            ++execution_step;
            LOG_DEBUG("Client: reading reply from server");
            auto rcv_buffer = socket->ReadUntil("\n");
            boost::algorithm::trim(rcv_buffer);
            LOG_DEBUG("Client: received data: " << rcv_buffer);
            ASSERT_EQ(SERVER_ECHO_PREFIX + snd_buffer, rcv_buffer);
            ++execution_step;
          }
        });

        tcp_server->SubscribeOnConnected([&](TcpServerIdType id) {
          LOG_DEBUG("Server: Client Connected");
          ASSERT_EQ(1u, id);
          ++execution_step;
        });

        tcp_server->SubscribeOnData(
            [&](TcpServerIdType id, const BufferType& data) {
              ASSERT_EQ(1u, id);
              ++execution_step;
              const auto rcv_buffer = boost::algorithm::trim_copy(data);
              LOG_DEBUG("Server: received data: " << rcv_buffer);
              ++execution_step;
              BufferType snd_buffer{SERVER_ECHO_PREFIX};
              snd_buffer += rcv_buffer;
              LOG_DEBUG("Server: sending data: " << snd_buffer);
              tcp_server->Write(id, snd_buffer + "\n");
              ++execution_step;
            });

        tcp_server->SubscribeOnDisconnected([&](TcpServerIdType id) {
          LOG_DEBUG("Client DisConnected");
          ASSERT_EQ(1u, id);
          ++execution_step;
          tcp_server->Stop();
        });

        tcp_server->SubscribeOnStopped([&]() {
          ASSERT_EQ(net_sequential_scheduler_name,
                    cppecho::core::GetCurrentThreadScheduler().GetName());
          LOG_DEBUG("Server has been closed.");
          ++execution_step;

          server_stopped = true;
          waiter.notify_one();
        });

        LOG_DEBUG("Starting server");
        tcp_server->Start(SERVER_PORT);
      },
      net_sequential_scheduler);

  LOG_DEBUG("Waiting server to be stopped");
  {
    std::unique_lock<std::mutex> lock(mutex);
    waiter.wait(lock, [&]() { return server_stopped.load(); });
  }

  LOG_DEBUG("Waited server to be stopped");

  LOG_DEBUG("Waiting all");
  WaitAll();
  LOG_DEBUG("Waited all");

  ASSERT_EQ(19, execution_step);
}

TEST(TestTcpServer, MaxConnections) {
  LOG_AUTO_TRACE();

  auto schedulers_initiator = make_unique<SchedulersInitiator>();

  const std::string net_sequential_scheduler_name{"net_sequential"};

  SequentialScheduler net_sequential_scheduler(
      GetNetworkServiceAccessorInstance().GetRef(),
      net_sequential_scheduler_name.c_str());
  std::atomic_int execution_step{0};

  std::unique_ptr<TcpServer> tcp_server;

  std::shared_ptr<TcpSocket> client1;
  std::shared_ptr<TcpSocket> client2;
  // this one should fail to connect
  std::shared_ptr<TcpSocket> client3;

  std::mutex mutex;
  std::condition_variable waiter;
  std::atomic_bool server_stopped{false};

  RunAsync(
      [&] {
        ASSERT_EQ(net_sequential_scheduler_name,
                  cppecho::core::GetCurrentThreadScheduler().GetName());

        tcp_server = make_unique<TcpServer>(2);
        client1 = TcpSocket::Create();
        client2 = TcpSocket::Create();
        // this one should fail to connect
        client3 = TcpSocket::Create();

        client1->SubscribeOnDisconnected([&](TcpSocket&) {
          ASSERT_EQ(net_sequential_scheduler_name,
                    cppecho::core::GetCurrentThreadScheduler().GetName());
          ++execution_step;
          LOG_DEBUG("Client #1 Disconnected");
        });

        client2->SubscribeOnDisconnected([&](TcpSocket&) {
          ASSERT_EQ(net_sequential_scheduler_name,
                    cppecho::core::GetCurrentThreadScheduler().GetName());
          ++execution_step;
          LOG_DEBUG("Client #2 Disconnected");
        });

        client3->SubscribeOnDisconnected([&](TcpSocket&) {
          ASSERT_EQ(net_sequential_scheduler_name,
                    cppecho::core::GetCurrentThreadScheduler().GetName());
          ++execution_step;
          LOG_DEBUG("Client #3 Disconnected");
          tcp_server->Stop();
        });

        tcp_server->SubscribeOnConnected([&](TcpServerIdType id) {
          ASSERT_GT(id, 0u);
          ASSERT_EQ(net_sequential_scheduler_name,
                    cppecho::core::GetCurrentThreadScheduler().GetName());
          LOG_DEBUG("Server: Connected: id=" << id);
          ++execution_step;
        });

        tcp_server->SubscribeOnDisconnected([&](TcpServerIdType id) {
          ASSERT_GT(id, 0u);
          ASSERT_EQ(net_sequential_scheduler_name,
                    cppecho::core::GetCurrentThreadScheduler().GetName());
          LOG_DEBUG("Server: DisConnected: id=" << id);
          ++execution_step;
        });

        tcp_server->SubscribeOnListening([&]() {
          ASSERT_EQ(net_sequential_scheduler_name,
                    cppecho::core::GetCurrentThreadScheduler().GetName());
          client1->Connect("127.0.0.1", SERVER_PORT);
          client1->Start();
          ++execution_step;

          client2->Connect("127.0.0.1", SERVER_PORT);
          client2->Start();
          ++execution_step;

          client3->Connect("127.0.0.1", SERVER_PORT);
          client3->Start();
          ++execution_step;
        });

        tcp_server->SubscribeOnStopped([&]() {
          ASSERT_EQ(net_sequential_scheduler_name,
                    cppecho::core::GetCurrentThreadScheduler().GetName());
          LOG_DEBUG("Server has been closed.");
          ++execution_step;

          server_stopped = true;
          waiter.notify_one();
        });

        ++execution_step;
        LOG_DEBUG("Starting server");
        tcp_server->Start(SERVER_PORT);
        LOG_DEBUG("Start has finished");
        ++execution_step;
      },
      net_sequential_scheduler);

  LOG_DEBUG("Waiting server to be stopped");
  {
    std::unique_lock<std::mutex> lock(mutex);
    waiter.wait(lock, [&]() { return server_stopped.load(); });
  }

  LOG_DEBUG("Waited server to be stopped");

  LOG_DEBUG("Waiting all");
  WaitAll();
  LOG_DEBUG("Waited all");

  ASSERT_EQ(13, execution_step);
}
