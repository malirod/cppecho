// Copyright [2016] <Malinovsky Rodion>

#include "boost/coroutine2/all.hpp"
#include "core/alias.h"
#include "util/logger.h"

namespace cppecho {
namespace core {

class CoroHelper {
 public:
  CoroHelper();

  explicit CoroHelper(HandlerType handler);

  void Start(HandlerType handler);

  void Yield();

  void Resume();

 private:
  DECLARE_GET_LOGGER("Core.CoHelper")

  class Guard {
   public:
    explicit Guard(CoroHelper& ptr_co_helper);
    ~Guard();

   private:
    CoroHelper* ptr_co_helper_ = nullptr;
  };

  using CoroType = boost::coroutines2::coroutine<void>;

  CoroType::pull_type MakeCoro();
  // std::unique_ptr<CoroType::pull_type> MakeCoro();

  HandlerType handler_;

  CoroType::push_type* ptr_yield_;

  // std::unique_ptr<CoroType::pull_type> coro_;
  CoroType::pull_type coro_;
};

void Yield();

bool IsInsideCoroutine();

}  // namespace core
}  // namespace cppecho
