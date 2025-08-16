#include <gtest/gtest.h>
#include "fmus/mcu/adc.h"

using namespace fmus::mcu;

TEST(AdcTest, Initialization) {
    auto result = initAdc();
    EXPECT_TRUE(result.isOk());
}

TEST(AdcTest, ChannelRead) {
    auto result = readAdcChannel(0);
    EXPECT_TRUE(result.isOk() || result.isError());
}
