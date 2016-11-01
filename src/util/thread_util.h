// Copyright [2016] <Malinovsky Rodion>

#pragma once

#include <functional>
#include <string>
#include <thread>
#include "util/logger.h"
#include "util/singleton.h"
#include "util/type_traits.h"

namespace cppecho {
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

  template <typename Action>
  static std::thread CreateThread(Action action, const char* name);
};

void SleepFor(int ms);

template <typename Action>
std::thread ThreadUtil::CreateThread(Action action, const char* name) {
  LOG_AUTO_TRACE();
  return std::thread([action, name] {
    SetCurrentThreadName(name);
    SetCurrentThreadNumber(GenNewThreadNumber());
    LOG_TRACE("Created thread " << GetCurrentThreadId());
    try {
      action();
    } catch (std::exception& e) {
      (void)e;
      LOG_ERROR(GetCurrentThreadId() << ": Thread ended with error: "
                                     << e.what());
    }
  });
}

}  // namespace util
}  // namespace cppecho
