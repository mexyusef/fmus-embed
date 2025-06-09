#include <fmus/sensors/accelerometer.h>
#include <fmus/core/logging.h>
#include <cmath>
#include <algorithm>
#include <unordered_map>

namespace fmus {
namespace sensors {

// Mapping of accelerometer ranges to string descriptions
static const std::unordered_map<AccelerometerRange, const char*> rangeStrings = {
    { AccelerometerRange::Range_2G, "±2g" },
    { AccelerometerRange::Range_4G, "±4g" },
    { AccelerometerRange::Range_8G, "±8g" },
    { AccelerometerRange::Range_16G, "±16g" }
};

// Mapping of accelerometer data rates to string descriptions
static const std::unordered_map<AccelerometerDataRate, const char*> dataRateStrings = {
    { AccelerometerDataRate::Rate_1Hz, "1 Hz" },
    { AccelerometerDataRate::Rate_10Hz, "10 Hz" },
    { AccelerometerDataRate::Rate_25Hz, "25 Hz" },
    { AccelerometerDataRate::Rate_50Hz, "50 Hz" },
    { AccelerometerDataRate::Rate_100Hz, "100 Hz" },
    { AccelerometerDataRate::Rate_200Hz, "200 Hz" },
    { AccelerometerDataRate::Rate_400Hz, "400 Hz" },
    { AccelerometerDataRate::Rate_800Hz, "800 Hz" }
};

// Mapping of ranges to actual g values
static const std::unordered_map<AccelerometerRange, float> rangeValues = {
    { AccelerometerRange::Range_2G, 2.0f },
    { AccelerometerRange::Range_4G, 4.0f },
    { AccelerometerRange::Range_8G, 8.0f },
    { AccelerometerRange::Range_16G, 16.0f }
};

float AccelerometerData::getMagnitude() const
{
    return std::sqrt(x * x + y * y + z * z);
}

bool AccelerometerData::isFreeFall(float threshold) const
{
    return getMagnitude() < threshold;
}

AccelerometerConfig::AccelerometerConfig()
    : range(AccelerometerRange::Range_2G),
      dataRate(AccelerometerDataRate::Rate_100Hz),
      highResolution(true),
      lowPower(false)
{
}

Accelerometer::Accelerometer(uint8_t deviceAddress)
    : m_deviceAddress(deviceAddress),
      m_initialized(false),
      m_calibrationOffset{0.0f, 0.0f, 0.0f}
{
}

Accelerometer::~Accelerometer()
{
    // Mematikan sensor jika masih diinisialisasi
    if (m_initialized) {
        // Kode untuk mematikan sensor
    }
}

core::Result<void> Accelerometer::init()
{
    if (m_initialized) {
        return core::makeOk();
    }

    // Kode untuk menginisialisasi sensor akselerometer
    // Catatan: Implementasi ini adalah untuk ilustrasi saja

    FMUS_LOG_INFO("Initializing accelerometer at address 0x" + std::to_string(m_deviceAddress));

    // Simulasi inisialisasi berhasil
    m_initialized = true;

    // Mengkonfigurasi sensor dengan pengaturan default
    setRange(m_config.range);
    setDataRate(m_config.dataRate);
    setHighResolution(m_config.highResolution);
    setLowPower(m_config.lowPower);

    FMUS_LOG_INFO("Accelerometer initialized successfully");

    return core::makeOk();
}

core::Result<std::unique_ptr<SensorData>> Accelerometer::read()
{
    if (!m_initialized) {
        return core::makeError<std::unique_ptr<SensorData>>(
            core::ErrorCode::SensorInitFailed,
            "Accelerometer not initialized"
        );
    }

    // Kode untuk membaca data dari sensor akselerometer
    // Catatan: Implementasi ini adalah untuk ilustrasi saja

    // Simulasi data sensor
    // Dalam implementasi nyata, kita akan membaca dari perangkat I2C
    float rawX = 0.0f;  // Misalnya, ini akan dibaca dari register sensor
    float rawY = 0.0f;
    float rawZ = 1.0f;  // Simulasi gravitasi di sumbu z

    // Konversi data mentah menjadi satuan g
    float rangeValue = rangeValues.at(m_config.range);
    float conversionFactor = rangeValue / 32768.0f;  // Untuk sensor 16-bit

    std::unique_ptr<AccelerometerData> data = std::make_unique<AccelerometerData>();
    data->x = (rawX * conversionFactor) - m_calibrationOffset[0];
    data->y = (rawY * conversionFactor) - m_calibrationOffset[1];
    data->z = (rawZ * conversionFactor) - m_calibrationOffset[2];
    data->timestamp = core::getTimestamp();

    return core::makeOk<std::unique_ptr<SensorData>>(std::move(data));
}

core::Result<void> Accelerometer::calibrate()
{
    if (!m_initialized) {
        return core::makeError<void>(
            core::ErrorCode::SensorInitFailed,
            "Accelerometer not initialized"
        );
    }

    FMUS_LOG_INFO("Calibrating accelerometer");

    // Dalam kalibrasi sebenarnya, kita akan mengambil beberapa sampel
    // dan menghitung offset untuk setiap sumbu

    // Simulasi kalibrasi
    // Misalnya, dengan akselerometer pada permukaan datar,
    // kita mengharapkan (0, 0, 1g)
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

        AccelerometerData* data = dynamic_cast<AccelerometerData*>(result.value().get());
        sumX += data->x;
        sumY += data->y;
        sumZ += data->z - 1.0f;  // Mengurangi 1g untuk gravitasi pada sumbu z

        // Dalam implementasi sebenarnya, kita mungkin akan menambahkan penundaan di sini
    }

