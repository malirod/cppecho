// Copyright [2018] <Malinovsky Rodion>

#include "core/thread_pool.h"
#include <boost/asio.hpp>
#include <utility>
#include "util/logger.h"
#include "util/thread_util.h"

DECLARE_GLOBAL_GET_LOGGER("Core.ThreadPool")

rms::core::ThreadPool::ThreadPool(const std::size_t thread_count, const char* name)
    : name_(name)
    , asio_service_()
    , work_(std::make_unique<AsioServiceWorkType>(asio_service_))
    , barrier_(thread_count + 1u) {
  LOG_AUTO_TRACE();
  threads_.reserve(thread_count);
  for (std::size_t i = 0u; i < thread_count; ++i) {
    threads_.emplace_back(util::ThreadUtil::CreateThread(
        [this] {
          util::ThreadUtil::SetCurrentThreadIoService(*this);
          barrier_.wait();
          while (true) {
            asio_service_.run();
            std::unique_lock<std::mutex> lock(mutex_);
            if (stopped_) {
              break;
            }
            if (!work_) {
              work_ = std::make_unique<AsioServiceWorkType>(asio_service_);
              asio_service_.reset();
              awaiter_.notify_all();
            }
          }
        },
        name_));
  }
  barrier_.wait();
  LOG_DEBUG(name_ << ": Thread pool created with threads: " << thread_count);
}

rms::core::ThreadPool::~ThreadPool() {
  LOG_AUTO_TRACE();
  {
    std::lock_guard<std::mutex> lock(mutex_);
    stopped_ = true;
    work_.reset();
  }
  asio_service_.stop();
  LOG_DEBUG(GetName() << ": Stopping thread pool");
  for (auto&& item : threads_) {
    item.join();
  }
  LOG_DEBUG(GetName() << ": Thread pool stopped");
}

void rms::core::ThreadPool::Schedule(HandlerType handler) {
  LOG_AUTO_TRACE();
  asio_service_.post(std::move(handler));
}

void rms::core::ThreadPool::Wait() {
  LOG_AUTO_TRACE();
  std::unique_lock<std::mutex> lock(mutex_);
  work_.reset();
  while (true) {
    awaiter_.wait(lock);
    LOG_DEBUG(GetName() << ": Wait completed: " << (work_ != nullptr));
    if (work_) {
      break;
    }
  }
}

const char* rms::core::ThreadPool::GetName() const {
  return name_;
}

rms::core::AsioServiceType& rms::core::ThreadPool::GetAsioService() {
  return asio_service_;
}
