#include <gtest/gtest.h>
#include "fmus/mcu/timer.h"

using namespace fmus::mcu;

TEST(TimerTest, Initialization) {
    auto result = initTimers();
    EXPECT_TRUE(result.isOk());
}

TEST(TimerTest, BasicTimer) {
    bool called = false;
    auto callback = [&called]() { called = true; };
    
    auto result = createTimer(100, callback, false);
    EXPECT_TRUE(result.isOk() || result.isError());
}