    // Menghitung rata-rata offset
    m_calibrationOffset[0] = sumX / numSamples;
    m_calibrationOffset[1] = sumY / numSamples;
    m_calibrationOffset[2] = sumZ / numSamples;

    FMUS_LOG_INFO("Accelerometer calibration complete. Offsets: (" +
                 std::to_string(m_calibrationOffset[0]) + ", " +
                 std::to_string(m_calibrationOffset[1]) + ", " +
                 std::to_string(m_calibrationOffset[2]) + ")");

    return core::makeOk();
}

core::Result<void> Accelerometer::configure(const SensorConfig& config)
{
    // Memeriksa apakah config adalah jenis yang benar
    const AccelerometerConfig* accelConfig = dynamic_cast<const AccelerometerConfig*>(&config);
    if (!accelConfig) {
        return core::makeError<void>(
            core::ErrorCode::InvalidArgument,
            "Invalid configuration type"
        );
    }

    // Mengkonfigurasi sensor
    core::Result<void> result = core::makeOk();

    result = setRange(accelConfig->range).
             setDataRate(accelConfig->dataRate).
             setHighResolution(accelConfig->highResolution).
             setLowPower(accelConfig->lowPower).
             init();

    return result;
}

SensorType Accelerometer::getType() const
{
    return SensorType::Accelerometer;
}

std::string Accelerometer::getName() const
{
    return "Accelerometer";
}

bool Accelerometer::isInitialized() const
{
    return m_initialized;
}

Accelerometer& Accelerometer::setRange(AccelerometerRange range)
{
    if (m_config.range != range) {
        m_config.range = range;

        if (m_initialized) {
            // Kode untuk mengatur range pada perangkat keras
            FMUS_LOG_INFO("Setting accelerometer range to " + accelerometerRangeToString(range));
        }
    }

    return *this;
}

Accelerometer& Accelerometer::setDataRate(AccelerometerDataRate dataRate)
{
    if (m_config.dataRate != dataRate) {
        m_config.dataRate = dataRate;

        if (m_initialized) {
            // Kode untuk mengatur data rate pada perangkat keras
            FMUS_LOG_INFO("Setting accelerometer data rate to " + accelerometerDataRateToString(dataRate));
        }
    }

    return *this;
}

Accelerometer& Accelerometer::setHighResolution(bool enable)
{
    if (m_config.highResolution != enable) {
        m_config.highResolution = enable;

        if (m_initialized) {
            // Kode untuk mengatur mode high resolution pada perangkat keras
            FMUS_LOG_INFO(std::string("Setting accelerometer high resolution mode ") +
                         (enable ? "enabled" : "disabled"));
        }
    }

    return *this;
}

Accelerometer& Accelerometer::setLowPower(bool enable)
{
    if (m_config.lowPower != enable) {
        m_config.lowPower = enable;

        if (m_initialized) {
            // Kode untuk mengatur mode low power pada perangkat keras
            FMUS_LOG_INFO(std::string("Setting accelerometer low power mode ") +
                         (enable ? "enabled" : "disabled"));
        }
    }

    return *this;
}

AccelerometerRange Accelerometer::getRange() const
{
    return m_config.range;
}

AccelerometerDataRate Accelerometer::getDataRate() const
{
    return m_config.dataRate;
}

bool Accelerometer::isHighResolutionEnabled() const
{
    return m_config.highResolution;
}

bool Accelerometer::isLowPowerEnabled() const
{
    return m_config.lowPower;
}

std::string accelerometerRangeToString(AccelerometerRange range)
{
    auto it = rangeStrings.find(range);
    if (it != rangeStrings.end()) {
        return it->second;
    }

    return "Unknown range";
}

std::string accelerometerDataRateToString(AccelerometerDataRate dataRate)
{
    auto it = dataRateStrings.find(dataRate);
    if (it != dataRateStrings.end()) {
        return it->second;
    }

    return "Unknown data rate";
}

} // namespace sensors
} // namespace fmus
