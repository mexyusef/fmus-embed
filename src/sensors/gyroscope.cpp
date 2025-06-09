#include <fmus/sensors/gyroscope.h>
#include <fmus/core/logging.h>
#include <cmath>
#include <algorithm>
#include <unordered_map>

namespace fmus {
namespace sensors {

// Mapping of gyroscope ranges to string descriptions
static const std::unordered_map<GyroscopeRange, const char*> rangeStrings = {
    { GyroscopeRange::Range_250DPS, "±250 dps" },
    { GyroscopeRange::Range_500DPS, "±500 dps" },
    { GyroscopeRange::Range_1000DPS, "±1000 dps" },
    { GyroscopeRange::Range_2000DPS, "±2000 dps" }
};

// Mapping of gyroscope data rates to string descriptions
static const std::unordered_map<GyroscopeDataRate, const char*> dataRateStrings = {
    { GyroscopeDataRate::Rate_10Hz, "10 Hz" },
    { GyroscopeDataRate::Rate_25Hz, "25 Hz" },
    { GyroscopeDataRate::Rate_50Hz, "50 Hz" },
    { GyroscopeDataRate::Rate_100Hz, "100 Hz" },
    { GyroscopeDataRate::Rate_200Hz, "200 Hz" },
    { GyroscopeDataRate::Rate_400Hz, "400 Hz" },
    { GyroscopeDataRate::Rate_800Hz, "800 Hz" },
    { GyroscopeDataRate::Rate_1600Hz, "1600 Hz" }
};

// Mapping of ranges to actual dps values
static const std::unordered_map<GyroscopeRange, float> rangeValues = {
    { GyroscopeRange::Range_250DPS, 250.0f },
    { GyroscopeRange::Range_500DPS, 500.0f },
    { GyroscopeRange::Range_1000DPS, 1000.0f },
    { GyroscopeRange::Range_2000DPS, 2000.0f }
};

float GyroscopeData::getMagnitude() const
{
    return std::sqrt(x * x + y * y + z * z);
}

bool GyroscopeData::isStationary(float threshold) const
{
    return getMagnitude() < threshold;
}

GyroscopeConfig::GyroscopeConfig()
    : range(GyroscopeRange::Range_250DPS),
      dataRate(GyroscopeDataRate::Rate_100Hz),
      highPassFilter(false),
      lowPowerMode(false)
{
}

Gyroscope::Gyroscope(uint8_t deviceAddress)
    : m_deviceAddress(deviceAddress),
      m_initialized(false),
      m_calibrationOffset{0.0f, 0.0f, 0.0f}
{
}

Gyroscope::~Gyroscope()
{
    // Shutdown sensor if still initialized
    if (m_initialized) {
        // Code to shutdown sensor
    }
}

core::Result<void> Gyroscope::init()
{
    if (m_initialized) {
        return core::makeOk();
    }

    // Code to initialize gyroscope sensor
    // Note: This implementation is for illustration only

    FMUS_LOG_INFO("Initializing gyroscope at address 0x" + std::to_string(m_deviceAddress));

    // Simulate successful initialization
    m_initialized = true;

    // Configure sensor with default settings
    setRange(m_config.range);
    setDataRate(m_config.dataRate);
    setHighPassFilter(m_config.highPassFilter);
    setLowPowerMode(m_config.lowPowerMode);

    FMUS_LOG_INFO("Gyroscope initialized successfully");

    return core::makeOk();
}

core::Result<std::unique_ptr<SensorData>> Gyroscope::read()
{
    if (!m_initialized) {
        return core::makeError<std::unique_ptr<SensorData>>(
            core::ErrorCode::SensorInitFailed,
            "Gyroscope not initialized"
        );
    }

    // Code to read data from gyroscope sensor
    // Note: This implementation is for illustration only

    // Simulate sensor data
    // In a real implementation, we would read from I2C device
    float rawX = 0.0f;  // This would be read from sensor register
    float rawY = 0.0f;
    float rawZ = 0.0f;

    // Convert raw data to degrees per second
    float rangeValue = rangeValues.at(m_config.range);
    float conversionFactor = rangeValue / 32768.0f;  // For 16-bit sensor

    std::unique_ptr<GyroscopeData> data = std::make_unique<GyroscopeData>();
    data->x = (rawX * conversionFactor) - m_calibrationOffset[0];
    data->y = (rawY * conversionFactor) - m_calibrationOffset[1];
    data->z = (rawZ * conversionFactor) - m_calibrationOffset[2];
    data->timestamp = core::getTimestamp();

    return core::makeOk<std::unique_ptr<SensorData>>(std::move(data));
}

core::Result<void> Gyroscope::calibrate()
{
    if (!m_initialized) {
        return core::makeError<void>(
            core::ErrorCode::SensorInitFailed,
            "Gyroscope not initialized"
        );
    }

    FMUS_LOG_INFO("Calibrating gyroscope");

    // In a real calibration, we would take multiple samples
    // and calculate the average offset for each axis

    // Simulate calibration
    // Assuming the device is stationary, we expect (0, 0, 0)
    const int numSamples = 10;
    float sumX = 0.0f, sumY = 0.0f, sumZ = 0.0f;

    for (int i = 0; i < numSamples; ++i) {
        auto result = read();
        if (result.isError()) {
            return core::makeError<void>(
                result.error().code(),
                "Calibration failed: " + result.error().message()
            );
        }

        GyroscopeData* data = dynamic_cast<GyroscopeData*>(result.value().get());
        sumX += data->x;
        sumY += data->y;
        sumZ += data->z;

        // In a real implementation, we might add a delay here
    }

    // Calculate average offsets
    m_calibrationOffset[0] = sumX / numSamples;
    m_calibrationOffset[1] = sumY / numSamples;
    m_calibrationOffset[2] = sumZ / numSamples;

    FMUS_LOG_INFO("Gyroscope calibration complete. Offsets: (" +
                 std::to_string(m_calibrationOffset[0]) + ", " +
                 std::to_string(m_calibrationOffset[1]) + ", " +
                 std::to_string(m_calibrationOffset[2]) + ")");

    return core::makeOk();
}

core::Result<void> Gyroscope::configure(const SensorConfig& config)
{
    // Check if config is the correct type
    const GyroscopeConfig* gyroConfig = dynamic_cast<const GyroscopeConfig*>(&config);
    if (!gyroConfig) {
        return core::makeError<void>(
            core::ErrorCode::InvalidArgument,
            "Invalid configuration type"
        );
    }

    // Configure sensor
    core::Result<void> result = core::makeOk();

    result = setRange(gyroConfig->range).
             setDataRate(gyroConfig->dataRate).
             setHighPassFilter(gyroConfig->highPassFilter).
             setLowPowerMode(gyroConfig->lowPowerMode).
             init();

    return result;
}

SensorType Gyroscope::getType() const
{
    return SensorType::Gyroscope;
}

std::string Gyroscope::getName() const
{
    return "Gyroscope";
}

bool Gyroscope::isInitialized() const
{
    return m_initialized;
}

Gyroscope& Gyroscope::setRange(GyroscopeRange range)
{
    m_config.range = range;

    if (m_initialized) {
        // In a real implementation, we would update the sensor register
        FMUS_LOG_INFO("Setting gyroscope range to " + gyroscopeRangeToString(range));
    }

    return *this;
}

Gyroscope& Gyroscope::setDataRate(GyroscopeDataRate dataRate)
{
    m_config.dataRate = dataRate;

    if (m_initialized) {
        // In a real implementation, we would update the sensor register
        FMUS_LOG_INFO("Setting gyroscope data rate to " + gyroscopeDataRateToString(dataRate));
    }

    return *this;
}

Gyroscope& Gyroscope::setHighPassFilter(bool enable)
{
    m_config.highPassFilter = enable;

    if (m_initialized) {
        // In a real implementation, we would update the sensor register
        FMUS_LOG_INFO("Setting gyroscope high-pass filter to " + std::string(enable ? "enabled" : "disabled"));
    }

    return *this;
}

Gyroscope& Gyroscope::setLowPowerMode(bool enable)
{
    m_config.lowPowerMode = enable;

    if (m_initialized) {
        // In a real implementation, we would update the sensor register
        FMUS_LOG_INFO("Setting gyroscope low power mode to " + std::string(enable ? "enabled" : "disabled"));
    }

    return *this;
}

GyroscopeRange Gyroscope::getRange() const
{
    return m_config.range;
}

GyroscopeDataRate Gyroscope::getDataRate() const
{
    return m_config.dataRate;
}

bool Gyroscope::isHighPassFilterEnabled() const
{
    return m_config.highPassFilter;
}

bool Gyroscope::isLowPowerModeEnabled() const
{
    return m_config.lowPowerMode;
}

std::string gyroscopeRangeToString(GyroscopeRange range)
{
    auto it = rangeStrings.find(range);
    if (it != rangeStrings.end()) {
        return it->second;
    }
    return "Unknown";
}

std::string gyroscopeDataRateToString(GyroscopeDataRate dataRate)
{
    auto it = dataRateStrings.find(dataRate);
    if (it != dataRateStrings.end()) {
        return it->second;
    }
    return "Unknown";
}

} // namespace sensors
} // namespace fmus
