// Copyright [2018] <Malinovsky Rodion>

#include <boost/coroutine2/all.hpp>
#include <memory>
#include "core/alias.h"
#include "util/logger.h"

namespace rms {
namespace core {

/**
 * Class encapsulates coroutines. Based on Boost::Coroutine2.
 */
class CoroHelper {
 public:
  /**
   * Create default\empty coroutine.
   */
  CoroHelper();

  /**
   * Delete coroutine.
   */
  ~CoroHelper();

  /**
   * Create coroutine with actual body. Execution is started automatically.
   * @param handler Body of the coroutine.
   */
  explicit CoroHelper(HandlerType handler);

  /**
   * Assign new body to coroutine and start execution.
   * @param handler New body of the coroutine.
   */
  void Start(HandlerType handler);

  /**
   * Yields (suspend execution) this coroutine.
   */
  void Yield();

  /**
   * Resume execution of this coroutine.
   */
  void Resume();

  /**
   * Checks whether coroutine is usable.
   * @return
   */
  explicit operator bool() const;

 private:
  DECLARE_GET_LOGGER("Core.CoroHelper")

  using CoroType = boost::coroutines2::coroutine<void>;
  using CoroPullType = std::unique_ptr<CoroType::pull_type>;

  CoroPullType MakeCoroAndAutoStart();

  HandlerType handler_;

  CoroType::push_type* ptr_yield_;

  CoroPullType coro_;
};

/**
 * Yields (suspend execution) current coroutine. Asserts that called within coroutine.
 */
void Yield();

/**
 * Check whether executed within the context of coroutine.
 * @return True if executed within corouting. False otherwise.
 */
bool IsInsideCoroutine();

}  // namespace core
}  // namespace rms
