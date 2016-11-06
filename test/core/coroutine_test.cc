// Copyright [2016] <Malinovsky Rodion>

#include "boost/coroutine2/all.hpp"
#include "gtest/gtest.h"

namespace {

class IntGenerator {
 public:
  explicit IntGenerator(int initial_value)
      : initial_value_(initial_value)
      , coro_([this](CoroType::push_type& yield) {
        int result = initial_value_;
        while (true) {
          yield(result++);
        }
      }) {}

  int GenInt() {
    if (!coro_) {
      return -1;
    }
    auto result = coro_.get();
    coro_();
    return result;
  }

 private:
  using CoroType = boost::coroutines2::coroutine<int>;

  int initial_value_ = 0;

  CoroType::pull_type coro_;
};

}  // namespace

TEST(TestCoroutine, IntGeneratorDirect) {
  using CoroutineType = boost::coroutines2::coroutine<int>;
  CoroutineType::pull_type GenIntCoro([](CoroutineType::push_type& yield) {
    int first = 1;
    int second = 1;
    yield(first);
    yield(second);
    for (int i = 0; i < 8; ++i) {
      int third = first + second;
      first = second;
      second = third;
      yield(third);
    }
  });

  std::stringstream sstream;
  for (auto i : GenIntCoro) {
    sstream << i << ",";
  }
  ASSERT_EQ("1,1,2,3,5,8,13,21,34,55,", sstream.str());
}

TEST(TestCoroutine, IntGeneratorInDirect) {
  std::stringstream sstream;
  IntGenerator int_gen{10};
  for (int i = 0; i < 10; ++i) {
    sstream << int_gen.GenInt() << ",";
  }
  ASSERT_EQ("10,11,12,13,14,15,16,17,18,19,", sstream.str());
}
