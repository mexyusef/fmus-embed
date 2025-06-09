#include <fmus/gpio/gpio.h>
#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <chrono>

using namespace fmus;
using namespace fmus::gpio;

class GPIOTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a GPIO pin for testing
        gpio = std::make_unique<GPIO>(13);  // Test with pin 13

        // Initialize the GPIO pin
        auto result = gpio->init(GPIODirection::Output);
        ASSERT_TRUE(result.isOk()) << "Failed to initialize GPIO: " << result.error().toString();
    }

    void TearDown() override {
        // Clean up
        if (gpio && gpio->isInitialized()) {
            // Make sure the pin is off
            gpio->write(false);
        }
        gpio.reset();
    }

    std::unique_ptr<GPIO> gpio;
};

// Test initialization
TEST_F(GPIOTest, Initialization) {
    EXPECT_TRUE(gpio->isInitialized());
    EXPECT_EQ(gpio->getDirection(), GPIODirection::Output);
    EXPECT_EQ(gpio->getPin(), 13);

    // Test reinitialization with different mode
    auto result = gpio->init(GPIODirection::Input);
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(gpio->getDirection(), GPIODirection::Input);
}

// Test direction setting
TEST_F(GPIOTest, SetDirection) {
    // Test changing direction
    auto result = gpio->setDirection(GPIODirection::Input);
    ASSERT_TRUE(result.isOk()) << "Failed to set direction: " << result.error().toString();
    EXPECT_EQ(gpio->getDirection(), GPIODirection::Input);

    // Change back
    result = gpio->setDirection(GPIODirection::Output);
    ASSERT_TRUE(result.isOk()) << "Failed to set direction: " << result.error().toString();
    EXPECT_EQ(gpio->getDirection(), GPIODirection::Output);
}

// Test write and read operations
TEST_F(GPIOTest, WriteAndRead) {
    // Set output direction
    auto result = gpio->setDirection(GPIODirection::Output);
    ASSERT_TRUE(result.isOk()) << "Failed to set direction: " << result.error().toString();

    // Write high
    result = gpio->write(true);
    ASSERT_TRUE(result.isOk()) << "Failed to write: " << result.error().toString();

    // Read value (in this test environment, we should be able to read our output)
    auto readResult = gpio->read();
    ASSERT_TRUE(readResult.isOk()) << "Failed to read: " << readResult.error().toString();
    EXPECT_TRUE(readResult.value());

    // Write low
    result = gpio->write(false);
    ASSERT_TRUE(result.isOk()) << "Failed to write: " << result.error().toString();

    // Read value
    readResult = gpio->read();
    ASSERT_TRUE(readResult.isOk()) << "Failed to read: " << readResult.error().toString();
    EXPECT_FALSE(readResult.value());
}

// Test output blink sequence
TEST_F(GPIOTest, BlinkSequence) {
    // Set output direction
    auto result = gpio->setDirection(GPIODirection::Output);
    ASSERT_TRUE(result.isOk()) << "Failed to set direction: " << result.error().toString();

    // Perform a quick blink sequence (5 times)
    for (int i = 0; i < 5; i++) {
        // Set high
        result = gpio->write(true);
        ASSERT_TRUE(result.isOk()) << "Failed to write high: " << result.error().toString();

        // Verify high
        auto readResult = gpio->read();
        ASSERT_TRUE(readResult.isOk()) << "Failed to read: " << readResult.error().toString();
        EXPECT_TRUE(readResult.value());

        // Brief delay (50ms to keep test fast)
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // Set low
        result = gpio->write(false);
        ASSERT_TRUE(result.isOk()) << "Failed to write low: " << result.error().toString();

        // Verify low
        readResult = gpio->read();
        ASSERT_TRUE(readResult.isOk()) << "Failed to read: " << readResult.error().toString();
        EXPECT_FALSE(readResult.value());

        // Brief delay
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

// Test interrupts and edge detection (API test only)
TEST_F(GPIOTest, InterruptSetup) {
    // Set input direction
    auto result = gpio->setDirection(GPIODirection::Input);
    ASSERT_TRUE(result.isOk()) << "Failed to set direction: " << result.error().toString();

    // Set edge detection to rising
    result = gpio->setEdge(GPIOEdge::Rising);
    ASSERT_TRUE(result.isOk()) << "Failed to set edge detection: " << result.error().toString();
    EXPECT_EQ(gpio->getEdge(), GPIOEdge::Rising);

    // Set edge detection to falling
    result = gpio->setEdge(GPIOEdge::Falling);
    ASSERT_TRUE(result.isOk()) << "Failed to set edge detection: " << result.error().toString();
    EXPECT_EQ(gpio->getEdge(), GPIOEdge::Falling);

    // Set edge detection to both
    result = gpio->setEdge(GPIOEdge::Both);
    ASSERT_TRUE(result.isOk()) << "Failed to set edge detection: " << result.error().toString();
    EXPECT_EQ(gpio->getEdge(), GPIOEdge::Both);

    // Set edge detection to none
    result = gpio->setEdge(GPIOEdge::None);
    ASSERT_TRUE(result.isOk()) << "Failed to set edge detection: " << result.error().toString();
    EXPECT_EQ(gpio->getEdge(), GPIOEdge::None);
}

// Test pull up/down resistors
TEST_F(GPIOTest, PullUpDown) {
    // Test pull-up
    auto result = gpio->setPull(GPIOPull::Up);
    ASSERT_TRUE(result.isOk()) << "Failed to set pull-up: " << result.error().toString();
    EXPECT_EQ(gpio->getPull(), GPIOPull::Up);

    // Test pull-down
    result = gpio->setPull(GPIOPull::Down);
    ASSERT_TRUE(result.isOk()) << "Failed to set pull-down: " << result.error().toString();
    EXPECT_EQ(gpio->getPull(), GPIOPull::Down);

    // Test no pull
    result = gpio->setPull(GPIOPull::None);
    ASSERT_TRUE(result.isOk()) << "Failed to set no pull: " << result.error().toString();
    EXPECT_EQ(gpio->getPull(), GPIOPull::None);
}

// Test helper functions
TEST(GPIOHelperTest, StringConversions) {
    // Test direction to string
    EXPECT_EQ(gpioDirectionToString(GPIODirection::Input), "Input");
    EXPECT_EQ(gpioDirectionToString(GPIODirection::Output), "Output");

    // Test edge to string
    EXPECT_EQ(gpioEdgeToString(GPIOEdge::None), "None");
    EXPECT_EQ(gpioEdgeToString(GPIOEdge::Rising), "Rising");
    EXPECT_EQ(gpioEdgeToString(GPIOEdge::Falling), "Falling");
    EXPECT_EQ(gpioEdgeToString(GPIOEdge::Both), "Both");

    // Test pull to string
    EXPECT_EQ(gpioPullToString(GPIOPull::None), "None");
    EXPECT_EQ(gpioPullToString(GPIOPull::Up), "Pull-Up");
    EXPECT_EQ(gpioPullToString(GPIOPull::Down), "Pull-Down");
}
