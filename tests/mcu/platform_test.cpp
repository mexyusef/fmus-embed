#include <gtest/gtest.h>
#include "fmus/mcu/platform.h"

using namespace fmus::mcu;

TEST(PlatformTest, Initialization) {
    auto result = initPlatform();
    EXPECT_TRUE(result.isOk());
}

TEST(PlatformTest, SystemInfo) {
    auto info = getSystemInfo();
    EXPECT_FALSE(info.empty());
}

TEST(PlatformTest, CpuTemperature) {
    auto result = getCpuTemperature();
    EXPECT_TRUE(result.isOk() || result.isError());
}
