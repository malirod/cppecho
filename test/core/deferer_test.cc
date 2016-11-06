// Copyright [2016] <Malinovsky Rodion>

#include "core/deferer.h"
#include "boost/coroutine2/all.hpp"
#include "gtest/gtest.h"

TEST(TestDeferer, DefererSingleCicle) {
  int fired_times = 0;
  cppecho::core::Deferer deferer([&fired_times]() { ++fired_times; });
  ASSERT_EQ(0, fired_times);
  deferer.Resume();
  ASSERT_EQ(1, fired_times);
}

TEST(TestDeferer, TwoCicles) {
  int fired_times = 0;
  cppecho::core::Deferer deferer([&fired_times]() { ++fired_times; });
  ASSERT_EQ(0, fired_times);
  deferer.Resume();
  ASSERT_EQ(1, fired_times);
  deferer.Resume();
  ASSERT_EQ(1, fired_times);
}

TEST(TestDeferer, ReAssign) {
  int first_fired_times = 0;
  int second_fired_times = 0;
  cppecho::core::Deferer deferer(
      [&first_fired_times]() { ++first_fired_times; });
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
