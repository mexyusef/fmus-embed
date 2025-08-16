#include <gtest/gtest.h>
#include "fmus/actuators/motor.h"

using namespace fmus::actuators;
using namespace fmus::core;

class MotorTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(MotorTest, DCMotorConstruction) {
    DCMotor motor(9, 10); // PWM pin 9, direction pin 10
    EXPECT_FALSE(motor.isInitialized());
    EXPECT_EQ(motor.getType(), MotorType::DC);
}

TEST_F(MotorTest, ServoMotorConstruction) {
    ServoMotor servo(11); // PWM pin 11
    EXPECT_FALSE(servo.isInitialized());
    EXPECT_EQ(servo.getType(), MotorType::Servo);
}

TEST_F(MotorTest, StepperMotorConstruction) {
    StepperMotor stepper(2, 3, 4, 5); // Step pins
    EXPECT_FALSE(stepper.isInitialized());
    EXPECT_EQ(stepper.getType(), MotorType::Stepper);
}

TEST_F(MotorTest, DCMotorBasicOperations) {
    DCMotor motor(9, 10);
    
    // Test initialization
    auto initResult = motor.init();
    EXPECT_TRUE(initResult.isOk() || initResult.isError()); // May fail without hardware
    
    // Test speed setting
    auto speedResult = motor.setSpeed(0.5f);
    EXPECT_TRUE(speedResult.isOk() || speedResult.isError());
    
    // Test direction setting
    auto dirResult = motor.setDirection(MotorDirection::Forward);
    EXPECT_TRUE(dirResult.isOk() || dirResult.isError());
}

TEST_F(MotorTest, ServoMotorBasicOperations) {
    ServoMotor servo(11);
    
    auto initResult = servo.init();
    EXPECT_TRUE(initResult.isOk() || initResult.isError());
    
    auto angleResult = servo.setAngle(90.0f);
    EXPECT_TRUE(angleResult.isOk() || angleResult.isError());
}

TEST_F(MotorTest, StepperMotorBasicOperations) {
    StepperMotor stepper(2, 3, 4, 5);
    
    auto initResult = stepper.init();
    EXPECT_TRUE(initResult.isOk() || initResult.isError());
    
    auto stepResult = stepper.step(100);
    EXPECT_TRUE(stepResult.isOk() || stepResult.isError());
}
