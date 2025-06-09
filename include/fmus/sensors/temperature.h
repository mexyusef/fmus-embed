#pragma once

#include "sensor.h"
#include <cstdint>
#include <string>

namespace fmus {
namespace sensors {

/**
 * @brief Enumeration of temperature sensor types
 */
enum class TemperatureSensorType : uint8_t {
    DHT11 = 0,        ///< DHT11 sensor (low cost)
    DHT22 = 1,        ///< DHT22 sensor (higher precision)
    DS18B20 = 2,      ///< DS18B20 1-Wire sensor
    LM35 = 3,         ///< LM35 analog sensor
    BME280 = 4,       ///< BME280 I2C/SPI sensor
    SHT31 = 5,        ///< SHT31 I2C sensor
    Generic = 255     ///< Generic temperature sensor
};

/**
 * @brief Data structure for temperature sensor readings
 */
struct TemperatureData : public SensorData {
    float temperature;  ///< Temperature in degrees Celsius
    float humidity;     ///< Humidity in percent (0-100), if applicable
    float pressure;     ///< Pressure in hPa, if applicable
    uint64_t timestamp; ///< Timestamp of the reading in milliseconds

    /**
     * @brief Convert temperature to Fahrenheit
     *
     * @return float Temperature in degrees Fahrenheit
     */
    float getFahrenheit() const;

    /**
     * @brief Convert temperature to Kelvin
     *
     * @return float Temperature in Kelvin
     */
    float getKelvin() const;

    /**
     * @brief Check if humidity is in the comfort zone
     *
     * @param minHumidity Minimum comfortable humidity (default 30%)
     * @param maxHumidity Maximum comfortable humidity (default 60%)
     * @return bool True if humidity is in the comfort zone
     */
    bool isHumidityComfortable(float minHumidity = 30.0f, float maxHumidity = 60.0f) const;

    /**
     * @brief Check if temperature is in the comfort zone
     *
     * @param minTemp Minimum comfortable temperature (default 20°C)
     * @param maxTemp Maximum comfortable temperature (default 26°C)
     * @return bool True if temperature is in the comfort zone
     */
    bool isTemperatureComfortable(float minTemp = 20.0f, float maxTemp = 26.0f) const;
};

/**
 * @brief Configuration for temperature sensors
 */
struct TemperatureConfig : public SensorConfig {
    TemperatureSensorType sensorType;  ///< Temperature sensor type
    uint8_t pin;                       ///< Pin number for direct connection sensors
    uint8_t deviceAddress;             ///< I2C address for I2C sensors
    uint32_t updateInterval;           ///< Minimum time between readings in milliseconds

    /**
     * @brief Construct a new Temperature Config with default settings
     */
    TemperatureConfig();
};

/**
 * @brief Class for interfacing with temperature sensors
 */
class FMUS_EMBED_API TemperatureSensor : public Sensor<TemperatureData, TemperatureConfig> {
public:
    /**
     * @brief Construct a new Temperature Sensor
     *
     * @param sensorType The type of temperature sensor
     * @param value The pin number or I2C address depending on sensor type
     * @param isI2C If true, value is treated as I2C address; otherwise as pin number
     */
    TemperatureSensor(TemperatureSensorType sensorType = TemperatureSensorType::Generic,
                     uint8_t value = 0,
                     bool isI2C = false);

    /**
     * @brief Destructor
     */
    ~TemperatureSensor() override;

    /**
     * @brief Initialize the temperature sensor
     *
     * @return core::Result<void> Success or error
     */
    core::Result<void> init() override;

    /**
     * @brief Read data from the temperature sensor
     *
     * @return core::Result<std::unique_ptr<SensorData>> The temperature data or error
     */
    core::Result<std::unique_ptr<SensorData>> read() override;

    /**
     * @brief Calibrate the temperature sensor
     *
     * @return core::Result<void> Success or error
     */
    core::Result<void> calibrate() override;

    /**
     * @brief Configure the temperature sensor
     *
     * @param config The sensor configuration
     * @return core::Result<void> Success or error
     */
    core::Result<void> configure(const SensorConfig& config) override;

    /**
     * @brief Get the sensor type
     *
     * @return SensorType The sensor type
     */
    SensorType getType() const override;

    /**
     * @brief Get the sensor name
     *
     * @return std::string The sensor name
     */
    std::string getName() const override;

    /**
     * @brief Check if the sensor is initialized
     *
     * @return bool True if initialized
     */
    bool isInitialized() const override;

    /**
     * @brief Set the temperature sensor type
     *
     * @param sensorType The temperature sensor type
     * @return TemperatureSensor& This sensor for method chaining
     */
    TemperatureSensor& setSensorType(TemperatureSensorType sensorType);

    /**
     * @brief Set the update interval
     *
     * @param interval The update interval in milliseconds
     * @return TemperatureSensor& This sensor for method chaining
     */
    TemperatureSensor& setUpdateInterval(uint32_t interval);

    /**
     * @brief Get the temperature sensor type
     *
     * @return TemperatureSensorType The temperature sensor type
     */
    TemperatureSensorType getTemperatureSensorType() const;

    /**
     * @brief Get the update interval
     *
     * @return uint32_t The update interval in milliseconds
     */
    uint32_t getUpdateInterval() const;

private:
    bool m_initialized;              ///< Initialization state
    TemperatureConfig m_config;      ///< Current configuration
    float m_tempCalibrationOffset;   ///< Temperature calibration offset
    float m_humidityCalibrationOffset; ///< Humidity calibration offset
    uint64_t m_lastReadTime;         ///< Timestamp of the last reading
    TemperatureData m_lastReading;   ///< Cache of the last reading
};

/**
 * @brief Get a string representation of a temperature sensor type
 *
 * @param sensorType The temperature sensor type
 * @return std::string The string representation
 */
FMUS_EMBED_API std::string temperatureSensorTypeToString(TemperatureSensorType sensorType);

} // namespace sensors
} // namespace fmus
