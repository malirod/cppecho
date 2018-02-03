// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include "util/logger.h"
#include "util/singleton.h"

namespace rms {
namespace core {
class IScheduler;
}
}  // namespace rms

namespace rms {
namespace core {

class AsyncProxyBase {
 public:
  explicit AsyncProxyBase(IScheduler& destination);

  ~AsyncProxyBase();

 private:
  DECLARE_GET_LOGGER("Core.AsyncProxyBase")

  IScheduler& source;
};

template <typename T>
class WithAsyncProxy : public util::SingleAccessor<IScheduler> {
 public:
  struct Access : AsyncProxyBase {
    explicit Access(util::SingleAccessor<IScheduler>& single) : AsyncProxyBase(single) {}
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
}  // namespace rms
