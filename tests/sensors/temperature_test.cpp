#include <fmus/sensors/temperature.h>
#include <gtest/gtest.h>
#include <memory>

using namespace fmus;
using namespace fmus::sensors;

class TemperatureSensorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temperature sensor with a test configuration
        temperatureSensor = std::make_unique<TemperatureSensor>(TemperatureSensorType::DHT22, 5);

        // Initialize the sensor
        auto result = temperatureSensor->init();
        ASSERT_TRUE(result.isOk()) << "Failed to initialize temperature sensor: " << result.error().toString();
    }

    void TearDown() override {
        // Clean up
        temperatureSensor.reset();
    }

    std::unique_ptr<TemperatureSensor> temperatureSensor;
};

// Test initialization
TEST_F(TemperatureSensorTest, Initialization) {
    EXPECT_TRUE(temperatureSensor->isInitialized());
    EXPECT_EQ(temperatureSensor->getType(), SensorType::Temperature);
    EXPECT_EQ(temperatureSensor->getTemperatureSensorType(), TemperatureSensorType::DHT22);
    EXPECT_EQ(temperatureSensor->getName(), "Temperature Sensor (DHT22)");
}

// Test reading data
TEST_F(TemperatureSensorTest, ReadData) {
    // Read data
    auto result = temperatureSensor->read();
    ASSERT_TRUE(result.isOk()) << "Failed to read data: " << result.error().toString();

    // Cast to the appropriate type
    auto* data = dynamic_cast<TemperatureData*>(result.value().get());
    ASSERT_NE(data, nullptr) << "Failed to cast sensor data to TemperatureData";

    // Check data range (DHT22 should provide both temperature and humidity)
    EXPECT_GE(data->temperature, 0.0f);
    EXPECT_LE(data->temperature, 50.0f);
    EXPECT_GE(data->humidity, 0.0f);
    EXPECT_LE(data->humidity, 100.0f);
    EXPECT_GT(data->timestamp, 0);

    // BME280 should also provide pressure, so test with that
    auto bme280Sensor = std::make_unique<TemperatureSensor>(TemperatureSensorType::BME280, 0x76);
    auto initResult = bme280Sensor->init();
    ASSERT_TRUE(initResult.isOk()) << "Failed to initialize BME280 sensor: " << initResult.error().toString();

    auto bmeResult = bme280Sensor->read();
    ASSERT_TRUE(bmeResult.isOk()) << "Failed to read data from BME280: " << bmeResult.error().toString();

    auto* bmeData = dynamic_cast<TemperatureData*>(bmeResult.value().get());
    ASSERT_NE(bmeData, nullptr) << "Failed to cast BME280 data to TemperatureData";

    // Check pressure data is available
    EXPECT_GT(bmeData->pressure, 900.0f);
    EXPECT_LT(bmeData->pressure, 1100.0f);
}

// Test temperature conversions
TEST_F(TemperatureSensorTest, TemperatureConversions) {
    // Create a data instance for testing
    TemperatureData data;
    data.temperature = 25.0f;

    // Test Fahrenheit conversion
    EXPECT_FLOAT_EQ(data.getFahrenheit(), 77.0f);

    // Test Kelvin conversion
    EXPECT_FLOAT_EQ(data.getKelvin(), 298.15f);
}

// Test comfort functions
TEST_F(TemperatureSensorTest, ComfortFunctions) {
    // Create a data instance for testing
    TemperatureData data;

    // Test temperature comfort
    data.temperature = 22.0f; // Comfortable
    EXPECT_TRUE(data.isTemperatureComfortable());

    data.temperature = 15.0f; // Too cold
    EXPECT_FALSE(data.isTemperatureComfortable());

    data.temperature = 30.0f; // Too hot
    EXPECT_FALSE(data.isTemperatureComfortable());

    // Test with custom range
    EXPECT_TRUE(data.isTemperatureComfortable(28.0f, 32.0f));

    // Test humidity comfort
    data.humidity = 45.0f; // Comfortable
    EXPECT_TRUE(data.isHumidityComfortable());

    data.humidity = 20.0f; // Too dry
    EXPECT_FALSE(data.isHumidityComfortable());

    data.humidity = 80.0f; // Too humid
    EXPECT_FALSE(data.isHumidityComfortable());

    // Test with custom range
    EXPECT_TRUE(data.isHumidityComfortable(70.0f, 90.0f));
}

