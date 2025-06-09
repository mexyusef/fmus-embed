#include <fmus/sensors/accelerometer.h>
#include <gtest/gtest.h>
#include <memory>

using namespace fmus;
using namespace fmus::sensors;

class AccelerometerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create an accelerometer with a test address
        accelerometer = std::make_unique<Accelerometer>(0x53);

        // Initialize the accelerometer
        auto result = accelerometer->init();
        ASSERT_TRUE(result.isOk()) << "Failed to initialize accelerometer: " << result.error().toString();
    }

    void TearDown() override {
        // Clean up
        accelerometer.reset();
    }

    std::unique_ptr<Accelerometer> accelerometer;
};

// Test initialization
TEST_F(AccelerometerTest, Initialization) {
    EXPECT_TRUE(accelerometer->isInitialized());
    EXPECT_EQ(accelerometer->getType(), SensorType::Accelerometer);
    EXPECT_EQ(accelerometer->getName(), "Accelerometer");
}

// Test configuration
TEST_F(AccelerometerTest, Configuration) {
    // Default configuration
    EXPECT_EQ(accelerometer->getRange(), AccelerometerRange::Range_2G);
    EXPECT_EQ(accelerometer->getDataRate(), AccelerometerDataRate::Rate_100Hz);
    EXPECT_TRUE(accelerometer->isHighResolutionEnabled());
    EXPECT_FALSE(accelerometer->isLowPowerEnabled());

    // Change configuration
    accelerometer->setRange(AccelerometerRange::Range_8G);
    accelerometer->setDataRate(AccelerometerDataRate::Rate_400Hz);
    accelerometer->setHighResolution(false);
    accelerometer->setLowPower(true);

    // Check new configuration
    EXPECT_EQ(accelerometer->getRange(), AccelerometerRange::Range_8G);
    EXPECT_EQ(accelerometer->getDataRate(), AccelerometerDataRate::Rate_400Hz);
    EXPECT_FALSE(accelerometer->isHighResolutionEnabled());
    EXPECT_TRUE(accelerometer->isLowPowerEnabled());

    // Test fluent interface
    accelerometer->setRange(AccelerometerRange::Range_4G)
                 .setDataRate(AccelerometerDataRate::Rate_200Hz)
                 .setHighResolution(true)
                 .setLowPower(false);

    // Check new configuration
    EXPECT_EQ(accelerometer->getRange(), AccelerometerRange::Range_4G);
    EXPECT_EQ(accelerometer->getDataRate(), AccelerometerDataRate::Rate_200Hz);
    EXPECT_TRUE(accelerometer->isHighResolutionEnabled());
    EXPECT_FALSE(accelerometer->isLowPowerEnabled());
}

// Test reading data
TEST_F(AccelerometerTest, ReadData) {
    // Read data
    auto result = accelerometer->read();
    ASSERT_TRUE(result.isOk()) << "Failed to read data: " << result.error().toString();

    // Cast to the appropriate type
    auto* data = dynamic_cast<AccelerometerData*>(result.value().get());
    ASSERT_NE(data, nullptr) << "Failed to cast sensor data to AccelerometerData";

    // Check data
    // Note: Since our implementation is a simulation, we only check that the data exists
    // In a real test, we might check the specific values
    EXPECT_NEAR(data->z, 1.0f, 0.1f);  // We simulate gravity on the z axis
}

// Test calibration
TEST_F(AccelerometerTest, Calibration) {
    // Calibrate
    auto result = accelerometer->calibrate();
    ASSERT_TRUE(result.isOk()) << "Failed to calibrate: " << result.error().toString();

    // After calibration, the z value should still be close to 1g (gravity)
    // and x, y should be close to 0
    auto readResult = accelerometer->read();
    ASSERT_TRUE(readResult.isOk()) << "Failed to read data: " << readResult.error().toString();

    auto* data = dynamic_cast<AccelerometerData*>(readResult.value().get());
    ASSERT_NE(data, nullptr) << "Failed to cast sensor data to AccelerometerData";

    EXPECT_NEAR(data->x, 0.0f, 0.1f);
    EXPECT_NEAR(data->y, 0.0f, 0.1f);
    EXPECT_NEAR(data->z, 1.0f, 0.1f);
}

// Test free fall detection
TEST_F(AccelerometerTest, FreeFallDetection) {
    // Create a data instance for testing
    AccelerometerData freefall;
    freefall.x = 0.0f;
    freefall.y = 0.0f;
    freefall.z = 0.0f;

    // This should be detected as free fall
    EXPECT_TRUE(freefall.isFreeFall());

    // Create a data instance for non-free fall
    AccelerometerData normal;
    normal.x = 0.0f;
    normal.y = 0.0f;
    normal.z = 1.0f;

    // This should not be detected as free fall
    EXPECT_FALSE(normal.isFreeFall());
}

// Test typed reading
TEST_F(AccelerometerTest, TypedReading) {
    // Use the typed reading method
    auto result = accelerometer->readTyped();
    ASSERT_TRUE(result.isOk()) << "Failed to read typed data: " << result.error().toString();

    // Check the data
    const AccelerometerData& data = result.value();
    EXPECT_NEAR(data.z, 1.0f, 0.1f);
}

// Test helper functions
TEST(AccelerometerHelperTest, StringConversions) {
    // Test range to string
    EXPECT_EQ(accelerometerRangeToString(AccelerometerRange::Range_2G), "±2g");
    EXPECT_EQ(accelerometerRangeToString(AccelerometerRange::Range_4G), "±4g");
    EXPECT_EQ(accelerometerRangeToString(AccelerometerRange::Range_8G), "±8g");
    EXPECT_EQ(accelerometerRangeToString(AccelerometerRange::Range_16G), "±16g");

    // Test data rate to string
    EXPECT_EQ(accelerometerDataRateToString(AccelerometerDataRate::Rate_1Hz), "1 Hz");
    EXPECT_EQ(accelerometerDataRateToString(AccelerometerDataRate::Rate_100Hz), "100 Hz");
    EXPECT_EQ(accelerometerDataRateToString(AccelerometerDataRate::Rate_800Hz), "800 Hz");
}
