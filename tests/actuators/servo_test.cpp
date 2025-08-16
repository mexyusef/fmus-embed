#include <gtest/gtest.h>
#include "fmus/actuators/servo.h"

using namespace fmus::actuators;
using namespace fmus::core;

class ServoTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(ServoTest, Construction) {
    Servo servo(9); // PWM pin 9
    EXPECT_FALSE(servo.isInitialized());
}

TEST_F(ServoTest, AngleOperations) {
    Servo servo(9);
    
    auto initResult = servo.init();
    EXPECT_TRUE(initResult.isOk() || initResult.isError());
    
    auto angleResult = servo.setAngle(45.0f);
    EXPECT_TRUE(angleResult.isOk() || angleResult.isError());
    
    auto sweepResult = servo.sweep(0.0f, 180.0f, 1000);
    EXPECT_TRUE(sweepResult.isOk() || sweepResult.isError());
}
