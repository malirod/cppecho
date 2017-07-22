// Copyright [2017] <Malinovsky Rodion>

#pragma once

#include "core/async.h"
#include "util/singleton.h"

namespace cppecho {
namespace core {

class AsyncProxy {
 public:
  explicit AsyncProxy(IScheduler& destination);

  ~AsyncProxy();

 private:
  DECLARE_GET_LOGGER("Core.AsyncProxy")

  IScheduler& source;
};

template <typename T>
class WithAsyncProxy : public util::SingleAccessor<IScheduler> {
 public:
  struct Access : AsyncProxy {
    explicit Access(util::SingleAccessor<IScheduler>& single)
        : AsyncProxy(single) {}
    T* operator->() {
      return &util::single<T>();
    }
  };

  Access operator->() {
    return Access(*this);
  }
};

template <typename T>
WithAsyncProxy<T>& AsyncProxy() {
  return util::single<WithAsyncProxy<T>>();
}

}  // namespace core
}  // namespace cppecho
