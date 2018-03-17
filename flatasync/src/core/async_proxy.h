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

/**
 * RAII helper which switch to destination scheduler in ctor and switch back to current scheduler in dtor.
 */
class AsyncProxyBase {
 public:
  /**
   * Switch to another scheduler and holds reference to current scheduler.
   * @param destination New scheduler to switch to.
   */
  explicit AsyncProxyBase(IScheduler& destination);

  /**
   * Switch back to original scheduler
   */
  ~AsyncProxyBase();

 private:
  DECLARE_GET_LOGGER("Core.AsyncProxyBase")

  IScheduler& source;
};

/**
 * Allows to run methods of a class in specific scheduler. Destination scheduler should be specified via Attach.
 * @tparam T Class to wrap with async calls.
 */
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

/**
 * Factory to create async proxy helper. All
 * @tparam T Class to wrap with async calls. Allows to run methods of a class in specific scheduler.
 * @return Async proxy wrapper.
 */
template <typename T>
WithAsyncProxy<T>& AsyncProxy() {
  return util::single<WithAsyncProxy<T>>();
}

}  // namespace core
}  // namespace rms
