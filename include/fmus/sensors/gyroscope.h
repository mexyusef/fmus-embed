#pragma once

#include "sensor.h"
#include <cstdint>
#include <array>
#include <string>

namespace fmus {
namespace sensors {

/**
 * @brief Enumeration of gyroscope measurement ranges
 */
enum class GyroscopeRange : uint8_t {
    Range_250DPS = 0,    ///< ±250 degrees per second range
    Range_500DPS = 1,    ///< ±500 degrees per second range
    Range_1000DPS = 2,   ///< ±1000 degrees per second range
    Range_2000DPS = 3    ///< ±2000 degrees per second range
};

/**
 * @brief Enumeration of gyroscope data rates
 */
enum class GyroscopeDataRate : uint8_t {
    Rate_10Hz = 0,    ///< 10 Hz data rate
    Rate_25Hz = 1,    ///< 25 Hz data rate
    Rate_50Hz = 2,    ///< 50 Hz data rate
    Rate_100Hz = 3,   ///< 100 Hz data rate
    Rate_200Hz = 4,   ///< 200 Hz data rate
    Rate_400Hz = 5,   ///< 400 Hz data rate
    Rate_800Hz = 6,   ///< 800 Hz data rate
    Rate_1600Hz = 7   ///< 1600 Hz data rate
};

/**
 * @brief Data structure for gyroscope readings
 */
struct GyroscopeData : public SensorData {
    float x;           ///< X-axis angular velocity in degrees per second
    float y;           ///< Y-axis angular velocity in degrees per second
    float z;           ///< Z-axis angular velocity in degrees per second
    uint64_t timestamp; ///< Timestamp of the reading in milliseconds

    /**
     * @brief Get the magnitude of the angular velocity vector
     *
     * @return float The magnitude in degrees per second
     */
    float getMagnitude() const;

    /**
     * @brief Check if the gyroscope is stationary
     *
     * @param threshold The threshold for motion detection (default 1.0 dps)
     * @return bool True if stationary
     */
    bool isStationary(float threshold = 1.0f) const;
};

/**
 * @brief Configuration for gyroscopes
 */
struct GyroscopeConfig : public SensorConfig {
    GyroscopeRange range;         ///< Measurement range
    GyroscopeDataRate dataRate;   ///< Data rate
    bool highPassFilter;          ///< High-pass filter enabled
    bool lowPowerMode;            ///< Low power mode

    /**
     * @brief Construct a new Gyroscope Config with default settings
     */
    GyroscopeConfig();
};

/**
 * @brief Class for interfacing with gyroscope sensors
 */
class FMUS_EMBED_API Gyroscope : public Sensor<GyroscopeData, GyroscopeConfig> {
public:
    /**
     * @brief Construct a new Gyroscope
     *
     * @param deviceAddress The I2C address of the device
     */
    explicit Gyroscope(uint8_t deviceAddress);

    /**
     * @brief Destructor
     */
    ~Gyroscope() override;

    /**
     * @brief Initialize the gyroscope
     *
     * @return core::Result<void> Success or error
     */
    core::Result<void> init() override;

    /**
     * @brief Read data from the gyroscope
     *
     * @return core::Result<std::unique_ptr<SensorData>> The gyroscope data or error
     */
    core::Result<std::unique_ptr<SensorData>> read() override;

    /**
     * @brief Calibrate the gyroscope
     *
     * @return core::Result<void> Success or error
     */
    core::Result<void> calibrate() override;

    /**
     * @brief Configure the gyroscope
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
     * @return Gyroscope& This gyroscope for method chaining
     */
    Gyroscope& setRange(GyroscopeRange range);

    /**
     * @brief Set the data rate
     *
     * @param dataRate The data rate
     * @return Gyroscope& This gyroscope for method chaining
     */
    Gyroscope& setDataRate(GyroscopeDataRate dataRate);

    /**
     * @brief Enable or disable high-pass filter
     *
     * @param enable True to enable, false to disable
     * @return Gyroscope& This gyroscope for method chaining
     */
    Gyroscope& setHighPassFilter(bool enable);

    /**
     * @brief Enable or disable low power mode
     *
     * @param enable True to enable, false to disable
     * @return Gyroscope& This gyroscope for method chaining
     */
    Gyroscope& setLowPowerMode(bool enable);

    /**
     * @brief Get the current measurement range
     *
     * @return GyroscopeRange The measurement range
     */
    GyroscopeRange getRange() const;

    /**
     * @brief Get the current data rate
     *
     * @return GyroscopeDataRate The data rate
     */
    GyroscopeDataRate getDataRate() const;

    /**
     * @brief Check if high-pass filter is enabled
     *
     * @return bool True if enabled
     */
    bool isHighPassFilterEnabled() const;

    /**
     * @brief Check if low power mode is enabled
     *
     * @return bool True if enabled
     */
    bool isLowPowerModeEnabled() const;

private:
    uint8_t m_deviceAddress;               ///< I2C device address
    bool m_initialized;                    ///< Initialization state
    GyroscopeConfig m_config;              ///< Current configuration
    std::array<float, 3> m_calibrationOffset; ///< Calibration offset values
};

/**
 * @brief Get a string representation of a gyroscope range
 *
 * @param range The gyroscope range
 * @return std::string The string representation
 */
FMUS_EMBED_API std::string gyroscopeRangeToString(GyroscopeRange range);

/**
 * @brief Get a string representation of a gyroscope data rate
 *
 * @param dataRate The gyroscope data rate
 * @return std::string The string representation
 */
FMUS_EMBED_API std::string gyroscopeDataRateToString(GyroscopeDataRate dataRate);

} // namespace sensors
} // namespace fmus
