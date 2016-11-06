// Copyright [2016] <Malinovsky Rodion>

#include "boost/coroutine2/all.hpp"
#include "core/alias.h"

namespace cppecho {
namespace core {

class Deferer {
 public:
  explicit Deferer(HandlerType handler);

  void Start(HandlerType handler);

  void Resume();

 private:
  using CoroType = boost::coroutines2::coroutine<void>;

  CoroType::pull_type MakeCoro();

  HandlerType handler_;

  CoroType::pull_type coro_;
};

}  // namespace core
}  // namespace cppecho
