// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include <boost/thread/barrier.hpp>
#include <condition_variable>
#include <cstddef>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include "core/alias.h"
#include "core/iioservice.h"
#include "core/ischeduler.h"

namespace rms {
namespace core {

/**
 * Allows to create and manipulate thread pools.
 */
class ThreadPool : public IScheduler, public IIoService {
 public:
  /**
   * Creates thread pool with given name and count of threads.
   * @param thread_count Count of threads in the pool.
   * @param name Thread pull name.
   */
  ThreadPool(const std::size_t thread_count, const char* name);

  /**
   * Destroy thread pool, stopp all threads and wait until they are done.
   */
  ~ThreadPool();

  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  /**
   * Schedule the task to be executed on random thread in thread pool.
   * @param handler Task to be executed in thread pool.
   */
  void Schedule(HandlerType handler) override;

  /**
   * Wait until thread pool is done.
   */
  void Wait();

  /**
   * Get thread pool name.
   * @return Name of the thread pool.
   */
  const char* GetName() const override;

 private:
  /**
   * Get underlying asio io service.
   * @return Asio io service which executes all tasks.
   */
  AsioServiceType& GetAsioService() override;

  const char* name_;

  AsioServiceType asio_service_;

  std::unique_ptr<AsioServiceWorkType> work_;

  std::vector<std::thread> threads_;

  std::mutex mutex_;

  std::condition_variable awaiter_;

  bool stopped_ = false;

  boost::barrier barrier_;
};

}  // namespace core
}  // namespace rms
