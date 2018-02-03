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

class ThreadPool : public IScheduler, public IIoService {
 public:
  ThreadPool(const std::size_t thread_count, const char* name);

  ~ThreadPool();

  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  void Schedule(HandlerType handler) override;

  void Wait();

  const char* GetName() const override;

 private:
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
