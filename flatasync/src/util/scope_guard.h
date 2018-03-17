// Copyright [2018] <Malinovsky Rodion>

#pragma once

#include <utility>

namespace rms {
namespace util {

/**
 * Class uses RAII idiom for atomatic cleanup.
 * @tparam CleanupAction Callable to be executed when going out of scope.
 */
template <typename CleanupAction>
class ScopeGuard {
 public:
  /**
   * Create guard object.
   * @param cleanup_action action to be performed when going out of scope.
   */
  explicit ScopeGuard(CleanupAction&& cleanup_action) : cleanup_action_(std::move(cleanup_action)) {}

  /**
   * Destroy object and call cleanup action..
   */
  ~ScopeGuard() {
    cleanup_action_();
  }

 private:
  CleanupAction cleanup_action_;
};

template <typename CleanupAction>
ScopeGuard<CleanupAction> MakeScopeGuard(CleanupAction cleanup_action) {
  return ScopeGuard<CleanupAction>(std::move(cleanup_action));
}

}  // namespace util
}  // namespace rms
