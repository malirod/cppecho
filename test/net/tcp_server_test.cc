// Copyright [2017] <Malinovsky Rodion>

#include <deque>
#include <memory>

#include "core/async.h"
#include "core/default_scheduler_accessor.h"
#include "core/thread_pool.h"
#include "net/socket.h"
#include "net/tcp_server.h"
#include "net/util.h"
#include "util/logger.h"
#include "util/smartptr_util.h"

#include "boost/algorithm/string.hpp"
#include "gtest/gtest.h"

DECLARE_GLOBAL_GET_LOGGER("Test.Net.TCPServer");

namespace {

using cppecho::core::ThreadPool;
using cppecho::core::RunAsync;
using cppecho::core::WaitAll;
using cppecho::net::BufferType;
using cppecho::net::Socket;
using cppecho::net::TcpServer;
using cppecho::net::TcpServerIdType;
using cppecho::util::make_unique;
using cppecho::net::GetNetworkServiceAccessorInstance;
using cppecho::net::GetNetworkSchedulerAccessorInstance;
using cppecho::core::GetDefaultIoServiceAccessorInstance;
using cppecho::core::GetDefaultSchedulerAccessorInstance;

const int SERVER_PORT = 10125;

const char SERVER_ECHO_PREFIX[] = "echo: ";

const char GREETING[] = "Hello World!!!";

class TestTcpServer : public ::testing::Test {
 public:
  TestTcpServer();

  ~TestTcpServer() override;

 protected:
  void LaunchClient();

  std::unique_ptr<ThreadPool> thread_pool_net_;

  std::unique_ptr<ThreadPool> thread_pool_main_;

  std::unique_ptr<TcpServer> tcp_server_;

  std::atomic_int execution_step_{0};
};

TestTcpServer::TestTcpServer() {
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
}

TestTcpServer::~TestTcpServer() {
  LOG_DEBUG("Teardown");

  GetNetworkServiceAccessorInstance().Detach();
  GetNetworkSchedulerAccessorInstance().Detach();

  GetDefaultIoServiceAccessorInstance().Detach();
  GetDefaultSchedulerAccessorInstance().Detach();

  thread_pool_net_.reset();
  thread_pool_main_.reset();
}

void TestTcpServer::LaunchClient() {
  LOG_AUTO_TRACE();
  RunAsync(
      [&] {
        Socket socket;
        socket.Connect("127.0.0.1", SERVER_PORT);

        for (int i = 1; i <= 3; ++i) {
          BufferType snd_buffer{GREETING};
          snd_buffer += " #" + std::to_string(i);
          LOG_DEBUG("Client: sending data: " << snd_buffer);
          socket.Write(snd_buffer + "\n");
          execution_step_++;
          LOG_DEBUG("Client: reading reply from server");
          auto rcv_buffer = socket.ReadUntil("\n");
          boost::algorithm::trim(rcv_buffer);
          LOG_DEBUG("Client: received data: " << rcv_buffer);
          ASSERT_EQ(SERVER_ECHO_PREFIX + snd_buffer, rcv_buffer);
          execution_step_++;
        }
      },
      *thread_pool_net_);
}

}  // namespace

TEST_F(TestTcpServer, TcpServerEchoTest) {
  LOG_AUTO_TRACE();

  RunAsync(
      [&] {
        tcp_server_ = make_unique<TcpServer>(2);

        tcp_server_->SubscribeOnListening([&]() {
          execution_step_++;
          LaunchClient();
        });

        tcp_server_->SubscribeOnConnected([&](TcpServerIdType id) {
          ASSERT_EQ(1u, id);
          execution_step_++;
        });

        tcp_server_->SubscribeOnData(
            [&](TcpServerIdType id, const BufferType& data) {
              ASSERT_EQ(1u, id);
              execution_step_++;
              const auto rcv_buffer = boost::algorithm::trim_copy(data);
              LOG_DEBUG("Server: received data: " << rcv_buffer);
              execution_step_++;
              BufferType snd_buffer{SERVER_ECHO_PREFIX};
              snd_buffer += rcv_buffer;
              LOG_DEBUG("Server: sending data: " << snd_buffer);
              tcp_server_->Write(id, snd_buffer + "\n");
              execution_step_++;
            });

        tcp_server_->SubscribeOnDisconnected([&](TcpServerIdType id) {
          ASSERT_EQ(1u, id);
          execution_step_++;
          tcp_server_->Stop();
        });

        tcp_server_->Start(SERVER_PORT);
      },
      *thread_pool_net_);

  WaitAll();

  ASSERT_EQ(18, execution_step_);
}
