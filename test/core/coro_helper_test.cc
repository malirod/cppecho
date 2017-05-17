// Copyright [2016] <Malinovsky Rodion>

#include "core/coro_helper.h"
#include <stdexcept>
#include <thread>
#include "gtest/gtest.h"

using cppecho::core::CoroHelper;
using cppecho::core::Yield;

DECLARE_GLOBAL_GET_LOGGER("Core.CoroHelperTest")

TEST(TestCoroHelper, DefaultConstructible) {
  CoroHelper coro_helper;
  ASSERT_FALSE(coro_helper);
  coro_helper.Yield();
  coro_helper.Resume();
}

TEST(TestCoroHelper, PassHandleViaCtor) {
  int value = 0;

  CoroHelper coro_helper{[&value]() {
    while (true) {
      ++value;
      Yield();
    }
  }};
  ASSERT_EQ(1, value);
  coro_helper.Resume();
  ASSERT_EQ(2, value);
  coro_helper.Resume();
  ASSERT_EQ(3, value);
  coro_helper.Resume();
  ASSERT_EQ(4, value);
}

TEST(TestCoroHelper, PassFinitHandleViaStart) {
  int value = 0;

  CoroHelper coro_helper;
  coro_helper.Start([&value]() {
    value += 5;
    Yield();
    value += 3;
  });

  ASSERT_EQ(5, value);
  coro_helper.Resume();
  ASSERT_EQ(8, value);
}

TEST(TestCoroHelper, ResumeMoreThanYield) {
  int value = 0;

  CoroHelper coro_helper;
  coro_helper.Start([&value]() {
    value += 5;
    Yield();
    value += 3;
  });

  ASSERT_EQ(5, value);
  coro_helper.Resume();
  ASSERT_EQ(8, value);
  ASSERT_FALSE(coro_helper);
  coro_helper.Resume();
  coro_helper.Resume();
  coro_helper.Resume();
}

TEST(TestCoroHelper, PassHandleViaStart) {
  int value = 0;

  CoroHelper coro_helper;
  coro_helper.Start([&value]() {
    while (true) {
      value += 5;
      Yield();
    }
  });

  ASSERT_EQ(5, value);
  coro_helper.Resume();
  ASSERT_EQ(10, value);
}

TEST(TestCoroHelper, DestroyYieldedCoro) {
  int value = 0;

  {
    CoroHelper coro_helper;
    coro_helper.Start([&value]() {
      while (true) {
        value += 5;
        Yield();
      }
    });
  }
  ASSERT_EQ(5, value);
}

TEST(TestCoroHelper, PassHandleViaStartTwice) {
  int value = 0;

  CoroHelper coro_helper;
  coro_helper.Start([&value]() {
    while (true) {
      value += 5;
      Yield();
    }
  });

  ASSERT_EQ(5, value);
  coro_helper.Resume();
  ASSERT_EQ(10, value);
  coro_helper.Start([&value]() {
    while (true) {
      value += 10;
      Yield();
    }
  });
  ASSERT_EQ(20, value);
  coro_helper.Resume();
  ASSERT_EQ(30, value);
}

TEST(TestCoroHelper, PassHandleViaNonDefaultCtorAndStart) {
  int value = 0;

  CoroHelper coro_helper{[&value]() {
    while (true) {
      ++value;
      Yield();
    }
  }};
  ASSERT_EQ(1, value);
  coro_helper.Start([&value]() {
    while (true) {
      value += 5;
      Yield();
    }
  });
  ASSERT_EQ(6, value);
  coro_helper.Resume();
  ASSERT_EQ(11, value);
  coro_helper.Start([&value]() {
    while (true) {
      value += 10;
      Yield();
    }
  });
  ASSERT_EQ(21, value);
  coro_helper.Resume();
  ASSERT_EQ(31, value);
}

TEST(TestCoroHelper, HandleException) {
  int value = 0;
  bool is_expception_caught = false;

  try {
    CoroHelper coro_helper{[&value]() {
      while (true) {
        ++value;
        if (value == 2) {
          throw std::runtime_error("Test exception in handler");
        }
        Yield();
      }
    }};

    ASSERT_EQ(1, value);

    coro_helper.Resume();

    ASSERT_EQ(11, value);
  } catch (const std::runtime_error&) {
    is_expception_caught = true;
  }
  ASSERT_TRUE(is_expception_caught);
}

TEST(TestCoroHelper, SwitchThread) {
  std::atomic_int value{0};

  CoroHelper coro_helper;
  coro_helper.Start([&value]() {
    while (true) {
      ++value;
      Yield();
    }
  });

  ASSERT_EQ(1, value);
  coro_helper.Resume();
  ASSERT_EQ(2, value);

  auto new_trhread = std::thread([&] {
    coro_helper.Resume();
    ASSERT_EQ(3, value);
    coro_helper.Resume();
    ASSERT_EQ(4, value);
  });
  new_trhread.join();

  coro_helper.Resume();
  ASSERT_EQ(5, value);
}
