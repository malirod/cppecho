// Copyright [2016] <Malinovsky Rodion>

#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include "core/alias.h"
#include "core/iioservice.h"
#include "core/ischeduler.h"

namespace cppecho {
namespace core {

class ThreadPool : public IScheduler, public IIoService {
 public:
  ThreadPool(const std::size_t thread_count, const char* name);

  ~ThreadPool();

  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  void Schedule(HandlerType handler) override;

  void Wait();

  const char* Name() const override;

 private:
  AsioServiceType& GetAsioService() override;

  const char* name_;

  AsioServiceType asio_service_;

  std::unique_ptr<AsioServiceWorkType> work_;

  std::vector<std::thread> threads_;

  std::mutex mutex_;

  std::condition_variable awaiter_;

  bool stopped_ = false;
};

}  // namespace core
}  // namespace cppecho
