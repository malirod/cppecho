// Copyright [2016] <Malinovsky Rodion>

#include "core/thread_pool.h"
#include <utility>
#include "util/logger.h"
#include "util/smartptr_util.h"
#include "util/thread_util.h"

DECLARE_GLOBAL_GET_LOGGER("Core.ThreadPool")

cppecho::core::ThreadPool::ThreadPool(const std::size_t thread_count,
                                      const char* name)
    : name_(name)
    , asio_service_()
    , work_(util::make_unique<AsioServiceWorkType>(asio_service_)) {
  LOG_AUTO_TRACE();
  threads_.reserve(thread_count);
  for (std::size_t i = 0u; i < thread_count; ++i) {
    threads_.emplace_back(util::ThreadUtil::CreateThread(
        [this] {
          while (true) {
            asio_service_.run();
            std::unique_lock<std::mutex> lock(mutex_);
            if (stopped_) {
              break;
            }
            if (!work_) {
              work_ = util::make_unique<AsioServiceWorkType>(asio_service_);
              asio_service_.reset();
              awaiter_.notify_all();
            }
          }
        },
        name_));
  }
  LOG_DEBUG(name_ << ": Thread pool created with threads: " << thread_count);
}

cppecho::core::ThreadPool::~ThreadPool() {
  LOG_AUTO_TRACE();
  {
    std::lock_guard<std::mutex> lock(mutex_);
    stopped_ = true;
    work_.reset();
  }
  LOG_DEBUG(Name() << ": Stopping thread pool");
  for (auto&& item : threads_) {
    item.join();
  }
  LOG_DEBUG(Name() << ": Thread pool stopped");
}

void cppecho::core::ThreadPool::Schedule(HandlerType handler) {
  LOG_AUTO_TRACE();
  asio_service_.post(std::move(handler));
}

void cppecho::core::ThreadPool::Wait() {
  LOG_AUTO_TRACE();
  std::unique_lock<std::mutex> lock(mutex_);
  work_.reset();
  while (true) {
    awaiter_.wait(lock);
    LOG_DEBUG(Name() << ": Wait completed: " << (work_ != nullptr));
    if (work_) {
      break;
    }
  }
}

const char* cppecho::core::ThreadPool::Name() const {
  return name_;
}

cppecho::core::AsioServiceType& cppecho::core::ThreadPool::GetAsioService() {
  return asio_service_;
}
