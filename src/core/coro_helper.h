// Copyright [2016] <Malinovsky Rodion>

#include <memory>
#include "boost/coroutine2/all.hpp"
#include "core/alias.h"
#include "util/logger.h"

namespace cppecho {
namespace core {

class CoroHelper {
 public:
  CoroHelper();

  ~CoroHelper();

  explicit CoroHelper(HandlerType handler);

  void Start(HandlerType handler);

  void Yield();

  void Resume();

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

void Yield();

bool IsInsideCoroutine();

}  // namespace core
}  // namespace cppecho
