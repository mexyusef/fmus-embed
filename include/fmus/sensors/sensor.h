#pragma once

#include "../fmus_config.h"
#include "../core/result.h"
#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace fmus {
namespace sensors {

/**
 * @brief Enumeration of sensor types
 */
enum class SensorType : uint8_t {
    Unknown = 0,       ///< Unknown sensor type
    Accelerometer = 1, ///< Acceleration sensor
    Gyroscope = 2,     ///< Angular velocity sensor
    Magnetometer = 3,  ///< Magnetic field sensor
    Temperature = 4,   ///< Temperature sensor
    Pressure = 5,      ///< Pressure sensor
    Humidity = 6,      ///< Humidity sensor
    Light = 7,         ///< Light intensity sensor
    Proximity = 8,     ///< Proximity sensor
    Current = 9,       ///< Current sensor
    Voltage = 10,      ///< Voltage sensor
    GPS = 11,          ///< GPS location sensor
    IMU = 12           ///< Inertial measurement unit (combined sensors)
};

/**
 * @brief Generic data structure for sensor readings
 */
struct SensorData {
    virtual ~SensorData() = default;
};

/**
 * @brief Generic configuration for sensors
 */
struct SensorConfig {
    virtual ~SensorConfig() = default;
};

/**
 * @brief Base class for all sensors
 */
class FMUS_EMBED_API ISensor {
public:
    virtual ~ISensor() = default;

    /**
     * @brief Initialize the sensor
     *
     * @return core::Result<void> Success or error
     */
    virtual core::Result<void> init() = 0;

    /**
     * @brief Read data from the sensor
     *
     * @return core::Result<std::unique_ptr<SensorData>> The sensor data or error
     */
    virtual core::Result<std::unique_ptr<SensorData>> read() = 0;

    /**
     * @brief Calibrate the sensor
     *
     * @return core::Result<void> Success or error
     */
    virtual core::Result<void> calibrate() = 0;

    /**
     * @brief Configure the sensor
     *
     * @param config The sensor configuration
     * @return core::Result<void> Success or error
     */
    virtual core::Result<void> configure(const SensorConfig& config) = 0;

    /**
     * @brief Get the sensor type
     *
     * @return SensorType The sensor type
     */
    virtual SensorType getType() const = 0;

    /**
     * @brief Get the sensor name
     *
     * @return std::string The sensor name
     */
    virtual std::string getName() const = 0;

    /**
     * @brief Check if the sensor is initialized
     *
     * @return bool True if initialized
     */
    virtual bool isInitialized() const = 0;
};

/**
 * @brief Template class for specific sensor types
 *
 * @tparam T The specific sensor data type
 * @tparam C The specific sensor configuration type
 */
template <typename T, typename C = SensorConfig>
class Sensor : public ISensor {
public:
    /**
     * @brief Read typed data from the sensor
     *
     * @return core::Result<T> The sensor data or error
     */
    virtual core::Result<T> readTyped() {
        auto result = read();
        if (result.isError()) {
            return core::makeError<T>(result.error().code(), result.error().message());
        }

        // Try to cast the generic sensor data to the specific type
        T* typedData = dynamic_cast<T*>(result.value().get());
        if (!typedData) {
            return core::makeError<T>(core::ErrorCode::InvalidArgument,
                                     "Failed to cast sensor data to the expected type");
        }

        // Move the data to a new instance
        T dataCopy = *typedData;
        return core::makeOk<T>(std::move(dataCopy));
    }

    /**
     * @brief Configure the sensor with a typed configuration
     *
     * @param config The typed sensor configuration
     * @return core::Result<void> Success or error
     */
    virtual core::Result<void> configureTyped(const C& config) {
        return configure(config);
    }
};

/**
 * @brief Create a sensor instance by sensor type
 *
 * @param type The sensor type to create
 * @return std::unique_ptr<ISensor> The created sensor or nullptr if type is unsupported
 */
FMUS_EMBED_API std::unique_ptr<ISensor> createSensor(SensorType type);

/**
 * @brief Convert a sensor type to a string
 *
 * @param type The sensor type
 * @return std::string The string representation
 */
FMUS_EMBED_API std::string sensorTypeToString(SensorType type);

} // namespace sensors
} // namespace fmus
