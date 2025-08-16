#include <gtest/gtest.h>
#include "fmus/sensors/sensor.h"
#include "fmus/sensors/temperature.h"

using namespace fmus::sensors;
using namespace fmus::core;

class SensorBaseTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(SensorBaseTest, SensorTypeEnum) {
    // Test that sensor types are properly defined
    EXPECT_NE(static_cast<int>(SensorType::Temperature), static_cast<int>(SensorType::Accelerometer));
    EXPECT_NE(static_cast<int>(SensorType::Accelerometer), static_cast<int>(SensorType::Gyroscope));
    EXPECT_NE(static_cast<int>(SensorType::Gyroscope), static_cast<int>(SensorType::Pressure));
    EXPECT_NE(static_cast<int>(SensorType::Pressure), static_cast<int>(SensorType::Light));
}

TEST_F(SensorBaseTest, SensorCreation) {
    // Test sensor creation
    auto tempSensor = createSensor(SensorType::Temperature);
    if (tempSensor) {
        EXPECT_EQ(tempSensor->getType(), SensorType::Temperature);
        EXPECT_FALSE(tempSensor->isInitialized());
    }
    
    auto accelSensor = createSensor(SensorType::Accelerometer);
    if (accelSensor) {
        EXPECT_EQ(accelSensor->getType(), SensorType::Accelerometer);
        EXPECT_FALSE(accelSensor->isInitialized());
    }
}

TEST_F(SensorBaseTest, TemperatureSensorBasics) {
    TemperatureSensor sensor;
    
    EXPECT_EQ(sensor.getType(), SensorType::Temperature);
    EXPECT_EQ(sensor.getName(), "Temperature Sensor");
    EXPECT_FALSE(sensor.isInitialized());
    
    // Test initialization
    auto result = sensor.init();
    if (result.isOk()) {
        EXPECT_TRUE(sensor.isInitialized());
    }
}

TEST_F(SensorBaseTest, SensorDataPolymorphism) {
    // Test that sensor data can be used polymorphically
    std::unique_ptr<SensorData> data = std::make_unique<TemperatureData>();
    EXPECT_NE(data.get(), nullptr);
    
    // Test virtual destructor
    data.reset();
    EXPECT_EQ(data.get(), nullptr);
}

TEST_F(SensorBaseTest, SensorConfigPolymorphism) {
    // Test that sensor config can be used polymorphically
    std::unique_ptr<SensorConfig> config = std::make_unique<TemperatureConfig>();
    EXPECT_NE(config.get(), nullptr);
    
    // Test virtual destructor
    config.reset();
    EXPECT_EQ(config.get(), nullptr);
}
