#pragma once

#include "sensor.h"
#include <cstdint>
#include <string>

namespace fmus {
namespace sensors {

/**
 * @brief Enumeration of pressure sensor types
 */
enum class PressureSensorType : uint8_t {
    BMP280 = 0,       ///< BMP280 pressure sensor
    BMP180 = 1,       ///< BMP180 pressure sensor (older)
    LPS22HB = 2,      ///< LPS22HB pressure sensor
    DPS310 = 3,       ///< DPS310 pressure sensor
    MS5611 = 4,       ///< MS5611 pressure sensor
    MPL3115A2 = 5,    ///< MPL3115A2 pressure sensor
    Generic = 255     ///< Generic pressure sensor
};

/**
 * @brief Enumeration of sensor sampling rates
 */
enum class PressureSampleRate : uint8_t {
    Hz_1 = 0,     ///< 1 sample per second
    Hz_10 = 1,    ///< 10 samples per second
    Hz_25 = 2,    ///< 25 samples per second
    Hz_50 = 3,    ///< 50 samples per second
    Hz_75 = 4,    ///< 75 samples per second
    Hz_100 = 5    ///< 100 samples per second
};

/**
 * @brief Data structure for pressure sensor readings
 */
struct PressureData : public SensorData {
    float pressure;       ///< Pressure in hectopascals (hPa) / millibars
    float temperature;    ///< Temperature in degrees Celsius
    float altitude;       ///< Estimated altitude in meters
    uint64_t timestamp;   ///< Timestamp of the reading in milliseconds

    /**
     * @brief Convert pressure to atm (standard atmospheres)
     *
     * @return float Pressure in atm
     */
    float getAtmospheres() const;

    /**
     * @brief Convert pressure to mmHg (millimeters of mercury)
     *
     * @return float Pressure in mmHg
     */
    float getMmHg() const;

    /**
     * @brief Convert pressure to inHg (inches of mercury)
     *
     * @return float Pressure in inHg
     */
    float getInHg() const;

    /**
     * @brief Get the sea level pressure for the current reading
     *
     * @param altitude Current altitude in meters
     * @return float Sea level pressure in hPa
     */
    float getSeaLevelPressure(float altitude) const;

    /**
     * @brief Predict if weather is likely to change based on pressure trend
     *
     * @param previousPressure Previous pressure reading in hPa
     * @param timeIntervalHours Time interval between readings in hours
     * @return bool True if pressure change indicates weather change
     */
    bool isWeatherChangeLikely(float previousPressure, float timeIntervalHours = 3.0f) const;

    /**
     * @brief Check if pressure indicates fair weather
     *
     * @return bool True if pressure indicates fair weather
     */
    bool isFairWeather() const;
};

/**
 * @brief Configuration for pressure sensors
 */
struct PressureConfig : public SensorConfig {
    PressureSensorType sensorType;    ///< Pressure sensor type
    uint8_t deviceAddress;            ///< I2C address for I2C sensors
    PressureSampleRate sampleRate;    ///< Sample rate
    uint8_t oversamplingRate;         ///< Oversampling rate (sensor specific)
    float seaLevelPressure;           ///< Local sea level reference pressure in hPa
    uint32_t updateInterval;          ///< Minimum time between readings in milliseconds

    /**
     * @brief Construct a new Pressure Config with default settings
     */
    PressureConfig();
};

/**
 * @brief Class for interfacing with pressure sensors
 */
class FMUS_EMBED_API PressureSensor : public Sensor<PressureData, PressureConfig> {
public:
    /**
     * @brief Construct a new Pressure Sensor
     *
     * @param sensorType The type of pressure sensor
     * @param deviceAddress The I2C address of the device
     */
    PressureSensor(PressureSensorType sensorType = PressureSensorType::Generic, uint8_t deviceAddress = 0x76);

    /**
     * @brief Destructor
     */
    ~PressureSensor() override;

    /**
     * @brief Initialize the pressure sensor
     *
     * @return core::Result<void> Success or error
     */
    core::Result<void> init() override;

    /**
     * @brief Read data from the pressure sensor
     *
     * @return core::Result<std::unique_ptr<SensorData>> The pressure data or error
     */
    core::Result<std::unique_ptr<SensorData>> read() override;

    /**
     * @brief Calibrate the pressure sensor
     *
     * @return core::Result<void> Success or error
     */
    core::Result<void> calibrate() override;

    /**
     * @brief Configure the pressure sensor
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
     * @brief Set the pressure sensor type
     *
     * @param sensorType The pressure sensor type
     * @return PressureSensor& This sensor for method chaining
     */
    PressureSensor& setSensorType(PressureSensorType sensorType);

    /**
     * @brief Set the update interval
     *
     * @param interval The update interval in milliseconds
     * @return PressureSensor& This sensor for method chaining
     */
    PressureSensor& setUpdateInterval(uint32_t interval);

    /**
     * @brief Set the sample rate
     *
     * @param sampleRate The sample rate
     * @return PressureSensor& This sensor for method chaining
     */
    PressureSensor& setSampleRate(PressureSampleRate sampleRate);

    /**
     * @brief Set the oversampling rate
     *
     * @param rate The oversampling rate
     * @return PressureSensor& This sensor for method chaining
     */
    PressureSensor& setOversamplingRate(uint8_t rate);

    /**
     * @brief Set the sea level pressure reference
     *
     * @param pressure The sea level pressure in hPa
     * @return PressureSensor& This sensor for method chaining
     */
    PressureSensor& setSeaLevelPressure(float pressure);

    /**
     * @brief Get the pressure sensor type
     *
     * @return PressureSensorType The pressure sensor type
     */
    PressureSensorType getPressureSensorType() const;

    /**
     * @brief Get the update interval
     *
     * @return uint32_t The update interval in milliseconds
     */
    uint32_t getUpdateInterval() const;

    /**
     * @brief Get the sample rate
     *
     * @return PressureSampleRate The sample rate
     */
    PressureSampleRate getSampleRate() const;

    /**
     * @brief Get the oversampling rate
     *
     * @return uint8_t The oversampling rate
     */
    uint8_t getOversamplingRate() const;

    /**
     * @brief Get the sea level pressure reference
     *
     * @return float The sea level pressure in hPa
     */
    float getSeaLevelPressure() const;

private:
    bool m_initialized;              ///< Initialization state
    PressureConfig m_config;         ///< Current configuration
    float m_pressureCalibrationOffset; ///< Pressure calibration offset
    float m_temperatureCalibrationOffset; ///< Temperature calibration offset
    uint64_t m_lastReadTime;         ///< Timestamp of the last reading
    PressureData m_lastReading;      ///< Cache of the last reading
    float m_lastPressureValue;       ///< Last pressure value for tracking trends

    /**
     * @brief Calculate altitude from pressure
     *
     * @param pressure Pressure in hPa
     * @param seaLevelPressure Sea level pressure in hPa
     * @return float Altitude in meters
     */
    float calculateAltitude(float pressure, float seaLevelPressure) const;
};

/**
 * @brief Get a string representation of a pressure sensor type
 *
 * @param sensorType The pressure sensor type
 * @return std::string The string representation
 */
FMUS_EMBED_API std::string pressureSensorTypeToString(PressureSensorType sensorType);

/**
 * @brief Get a string representation of a pressure sample rate
 *
 * @param sampleRate The sample rate
 * @return std::string The string representation
 */
FMUS_EMBED_API std::string pressureSampleRateToString(PressureSampleRate sampleRate);

} // namespace sensors
} // namespace fmus
