#pragma once

#include "sensor.h"
#include <cstdint>
#include <array>
#include <string>

namespace fmus {
namespace sensors {

/**
 * @brief Enumeration of accelerometer measurement ranges
 */
enum class AccelerometerRange : uint8_t {
    Range_2G = 0,   ///< ±2g range
    Range_4G = 1,   ///< ±4g range
    Range_8G = 2,   ///< ±8g range
    Range_16G = 3   ///< ±16g range
};

/**
 * @brief Enumeration of accelerometer data rates
 */
enum class AccelerometerDataRate : uint8_t {
    Rate_1Hz = 0,    ///< 1 Hz data rate
    Rate_10Hz = 1,   ///< 10 Hz data rate
    Rate_25Hz = 2,   ///< 25 Hz data rate
    Rate_50Hz = 3,   ///< 50 Hz data rate
    Rate_100Hz = 4,  ///< 100 Hz data rate
    Rate_200Hz = 5,  ///< 200 Hz data rate
    Rate_400Hz = 6,  ///< 400 Hz data rate
    Rate_800Hz = 7   ///< 800 Hz data rate
};

/**
 * @brief Data structure for accelerometer readings
 */
struct AccelerometerData : public SensorData {
    float x;           ///< X-axis acceleration in g
    float y;           ///< Y-axis acceleration in g
    float z;           ///< Z-axis acceleration in g
    uint64_t timestamp; ///< Timestamp of the reading in milliseconds

    /**
     * @brief Get the magnitude of the acceleration vector
     *
     * @return float The magnitude in g
     */
    float getMagnitude() const;

    /**
     * @brief Check if the acceleration vector indicates free fall
     *
     * @param threshold The threshold for free fall detection (default 0.1g)
     * @return bool True if in free fall
     */
    bool isFreeFall(float threshold = 0.1f) const;
};

/**
 * @brief Configuration for accelerometers
 */
struct AccelerometerConfig : public SensorConfig {
    AccelerometerRange range;         ///< Measurement range
    AccelerometerDataRate dataRate;   ///< Data rate
    bool highResolution;              ///< High resolution mode
    bool lowPower;                    ///< Low power mode

    /**
     * @brief Construct a new Accelerometer Config with default settings
     */
    AccelerometerConfig();
};

/**
 * @brief Class for interfacing with accelerometer sensors
 */
class FMUS_EMBED_API Accelerometer : public Sensor<AccelerometerData, AccelerometerConfig> {
public:
    /**
     * @brief Construct a new Accelerometer
     *
     * @param deviceAddress The I2C address of the device
     */
    explicit Accelerometer(uint8_t deviceAddress);

    /**
     * @brief Destructor
     */
    ~Accelerometer() override;

    /**
     * @brief Initialize the accelerometer
     *
     * @return core::Result<void> Success or error
     */
    core::Result<void> init() override;

    /**
     * @brief Read data from the accelerometer
     *
     * @return core::Result<std::unique_ptr<SensorData>> The accelerometer data or error
     */
    core::Result<std::unique_ptr<SensorData>> read() override;

    /**
     * @brief Calibrate the accelerometer
     *
     * @return core::Result<void> Success or error
     */
    core::Result<void> calibrate() override;

    /**
     * @brief Configure the accelerometer
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
     * @brief Set the measurement range
     *
     * @param range The measurement range
     * @return Accelerometer& This accelerometer for method chaining
     */
    Accelerometer& setRange(AccelerometerRange range);

    /**
     * @brief Set the data rate
     *
     * @param dataRate The data rate
     * @return Accelerometer& This accelerometer for method chaining
     */
    Accelerometer& setDataRate(AccelerometerDataRate dataRate);

    /**
     * @brief Enable or disable high resolution mode
     *
     * @param enable True to enable, false to disable
     * @return Accelerometer& This accelerometer for method chaining
     */
    Accelerometer& setHighResolution(bool enable);

    /**
     * @brief Enable or disable low power mode
     *
     * @param enable True to enable, false to disable
     * @return Accelerometer& This accelerometer for method chaining
     */
    Accelerometer& setLowPower(bool enable);

    /**
     * @brief Get the current measurement range
     *
     * @return AccelerometerRange The measurement range
     */
    AccelerometerRange getRange() const;

    /**
     * @brief Get the current data rate
     *
     * @return AccelerometerDataRate The data rate
     */
    AccelerometerDataRate getDataRate() const;

    /**
     * @brief Check if high resolution mode is enabled
     *
     * @return bool True if enabled
     */
    bool isHighResolutionEnabled() const;

    /**
     * @brief Check if low power mode is enabled
     *
     * @return bool True if enabled
     */
    bool isLowPowerEnabled() const;

private:
    uint8_t m_deviceAddress;               ///< I2C device address
    bool m_initialized;                    ///< Initialization state
    AccelerometerConfig m_config;          ///< Current configuration
    std::array<float, 3> m_calibrationOffset; ///< Calibration offset values
};

/**
 * @brief Get a string representation of an accelerometer range
 *
 * @param range The accelerometer range
 * @return std::string The string representation
 */
FMUS_EMBED_API std::string accelerometerRangeToString(AccelerometerRange range);

/**
 * @brief Get a string representation of an accelerometer data rate
 *
 * @param dataRate The accelerometer data rate
 * @return std::string The string representation
 */
FMUS_EMBED_API std::string accelerometerDataRateToString(AccelerometerDataRate dataRate);

} // namespace sensors
} // namespace fmus
