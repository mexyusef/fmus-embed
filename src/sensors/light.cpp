#include <fmus/sensors/light.h>
#include <fmus/core/logging.h>
#include <unordered_map>
#include <cmath>

namespace fmus {
namespace sensors {

// Mapping of light sensor types to string descriptions
static const std::unordered_map<LightSensorType, const char*> sensorTypeStrings = {
    { LightSensorType::Generic, "Generic Light Sensor" },
    { LightSensorType::TSL2561, "TSL2561" },
    { LightSensorType::BH1750, "BH1750" },
    { LightSensorType::VEML7700, "VEML7700" },
    { LightSensorType::MAX44009, "MAX44009" }
};

// Mapping of gain settings to string descriptions
static const std::unordered_map<LightSensorGain, const char*> gainStrings = {
    { LightSensorGain::Low, "Low" },
    { LightSensorGain::Medium, "Medium" },
    { LightSensorGain::High, "High" },
    { LightSensorGain::Auto, "Auto" }
};

// Mapping of integration times to string descriptions
static const std::unordered_map<LightSensorIntegrationTime, const char*> integrationTimeStrings = {
    { LightSensorIntegrationTime::Time_13ms, "13 ms" },
    { LightSensorIntegrationTime::Time_101ms, "101 ms" },
    { LightSensorIntegrationTime::Time_402ms, "402 ms" },
    { LightSensorIntegrationTime::Time_Custom, "Custom" }
};

// Light level thresholds for descriptions
static const std::vector<std::pair<float, const char*>> lightLevelThresholds = {
    { 10.0f, "Dark" },
    { 50.0f, "Dim" },
    { 1000.0f, "Normal" },
    { 10000.0f, "Bright" },
    { std::numeric_limits<float>::max(), "Very Bright" }
};

bool LightSensorData::isDark(float threshold) const
{
    return lux < threshold;
}

std::string LightSensorData::getLightLevelDescription() const
{
    for (const auto& level : lightLevelThresholds) {
        if (lux < level.first) {
            return level.second;
        }
    }
    return "Unknown";
}

LightSensorConfig::LightSensorConfig()
    : sensorType(LightSensorType::Generic),
      gain(LightSensorGain::Medium),
      integrationTime(LightSensorIntegrationTime::Time_101ms),
      deviceAddress(0),
      continuousMode(true)
{
}

LightSensor::LightSensor(uint8_t deviceAddress, LightSensorType sensorType)
    : m_initialized(false),
      m_calibrationFactor(1.0f)
{
    m_config.deviceAddress = deviceAddress;
    m_config.sensorType = sensorType;
}

LightSensor::~LightSensor()
{
    // Shutdown sensor if still initialized
    if (m_initialized) {
        // Code to shutdown sensor
    }
}

core::Result<void> LightSensor::init()
{
    if (m_initialized) {
        return core::makeOk();
    }

    FMUS_LOG_INFO("Initializing light sensor: " + lightSensorTypeToString(m_config.sensorType) +
                 " at address 0x" + std::to_string(m_config.deviceAddress));

    // For I2C sensors, ensure address is configured
    if (m_config.deviceAddress == 0) {
        return core::Error(core::ErrorCode::InvalidArgument,
                        "Device address must be specified for light sensors");
    }

    // Different initialization for different sensor types
    switch (m_config.sensorType) {
        case LightSensorType::TSL2561:
            // Initialize TSL2561 sensor
            // ...
            break;

        case LightSensorType::BH1750:
            // Initialize BH1750 sensor
            // ...
            break;

        case LightSensorType::VEML7700:
        case LightSensorType::MAX44009:
            // Initialize other sensor types
            // ...
            break;

        case LightSensorType::Generic:
        default:
            // Generic sensor doesn't need special initialization
            break;
    }

    // Note: In a real implementation, we would perform hardware initialization
    // For simulation, we just set the initialization flag
    m_initialized = true;

    // Configure sensor with current settings
    setGain(m_config.gain);
    setIntegrationTime(m_config.integrationTime);
    setContinuousMode(m_config.continuousMode);

    FMUS_LOG_INFO("Light sensor initialized successfully");

    return core::makeOk();
}

core::Result<std::unique_ptr<SensorData>> LightSensor::read()
{
    if (!m_initialized) {
        return core::makeError<std::unique_ptr<SensorData>>(
            core::ErrorCode::SensorInitFailed,
            "Light sensor not initialized"
        );
    }

    // Code to read data from light sensor
    // Note: This implementation is for illustration only

    // Simulate sensor data
    // In a real implementation, we would read from I2C device
    float rawLux = 100.0f;  // This would be read from sensor register
    float rawIR = 20.0f;    // Only available on some sensors
    float rawVisible = 80.0f; // Only available on some sensors

    // Apply calibration factor
    std::unique_ptr<LightSensorData> data = std::make_unique<LightSensorData>();
    data->lux = rawLux * m_calibrationFactor;
    data->infrared = rawIR;
    data->visible = rawVisible;
    data->timestamp = core::getTimestamp();

    return core::makeOk<std::unique_ptr<SensorData>>(std::move(data));
}

core::Result<void> LightSensor::calibrate()
{
    if (!m_initialized) {
        return core::makeError<void>(
            core::ErrorCode::SensorInitFailed,
            "Light sensor not initialized"
        );
    }

    FMUS_LOG_INFO("Calibrating light sensor");

    // In a real calibration, we might compare against a reference light source
    // For simulation, we just set a calibration factor

    // Simulate calibration
    const float referenceValue = 100.0f;  // Reference light level in lux

    auto result = read();
    if (result.isError()) {
        return core::makeError<void>(
            result.error().code(),
            "Calibration failed: " + result.error().message()
        );
    }

    LightSensorData* data = dynamic_cast<LightSensorData*>(result.value().get());
    if (data->lux > 0) {
        m_calibrationFactor = referenceValue / data->lux;
    } else {
        m_calibrationFactor = 1.0f;
    }

    FMUS_LOG_INFO("Light sensor calibration complete. Calibration factor: " +
                 std::to_string(m_calibrationFactor));

    return core::makeOk();
}

core::Result<void> LightSensor::configure(const SensorConfig& config)
{
    // Check if config is the correct type
    const LightSensorConfig* lightConfig = dynamic_cast<const LightSensorConfig*>(&config);
    if (!lightConfig) {
        return core::makeError<void>(
            core::ErrorCode::InvalidArgument,
            "Invalid configuration type"
        );
    }

    // Store the configuration
    m_config = *lightConfig;

    // Apply configuration if already initialized
    if (m_initialized) {
        setGain(m_config.gain);
        setIntegrationTime(m_config.integrationTime);
        setContinuousMode(m_config.continuousMode);
    }

    return core::makeOk();
}

SensorType LightSensor::getType() const
{
    return SensorType::Light;
}

std::string LightSensor::getName() const
{
    return lightSensorTypeToString(m_config.sensorType);
}

bool LightSensor::isInitialized() const
{
    return m_initialized;
}

LightSensor& LightSensor::setGain(LightSensorGain gain)
{
    m_config.gain = gain;

    if (m_initialized) {
        // In a real implementation, we would update the sensor register
        FMUS_LOG_INFO("Setting light sensor gain to " + lightSensorGainToString(gain));
    }

    return *this;
}

LightSensor& LightSensor::setIntegrationTime(LightSensorIntegrationTime integrationTime)
{
    m_config.integrationTime = integrationTime;

    if (m_initialized) {
        // In a real implementation, we would update the sensor register
        FMUS_LOG_INFO("Setting light sensor integration time to " +
                     lightSensorIntegrationTimeToString(integrationTime));
    }

    return *this;
}

LightSensor& LightSensor::setContinuousMode(bool enable)
{
    m_config.continuousMode = enable;

    if (m_initialized) {
        // In a real implementation, we would update the sensor register
        FMUS_LOG_INFO("Setting light sensor continuous mode to " +
                     std::string(enable ? "enabled" : "disabled"));
    }

    return *this;
}

LightSensorGain LightSensor::getGain() const
{
    return m_config.gain;
}

LightSensorIntegrationTime LightSensor::getIntegrationTime() const
{
    return m_config.integrationTime;
}

bool LightSensor::isContinuousModeEnabled() const
{
    return m_config.continuousMode;
}

std::string lightSensorTypeToString(LightSensorType type)
{
    auto it = sensorTypeStrings.find(type);
    if (it != sensorTypeStrings.end()) {
        return it->second;
    }
    return "Unknown";
}

std::string lightSensorGainToString(LightSensorGain gain)
{
    auto it = gainStrings.find(gain);
    if (it != gainStrings.end()) {
        return it->second;
    }
    return "Unknown";
}

std::string lightSensorIntegrationTimeToString(LightSensorIntegrationTime integrationTime)
{
    auto it = integrationTimeStrings.find(integrationTime);
    if (it != integrationTimeStrings.end()) {
        return it->second;
    }
    return "Unknown";
}

} // namespace sensors
} // namespace fmus
