#pragma once

#include "sensor.h"
#include <cstdint>
#include <string>

namespace fmus {
namespace sensors {

/**
 * @brief Enumeration of light sensor types
 */
enum class LightSensorType : uint8_t {
    Generic = 0,       ///< Generic light sensor
    TSL2561 = 1,       ///< TSL2561 light sensor
    BH1750 = 2,        ///< BH1750 light sensor
    VEML7700 = 3,      ///< VEML7700 light sensor
    MAX44009 = 4       ///< MAX44009 light sensor
};

/**
 * @brief Enumeration of light sensor gain settings
 */
enum class LightSensorGain : uint8_t {
    Low = 0,           ///< Low gain
    Medium = 1,        ///< Medium gain
    High = 2,          ///< High gain
    Auto = 3           ///< Automatic gain
};

/**
 * @brief Enumeration of light sensor integration times
 */
enum class LightSensorIntegrationTime : uint8_t {
    Time_13ms = 0,     ///< 13 ms integration time
    Time_101ms = 1,    ///< 101 ms integration time
    Time_402ms = 2,    ///< 402 ms integration time
    Time_Custom = 3    ///< Custom integration time
};

/**
 * @brief Data structure for light sensor readings
 */
struct LightSensorData : public SensorData {
    float lux;             ///< Light intensity in lux
    float infrared;        ///< Infrared light level (if available)
    float visible;         ///< Visible light level (if available)
    uint64_t timestamp;    ///< Timestamp of the reading in milliseconds

    /**
     * @brief Check if it's dark based on light level
     *
     * @param threshold The darkness threshold in lux (default 10.0 lux)
     * @return bool True if dark
     */
    bool isDark(float threshold = 10.0f) const;

    /**
     * @brief Get a description of the light level
     *
     * @return std::string Description (Dark, Dim, Normal, Bright, Very Bright)
     */
    std::string getLightLevelDescription() const;
};

/**
 * @brief Configuration for light sensors
 */
struct LightSensorConfig : public SensorConfig {
    LightSensorType sensorType;                  ///< Type of light sensor
    LightSensorGain gain;                        ///< Gain setting
    LightSensorIntegrationTime integrationTime;  ///< Integration time
    uint8_t deviceAddress;                       ///< I2C device address
    bool continuousMode;                         ///< Continuous measurement mode

    /**
     * @brief Construct a new Light Sensor Config with default settings
     */
    LightSensorConfig();
};

/**
 * @brief Class for interfacing with light sensors
 */
class FMUS_EMBED_API LightSensor : public Sensor<LightSensorData, LightSensorConfig> {
public:
    /**
     * @brief Construct a new Light Sensor
     *
     * @param deviceAddress The I2C address of the device
     * @param sensorType The type of light sensor
     */
    explicit LightSensor(uint8_t deviceAddress, LightSensorType sensorType = LightSensorType::Generic);

    /**
     * @brief Destructor
     */
    ~LightSensor() override;

    /**
     * @brief Initialize the light sensor
     *
     * @return core::Result<void> Success or error
     */
    core::Result<void> init() override;

    /**
     * @brief Read data from the light sensor
     *
     * @return core::Result<std::unique_ptr<SensorData>> The light sensor data or error
     */
    core::Result<std::unique_ptr<SensorData>> read() override;

    /**
     * @brief Calibrate the light sensor
     *
     * @return core::Result<void> Success or error
     */
    core::Result<void> calibrate() override;

    /**
     * @brief Configure the light sensor
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
     * @brief Set the gain
     *
     * @param gain The gain setting
     * @return LightSensor& This light sensor for method chaining
     */
    LightSensor& setGain(LightSensorGain gain);

    /**
     * @brief Set the integration time
     *
     * @param integrationTime The integration time
     * @return LightSensor& This light sensor for method chaining
     */
    LightSensor& setIntegrationTime(LightSensorIntegrationTime integrationTime);

    /**
     * @brief Enable or disable continuous mode
     *
     * @param enable True to enable, false to disable
     * @return LightSensor& This light sensor for method chaining
     */
    LightSensor& setContinuousMode(bool enable);

    /**
     * @brief Get the current gain setting
     *
     * @return LightSensorGain The gain setting
     */
    LightSensorGain getGain() const;

    /**
     * @brief Get the current integration time
     *
     * @return LightSensorIntegrationTime The integration time
     */
    LightSensorIntegrationTime getIntegrationTime() const;

    /**
     * @brief Check if continuous mode is enabled
     *
     * @return bool True if enabled
     */
    bool isContinuousModeEnabled() const;

private:
    bool m_initialized;                ///< Initialization state
    LightSensorConfig m_config;        ///< Current configuration
    float m_calibrationFactor;         ///< Calibration factor
};

/**
 * @brief Get a string representation of a light sensor type
 *
 * @param type The light sensor type
 * @return std::string The string representation
 */
FMUS_EMBED_API std::string lightSensorTypeToString(LightSensorType type);

/**
 * @brief Get a string representation of a light sensor gain
 *
 * @param gain The light sensor gain
 * @return std::string The string representation
 */
FMUS_EMBED_API std::string lightSensorGainToString(LightSensorGain gain);

/**
 * @brief Get a string representation of a light sensor integration time
 *
 * @param integrationTime The light sensor integration time
 * @return std::string The string representation
 */
FMUS_EMBED_API std::string lightSensorIntegrationTimeToString(LightSensorIntegrationTime integrationTime);

} // namespace sensors
} // namespace fmus
