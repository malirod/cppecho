// Copyright [2017] <Malinovsky Rodion>

#include <deque>
#include <memory>

#include "core/async.h"
#include "core/default_scheduler_accessor.h"
#include "core/thread_pool.h"
#include "net/acceptor.h"
#include "net/util.h"
#include "util/logger.h"
#include "util/smartptr_util.h"

#include "gtest/gtest.h"

DECLARE_GLOBAL_GET_LOGGER("Test.Net.Socket");

namespace {

using cppecho::core::ThreadPool;
using cppecho::core::RunAsync;
using cppecho::core::WaitAll;
using cppecho::net::Acceptor;
using cppecho::net::BufferType;
using cppecho::net::Socket;
using cppecho::util::make_unique;
using cppecho::net::GetNetworkServiceAccessorInstance;
using cppecho::net::GetNetworkSchedulerAccessorInstance;
using cppecho::core::GetDefaultIoServiceAccessorInstance;
using cppecho::core::GetDefaultSchedulerAccessorInstance;

const int SERVER_PORT = 10123;

const char SERVER_ECHO_PREFIX[] = "echo: ";

const char GREETING[] = "Hello World!!!";

class TestSocket : public ::testing::Test {
 public:
  TestSocket();

  ~TestSocket() override;

 protected:
  void LaunchServer();

  void LaunchClient();

  std::unique_ptr<ThreadPool> thread_pool_net_;

  std::unique_ptr<ThreadPool> thread_pool_main_;

  boost::barrier barrier{2};

  std::atomic_int execution_step_{0};

  std::deque<std::unique_ptr<Socket>> client_connections_;
};

TestSocket::TestSocket() {
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

TestSocket::~TestSocket() {
  LOG_DEBUG("Teardown");

  GetNetworkServiceAccessorInstance().Detach();
  GetNetworkSchedulerAccessorInstance().Detach();

  GetDefaultIoServiceAccessorInstance().Detach();
  GetDefaultSchedulerAccessorInstance().Detach();

  thread_pool_net_.reset();
  thread_pool_main_.reset();
}

void TestSocket::LaunchServer() {
  LOG_AUTO_TRACE();
  RunAsync(
      [&] {
        Acceptor acceptor(SERVER_PORT);
        barrier.wait();
        LOG_DEBUG("Accepting connection on port " << SERVER_PORT);

        acceptor.DoAccept([&](std::unique_ptr<Socket> accepted_socket) {
          ASSERT_TRUE(accepted_socket);
          client_connections_.emplace_back(std::move(accepted_socket));
          auto& socket = *(client_connections_.back());

          // Ensure that new connection has disbled Nagle's algorythm
          const auto socket_opts = socket.GetSocketOpts();
          ASSERT_TRUE(socket_opts.at(Socket::SocketOpt::NoDelay));

          LOG_DEBUG("Accepted socket");
          execution_step_++;
          const auto rcv_buffer = socket.ReadExact(sizeof(GREETING) - 1);
          LOG_DEBUG("Server: received data: " << rcv_buffer);
          execution_step_++;
          BufferType snd_buffer{SERVER_ECHO_PREFIX};
          snd_buffer += rcv_buffer;
          LOG_DEBUG("Server: sending data: " << snd_buffer);
          socket.Write(snd_buffer);
          execution_step_++;
        });
      },
      *thread_pool_net_);
}

void TestSocket::LaunchClient() {
  LOG_AUTO_TRACE();
  RunAsync(
      [&] {
        Socket socket;
        socket.Connect("127.0.0.1", SERVER_PORT);
        BufferType snd_buffer{GREETING};
        LOG_DEBUG("Client: sending data: " << snd_buffer);
        socket.Write(snd_buffer);
        execution_step_++;
        LOG_DEBUG("Client: reading reply from server");
        const auto rcv_buffer = socket.ReadExact(sizeof(SERVER_ECHO_PREFIX) -
                                                 1 + snd_buffer.length());
        LOG_DEBUG("Client: received data: " << rcv_buffer);
        ASSERT_EQ(SERVER_ECHO_PREFIX + snd_buffer, rcv_buffer);
        execution_step_++;
      },
      *thread_pool_net_);
}

}  // namespace

TEST_F(TestSocket, SocketEchoTest) {
  LOG_AUTO_TRACE();

  LaunchServer();

  barrier.wait();

  LaunchClient();

  WaitAll();

  ASSERT_EQ(5, execution_step_);
}
