// Copyright [2018] <Malinovsky Rodion>

#include <gtest/gtest.h>
#include <algorithm>
#include <boost/context/detail/exception.hpp>
#include <boost/coroutine2/all.hpp>
#include <functional>
#include <ostream>

namespace {

class IntGenerator {
 public:
  explicit IntGenerator(int initial_value)
      : initial_value_(initial_value), coro_([this](CoroType::push_type& yield) {
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

namespace {

class Deferer {
 public:
  using HandlerType = std::function<void()>;
  explicit Deferer(HandlerType handler);

  void Start(HandlerType handler);

  void Resume();

 private:
  using CoroType = boost::coroutines2::coroutine<void>;

  CoroType::pull_type MakeCoro();

  HandlerType handler_;

  CoroType::pull_type coro_;
};

Deferer::Deferer(HandlerType handler) : handler_(std::move(handler)), coro_(MakeCoro()) {}

void Deferer::Start(HandlerType handler) {
  using std::swap;
  handler_ = std::move(handler);
  auto new_coro = MakeCoro();
  swap(coro_, new_coro);
}

void Deferer::Resume() {
  if (coro_) {
    if (coro_) {
      coro_();
    }
  }
}

Deferer::CoroType::pull_type Deferer::MakeCoro() {
  // CTor fires coro
  return CoroType::pull_type{[this](CoroType::push_type& yield) {
    yield();
    if (handler_) {
      handler_();
    }
  }};
}

}  // namespace

TEST(TestCoroutine, DefererSingleCicle) {
  int fired_times = 0;
  Deferer deferer([&fired_times]() { ++fired_times; });
  ASSERT_EQ(0, fired_times);
  deferer.Resume();
  ASSERT_EQ(1, fired_times);
}

TEST(TestCoroutine, DefererTwoCicles) {
  int fired_times = 0;
  Deferer deferer([&fired_times]() { ++fired_times; });
  ASSERT_EQ(0, fired_times);
  deferer.Resume();
  ASSERT_EQ(1, fired_times);
  deferer.Resume();
  ASSERT_EQ(1, fired_times);
}

TEST(TestCoroutine, DefererReAssign) {
  int first_fired_times = 0;
  int second_fired_times = 0;
  Deferer deferer([&first_fired_times]() { ++first_fired_times; });
  ASSERT_EQ(0, first_fired_times);
  deferer.Resume();
  ASSERT_EQ(1, first_fired_times);
  deferer.Start([&second_fired_times]() { ++second_fired_times; });
  ASSERT_EQ(1, first_fired_times);
  ASSERT_EQ(0, second_fired_times);
  deferer.Resume();
  ASSERT_EQ(1, first_fired_times);
  ASSERT_EQ(1, second_fired_times);
}
