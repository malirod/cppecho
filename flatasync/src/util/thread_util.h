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

/**
 * Group functions \ helpers to work with threads.
 */
class ThreadUtil {
 public:
  DECLARE_GET_LOGGER("Util.ThreadUtil")

  /**
   * Set current thread name. Don't use system calls. To be used with thread pool.
   * @param name new name of the thread.
   */
  static void SetCurrentThreadName(const char* name);

  /**
   * Set current thread number. Don't use system calls. To be used with thread pool.
   * @param number set numer of the current thread.
   */
  static void SetCurrentThreadNumber(int number);

  /**
   * Get current thread name. Don't use system calls. To be used with thread pool.
   * @return Name of the current thread.
   */
  static const char* GetCurrentThreadName();

  /**
   * Get current thread number. Don't use system calls. To be used with thread pool.
   * @return Number of the current thread.
   */
  static int GetCurrentThreadNumber();

  /**
   * Get current thread id. This is combination of thread name and thread number.
   * @return Thread id as string.
   */
  static std::string GetCurrentThreadId();

  /**
   * Generate sequental thread number.
   * @return New thread number.
   */
  static int GenNewThreadNumber();

  /**
   * Set asio io service for the current thread.
   * @param ioservice Asio io service to be assosiated with current thread.
   */
  static void SetCurrentThreadIoSerivce(rms::core::IIoService& ioservice);

  /**
   * Get asio io service assosiatedw with curent thread.
   * @return Asio io service assosiated with current thread.
   */
  static rms::core::IIoService& GetCurrentThreadIoSerivce();

  /**
   * Factory for threads.
   * @tparam Action Task type to be executed on new thread.
   * @param action Task type to be executed on new thread.
   * @param name Thread name. Number is calcualted automatically.
   * @return created thread.
   */
  template <typename Action>
  static std::thread CreateThread(Action action, const char* name);
};

/**
 * Sleep current thread for specified amount of msecs.
 * @param ms Msecs to spleep for.
 */
void SleepFor(int ms);

/**
 * Atomic with initialization. Default init is 0.
 * @tparam T Underlaying type of the atomic.
 */
template <typename T>
struct AtomicWithInit : std::atomic<T> {
  explicit AtomicWithInit(int initial_value = 0) : std::atomic<T>(initial_value) {}
};

/**
 * Tag class used for automatic thread numbers generation.
 */
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
