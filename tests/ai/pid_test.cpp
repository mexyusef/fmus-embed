#include <gtest/gtest.h>
#include "fmus/ai/pid.h"

using namespace fmus::ai;

TEST(PIDTest, BasicPID) {
    PIDController pid(1.0f, 0.1f, 0.01f);
    
    float output = pid.update(100.0f, 90.0f, 0.1f);
    EXPECT_NE(output, 0.0f);
}

TEST(PIDTest, GainSetting) {
    PIDController pid;
    pid.setGains(2.0f, 0.5f, 0.1f);
    
    float output = pid.update(50.0f, 45.0f, 0.1f);
    EXPECT_NE(output, 0.0f);
}
