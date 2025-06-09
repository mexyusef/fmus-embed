#include <fmus/sensors/pressure.h>
#include <fmus/core/error.h>
#include <fmus/core/logging.h>
#include <chrono>
#include <cmath>
#include <thread>
#include <random>

namespace fmus {
namespace sensors {

// Konstanta untuk konversi satuan
constexpr float HPA_TO_ATM = 1.0f / 1013.25f;
constexpr float HPA_TO_MMHG = 0.750062f;
constexpr float HPA_TO_INHG = 0.02953f;
constexpr float STANDARD_SEA_LEVEL_PRESSURE = 1013.25f; // hPa
constexpr float FAIR_WEATHER_PRESSURE_THRESHOLD = 1020.0f; // hPa
constexpr float PRESSURE_CHANGE_THRESHOLD = 1.6f; // hPa in 3 hours indicates weather change

// Implementasi metode PressureData
float PressureData::getAtmospheres() const {
    // Konversi dari hPa ke atm
    return pressure * HPA_TO_ATM;
}

float PressureData::getMmHg() const {
    // Konversi dari hPa ke mmHg (millimeters of mercury)
    return pressure * HPA_TO_MMHG;
}

float PressureData::getInHg() const {
    // Konversi dari hPa ke inHg (inches of mercury)
    return pressure * HPA_TO_INHG;
}

float PressureData::getSeaLevelPressure(float altitude) const {
    // Konversi tekanan saat ini ke tekanan permukaan laut menggunakan formula barometrik
    return pressure / std::pow(1.0f - (altitude / 44330.0f), 5.255f);
}

bool PressureData::isWeatherChangeLikely(float previousPressure, float timeIntervalHours) const {
    // Periksa apakah perubahan tekanan mengindikasikan perubahan cuaca
    // Perubahan > 1.6 hPa dalam 3 jam dianggap signifikan
    float rateOfChange = std::abs(pressure - previousPressure) / timeIntervalHours;
    float threshold = PRESSURE_CHANGE_THRESHOLD * (3.0f / timeIntervalHours);
    return rateOfChange > threshold;
}

bool PressureData::isFairWeather() const {
    // Tekanan tinggi biasanya mengindikasikan cuaca cerah
    return pressure > FAIR_WEATHER_PRESSURE_THRESHOLD;
}

// Implementasi konstruktor PressureConfig dengan nilai default
PressureConfig::PressureConfig()
    : sensorType(PressureSensorType::BMP280),
      deviceAddress(0x76), // Default address untuk BMP280
      sampleRate(PressureSampleRate::Hz_10),
      oversamplingRate(3), // Default oversampling x4
      seaLevelPressure(STANDARD_SEA_LEVEL_PRESSURE),
      updateInterval(1000) // Default 1 detik
{
}

// Implementasi PressureSensor
PressureSensor::PressureSensor(PressureSensorType sensorType, uint8_t deviceAddress)
    : m_initialized(false),
      m_pressureCalibrationOffset(0.0f),
      m_temperatureCalibrationOffset(0.0f),
      m_lastReadTime(0),
      m_lastPressureValue(0.0f)
{
    m_config.sensorType = sensorType;
    m_config.deviceAddress = deviceAddress;
    m_config.sampleRate = PressureSampleRate::Hz_10;
    m_config.oversamplingRate = 3;
    m_config.seaLevelPressure = STANDARD_SEA_LEVEL_PRESSURE;
    m_config.updateInterval = 1000; // Default 1 detik

    // Inisialisasi data pembacaan terakhir
    m_lastReading.pressure = 0.0f;
    m_lastReading.temperature = 0.0f;
    m_lastReading.altitude = 0.0f;
    m_lastReading.timestamp = 0;
}

PressureSensor::~PressureSensor() {
    // Pembersihan khusus tidak diperlukan
}

core::Result<void> PressureSensor::init() {
    FMUS_LOG_INFO("Initializing pressure sensor: " +
                 pressureSensorTypeToString(m_config.sensorType));

    // Untuk sensor I2C, pastikan alamat dikonfigurasi dengan benar
    if (m_config.deviceAddress == 0) {
        return core::Error(core::ErrorCode::InvalidArgument,
                        "Device address must be specified for pressure sensors");
    }

    // Implementasi inisialisasi yang berbeda untuk tipe sensor yang berbeda
    switch (m_config.sensorType) {
        case PressureSensorType::BMP280:
        case PressureSensorType::BMP180:
            // Inisialisasi koneksi I2C dan register untuk BMP280/BMP180
            // ...
            break;

        case PressureSensorType::LPS22HB:
        case PressureSensorType::DPS310:
        case PressureSensorType::MS5611:
        case PressureSensorType::MPL3115A2:
            // Inisialisasi koneksi dan register untuk sensor lain
            // ...
            break;

        case PressureSensorType::Generic:
        default:
            // Sensor generik tidak memerlukan inisialisasi khusus
            break;
    }

    // Catatan: Dalam implementasi nyata, kita akan melakukan inisialisasi hardware
    // Untuk simulasi, kita hanya mengatur flag inisialisasi
    m_initialized = true;

    // Lakukan pembacaan awal untuk memastikan sensor berfungsi
    auto result = read();
    if (result.isError()) {
        m_initialized = false;
        return core::Error(core::ErrorCode::SensorInitFailed,
                         "Failed to perform initial reading: " + result.error().message());
    }

    // Simpan nilai awal untuk tracking trend
    m_lastPressureValue = m_lastReading.pressure;

    FMUS_LOG_INFO("Pressure sensor initialized successfully");
    return {};
}

core::Result<std::unique_ptr<SensorData>> PressureSensor::read() {
    if (!m_initialized) {
        return core::Error(core::ErrorCode::SensorReadError,
                         "Pressure sensor not initialized");
    }

    // Cek apakah interval pembaruan sudah tercapai
    uint64_t currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();

    if (currentTime - m_lastReadTime < m_config.updateInterval) {
        // Jika belum waktunya membaca, kembalikan data terakhir
        auto cachedData = std::make_unique<PressureData>(m_lastReading);
        return core::makeOk<std::unique_ptr<SensorData>>(std::move(cachedData));
    }

    // Buat data sensor baru
    auto data = std::make_unique<PressureData>();
    data->timestamp = currentTime;

    // Simpan nilai tekanan sebelumnya sebelum mengganti dengan nilai baru
    m_lastPressureValue = m_lastReading.pressure;

    // Dalam implementasi nyata, kita akan membaca dari hardware
    // Untuk simulasi, hasilkan nilai yang masuk akal

    // Implementasi pembacaan yang berbeda untuk tipe sensor yang berbeda
    switch (m_config.sensorType) {
        case PressureSensorType::BMP280: {
            // Buat random generator untuk simulasi
            std::random_device rd;
            std::mt19937 gen(rd());

            // BMP280 memiliki rentang tekanan 300-1100 hPa
            // Buat simulasi tekanan sekitar tekanan di permukaan laut normal
            std::normal_distribution<float> pressureDist(m_config.seaLevelPressure, 2.0f);
            data->pressure = pressureDist(gen);

            // Simulasikan suhu ruangan dengan sedikit fluktuasi
            std::normal_distribution<float> tempDist(22.0f, 0.5f);
            data->temperature = tempDist(gen);

            // Hitung ketinggian berdasarkan tekanan dan tekanan permukaan laut
            data->altitude = calculateAltitude(data->pressure, m_config.seaLevelPressure);
            break;
        }

        case PressureSensorType::BMP180: {
            // BMP180 memiliki spesifikasi yang sedikit berbeda
            std::random_device rd;
            std::mt19937 gen(rd());

            // BMP180 memiliki akurasi yang lebih rendah dibanding BMP280
            std::normal_distribution<float> pressureDist(m_config.seaLevelPressure, 3.0f);
            data->pressure = pressureDist(gen);

            std::normal_distribution<float> tempDist(22.0f, 1.0f);
            data->temperature = tempDist(gen);

            data->altitude = calculateAltitude(data->pressure, m_config.seaLevelPressure);
            break;
        }

        case PressureSensorType::LPS22HB:
        case PressureSensorType::DPS310:
        case PressureSensorType::MS5611:
        case PressureSensorType::MPL3115A2:
        case PressureSensorType::Generic:
        default: {
            // Sensor lain dengan simulasi umum
            std::random_device rd;
            std::mt19937 gen(rd());

            std::normal_distribution<float> pressureDist(m_config.seaLevelPressure, 2.5f);
            data->pressure = pressureDist(gen);

            std::normal_distribution<float> tempDist(22.0f, 0.8f);
            data->temperature = tempDist(gen);

            data->altitude = calculateAltitude(data->pressure, m_config.seaLevelPressure);
            break;
        }
    }

    // Terapkan kalibrasi
    data->pressure += m_pressureCalibrationOffset;
    data->temperature += m_temperatureCalibrationOffset;

    // Recalculate altitude after calibration
    if (m_pressureCalibrationOffset != 0.0f) {
        data->altitude = calculateAltitude(data->pressure, m_config.seaLevelPressure);
    }

    // Simpan pembacaan terakhir
    m_lastReading = *data;
    m_lastReadTime = currentTime;

    FMUS_LOG_DEBUG("Pressure reading: " + std::to_string(data->pressure) + " hPa, " +
                  "Temperature: " + std::to_string(data->temperature) + "°C, " +
                  "Altitude: " + std::to_string(data->altitude) + " m");

    return core::makeOk<std::unique_ptr<SensorData>>(std::move(data));
}

core::Result<void> PressureSensor::calibrate() {
    if (!m_initialized) {
        return core::Error(core::ErrorCode::SensorCalibrationError,
                         "Pressure sensor not initialized");
    }

    FMUS_LOG_INFO("Calibrating pressure sensor");

    // Untuk simulasi, kita hanya mengatur offset kalibrasi ke nol
    m_pressureCalibrationOffset = 0.0f;
    m_temperatureCalibrationOffset = 0.0f;

    // Dalam implementasi nyata, kita akan:
    // 1. Membaca nilai referensi (misalnya dari sensor kalibrasi)
    // 2. Membaca nilai dari sensor kita
    // 3. Menghitung perbedaan
    // 4. Mengatur offset kalibrasi

    // Lakukan pembacaan untuk memastikan kalibrasi
    auto result = read();
    if (result.isError()) {
        return core::Error(core::ErrorCode::SensorCalibrationError,
                         "Failed to verify calibration: " + result.error().message());
    }

    FMUS_LOG_INFO("Pressure sensor calibrated successfully");
    return {};
}

core::Result<void> PressureSensor::configure(const SensorConfig& config) {
    // Coba konversi ke konfigurasi tekanan
    const PressureConfig* pressureConfig = dynamic_cast<const PressureConfig*>(&config);
    if (!pressureConfig) {
        return core::Error(core::ErrorCode::InvalidArgument,
                         "Invalid configuration type for pressure sensor");
    }

    bool requiresReinitialization = false;

    // Periksa apakah jenis sensor berubah
    if (pressureConfig->sensorType != m_config.sensorType) {
        m_config.sensorType = pressureConfig->sensorType;
        requiresReinitialization = true;
    }

    // Periksa apakah alamat berubah
    if (pressureConfig->deviceAddress != m_config.deviceAddress) {
        m_config.deviceAddress = pressureConfig->deviceAddress;
        requiresReinitialization = true;
    }

    // Periksa apakah sample rate berubah
    if (pressureConfig->sampleRate != m_config.sampleRate) {
        m_config.sampleRate = pressureConfig->sampleRate;
        requiresReinitialization = true;
    }

    // Periksa apakah oversampling berubah
    if (pressureConfig->oversamplingRate != m_config.oversamplingRate) {
        m_config.oversamplingRate = pressureConfig->oversamplingRate;
        requiresReinitialization = true;
    }

    // Perbarui tekanan permukaan laut referensi
    m_config.seaLevelPressure = pressureConfig->seaLevelPressure;

    // Selalu perbarui interval pembaruan
    m_config.updateInterval = pressureConfig->updateInterval;

    // Jika perlu inisialisasi ulang, lakukan
    if (requiresReinitialization && m_initialized) {
        m_initialized = false;
        return init();
    }

    return {};
}

SensorType PressureSensor::getType() const {
    return SensorType::Pressure;
}

std::string PressureSensor::getName() const {
    return "Pressure Sensor (" + pressureSensorTypeToString(m_config.sensorType) + ")";
}

bool PressureSensor::isInitialized() const {
    return m_initialized;
}

PressureSensor& PressureSensor::setSensorType(PressureSensorType sensorType) {
    if (m_config.sensorType != sensorType) {
        m_config.sensorType = sensorType;
        // Jika sudah diinisialisasi, inisialisasi ulang dengan jenis baru
        if (m_initialized) {
            m_initialized = false;
            init();
        }
    }
    return *this;
}

PressureSensor& PressureSensor::setUpdateInterval(uint32_t interval) {
    m_config.updateInterval = interval;
    return *this;
}

PressureSensor& PressureSensor::setSampleRate(PressureSampleRate sampleRate) {
    if (m_config.sampleRate != sampleRate) {
        m_config.sampleRate = sampleRate;
        // Jika sudah diinisialisasi, inisialisasi ulang dengan sample rate baru
        if (m_initialized) {
            m_initialized = false;
            init();
        }
    }
    return *this;
}

PressureSensor& PressureSensor::setOversamplingRate(uint8_t rate) {
    if (m_config.oversamplingRate != rate) {
        m_config.oversamplingRate = rate;
        // Jika sudah diinisialisasi, inisialisasi ulang dengan oversampling rate baru
        if (m_initialized) {
            m_initialized = false;
            init();
        }
    }
    return *this;
}

PressureSensor& PressureSensor::setSeaLevelPressure(float pressure) {
    m_config.seaLevelPressure = pressure;
    return *this;
}

PressureSensorType PressureSensor::getPressureSensorType() const {
    return m_config.sensorType;
}

uint32_t PressureSensor::getUpdateInterval() const {
    return m_config.updateInterval;
}

PressureSampleRate PressureSensor::getSampleRate() const {
    return m_config.sampleRate;
}

uint8_t PressureSensor::getOversamplingRate() const {
    return m_config.oversamplingRate;
}

float PressureSensor::getSeaLevelPressure() const {
    return m_config.seaLevelPressure;
}

float PressureSensor::calculateAltitude(float pressure, float seaLevelPressure) const {
    // Formula internasional barometrik untuk ketinggian
    // h = 44330 * (1 - (p/p0)^(1/5.255))
    if (pressure <= 0.0f || seaLevelPressure <= 0.0f) {
        return 0.0f;
    }

    float ratio = pressure / seaLevelPressure;
    return 44330.0f * (1.0f - std::pow(ratio, 0.190295f));
}

std::string pressureSensorTypeToString(PressureSensorType sensorType) {
    switch (sensorType) {
        case PressureSensorType::BMP280:
            return "BMP280";
        case PressureSensorType::BMP180:
            return "BMP180";
        case PressureSensorType::LPS22HB:
            return "LPS22HB";
        case PressureSensorType::DPS310:
            return "DPS310";
        case PressureSensorType::MS5611:
            return "MS5611";
        case PressureSensorType::MPL3115A2:
            return "MPL3115A2";
        case PressureSensorType::Generic:
            return "Generic";
        default:
            return "Unknown";
    }
}

std::string pressureSampleRateToString(PressureSampleRate sampleRate) {
    switch (sampleRate) {
        case PressureSampleRate::Hz_1:
            return "1 Hz";
        case PressureSampleRate::Hz_10:
            return "10 Hz";
        case PressureSampleRate::Hz_25:
            return "25 Hz";
        case PressureSampleRate::Hz_50:
            return "50 Hz";
        case PressureSampleRate::Hz_75:
            return "75 Hz";
        case PressureSampleRate::Hz_100:
            return "100 Hz";
        default:
            return "Unknown";
    }
}

} // namespace sensors
} // namespace fmus
