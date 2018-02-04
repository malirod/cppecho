// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include <atomic>
#include <exception>
#include <string>  // IWYU pragma: keep
#include <thread>
#include "util/logger.h"
#include "util/singleton.h"

namespace rms {
namespace core {

class IIoService;

}  // namespace core
}  // namespace rms

namespace rms {
namespace util {

class ThreadUtil {
 public:
  DECLARE_GET_LOGGER("Util.ThreadUtil")

  static void SetCurrentThreadName(const char* name);

  static void SetCurrentThreadNumber(int number);

  static const char* GetCurrentThreadName();

  static int GetCurrentThreadNumber();

  static std::string GetCurrentThreadId();

  static int GenNewThreadNumber();

  static void SetCurrentThreadIoSerivce(rms::core::IIoService& ioservice);

  static rms::core::IIoService& GetCurrentThreadIoSerivce();

  template <typename Action>
  static std::thread CreateThread(Action action, const char* name);
};

void SleepFor(int ms);

template <typename T>
struct AtomicWithInit : std::atomic<T> {
  explicit AtomicWithInit(int initial_value = 0) : std::atomic<T>(initial_value) {}
};

class DefaultThreadCounterTag;

template <typename Tag>
std::atomic<int>& GetAtomicInstance() {
  return single<AtomicWithInit<int>, Tag>();
}

template <typename Action>
std::thread ThreadUtil::CreateThread(Action action, const char* name) {
  LOG_AUTO_TRACE();
  return std::thread([action, name] {
    SetCurrentThreadName(name);
    SetCurrentThreadNumber(++GetAtomicInstance<DefaultThreadCounterTag>());
    const auto& id = GetCurrentThreadId();
    (void)id;
    LOG_TRACE("Created thread " << id);
    LOG_AUTO_NDC(id);
    try {
      action();
    } catch (std::exception& e) {
      (void)e;
      LOG_ERROR(id << ": Thread ended with error: " << e.what());
    }
  });
}

}  // namespace util
}  // namespace rms