// Test update interval
TEST_F(TemperatureSensorTest, UpdateInterval) {
    // Default update interval is 1000ms
    EXPECT_EQ(temperatureSensor->getUpdateInterval(), 1000u);

    // Read data once
    auto result1 = temperatureSensor->read();
    ASSERT_TRUE(result1.isOk()) << "Failed to read data: " << result1.error().toString();

    // Read again immediately, should get cached data
    auto result2 = temperatureSensor->read();
    ASSERT_TRUE(result2.isOk()) << "Failed to read data: " << result2.error().toString();

    // Cast to the appropriate type
    auto* data1 = dynamic_cast<TemperatureData*>(result1.value().get());
    auto* data2 = dynamic_cast<TemperatureData*>(result2.value().get());

    // Timestamps should be identical since we got cached data
    EXPECT_EQ(data1->timestamp, data2->timestamp);

    // Set a very short update interval
    temperatureSensor->setUpdateInterval(1);

    // Wait briefly
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    // Read again, should get fresh data
    auto result3 = temperatureSensor->read();
    ASSERT_TRUE(result3.isOk()) << "Failed to read data: " << result3.error().toString();

    auto* data3 = dynamic_cast<TemperatureData*>(result3.value().get());

    // Timestamps should be different
    EXPECT_NE(data1->timestamp, data3->timestamp);
}

// Test typed reading
TEST_F(TemperatureSensorTest, TypedReading) {
    // Use the typed reading method
    auto result = temperatureSensor->readTyped();
    ASSERT_TRUE(result.isOk()) << "Failed to read typed data: " << result.error().toString();

    // Access the data directly
    const TemperatureData& data = result.value();

    // Check that the data is valid
    EXPECT_GE(data.temperature, 0.0f);
    EXPECT_LE(data.temperature, 50.0f);
    EXPECT_GE(data.humidity, 0.0f);
    EXPECT_LE(data.humidity, 100.0f);
}

// Test calibration
TEST_F(TemperatureSensorTest, Calibration) {
    // Read uncalibrated data
    auto resultBefore = temperatureSensor->readTyped();
    ASSERT_TRUE(resultBefore.isOk()) << "Failed to read data: " << resultBefore.error().toString();

    // Calibrate
    auto calibrationResult = temperatureSensor->calibrate();
    ASSERT_TRUE(calibrationResult.isOk()) << "Failed to calibrate: " << calibrationResult.error().toString();

    // Read calibrated data
    auto resultAfter = temperatureSensor->readTyped();
    ASSERT_TRUE(resultAfter.isOk()) << "Failed to read data: " << resultAfter.error().toString();

    // In our simulation, calibration doesn't change values significantly
    EXPECT_NEAR(resultBefore.value().temperature, resultAfter.value().temperature, 0.1f);
    EXPECT_NEAR(resultBefore.value().humidity, resultAfter.value().humidity, 0.1f);
}

// Test helper functions
TEST(TemperatureSensorHelperTest, StringConversions) {
    // Test sensor type to string
    EXPECT_EQ(temperatureSensorTypeToString(TemperatureSensorType::DHT11), "DHT11");
    EXPECT_EQ(temperatureSensorTypeToString(TemperatureSensorType::DHT22), "DHT22");
    EXPECT_EQ(temperatureSensorTypeToString(TemperatureSensorType::DS18B20), "DS18B20");
    EXPECT_EQ(temperatureSensorTypeToString(TemperatureSensorType::LM35), "LM35");
    EXPECT_EQ(temperatureSensorTypeToString(TemperatureSensorType::BME280), "BME280");
    EXPECT_EQ(temperatureSensorTypeToString(TemperatureSensorType::SHT31), "SHT31");
    EXPECT_EQ(temperatureSensorTypeToString(TemperatureSensorType::Generic), "Generic");

    // Test invalid type
    EXPECT_EQ(temperatureSensorTypeToString(static_cast<TemperatureSensorType>(255)), "Unknown");
}
