// Copyright [2017] <Malinovsky Rodion>

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

class TestSocket : public ::testing::Test {
 public:
  TestSocket();

  ~TestSocket() override;

 protected:
  constexpr static const int SERVER_PORT = 10123;

  constexpr static const char* const SERVER_ECHO_PREFIX = "echo: ";

  void LaunchServer();

  void LaunchClient();

  std::unique_ptr<ThreadPool> thread_pool_net_;

  std::unique_ptr<ThreadPool> thread_pool_main_;

  boost::barrier barrier{2};
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

        acceptor.DoAccept([&](Socket& socket) {
          LOG_DEBUG("Accepted socket");

          BufferType rcv_buffer(5, 0);
          socket.Read(rcv_buffer);
          BufferType snd_buffer{SERVER_ECHO_PREFIX};
          snd_buffer += rcv_buffer;
          socket.Write(snd_buffer);
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
        BufferType snd_buffer{"hello"};
        socket.Write(snd_buffer);
        BufferType rcv_buffer(sizeof(SERVER_ECHO_PREFIX) + snd_buffer.length(),
                              0);
        socket.Read(rcv_buffer);
        ASSERT_EQ(SERVER_ECHO_PREFIX + snd_buffer, rcv_buffer);
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
}
