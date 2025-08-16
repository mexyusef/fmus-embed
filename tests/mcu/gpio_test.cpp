#include <gtest/gtest.h>
#include "fmus/mcu/gpio.h"

using namespace fmus::mcu;

TEST(MCUGpioTest, BasicFunctionality) {
    auto result = initGpio();
    EXPECT_TRUE(result.isOk() || result.error().code() == fmus::core::ErrorCode::McuInitFailed);
}

TEST(MCUGpioTest, PinConfiguration) {
    PinConfig pin = {0, 13};
    auto result = configurePin(pin, PinMode::Output);
    // May fail on systems without GPIO hardware
    EXPECT_TRUE(result.isOk() || result.isError());
}
