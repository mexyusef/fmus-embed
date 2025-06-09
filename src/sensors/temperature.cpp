#include <fmus/sensors/temperature.h>
#include <fmus/core/error.h>
#include <fmus/core/logging.h>
#include <chrono>
#include <cmath>
#include <thread>

namespace fmus {
namespace sensors {

// Implementasi metode TemperatureData
float TemperatureData::getFahrenheit() const {
    // Konversi dari Celsius ke Fahrenheit
    return (temperature * 9.0f / 5.0f) + 32.0f;
}

float TemperatureData::getKelvin() const {
    // Konversi dari Celsius ke Kelvin
    return temperature + 273.15f;
}

bool TemperatureData::isHumidityComfortable(float minHumidity, float maxHumidity) const {
    // Periksa apakah kelembaban dalam rentang nyaman
    return (humidity >= minHumidity && humidity <= maxHumidity);
}

bool TemperatureData::isTemperatureComfortable(float minTemp, float maxTemp) const {
    // Periksa apakah suhu dalam rentang nyaman
    return (temperature >= minTemp && temperature <= maxTemp);
}

// Implementasi konstruktor TemperatureConfig dengan nilai default
TemperatureConfig::TemperatureConfig()
    : sensorType(TemperatureSensorType::Generic),
      pin(0),
      deviceAddress(0),
      updateInterval(1000) // Default 1 detik
{
}

// Implementasi konstruktor TemperatureSensor
TemperatureSensor::TemperatureSensor(TemperatureSensorType sensorType, uint8_t value, bool isI2C)
    : m_initialized(false),
      m_tempCalibrationOffset(0.0f),
      m_humidityCalibrationOffset(0.0f),
      m_lastReadTime(0)
{
    m_config.sensorType = sensorType;

    if (isI2C) {
        m_config.pin = 0;
        m_config.deviceAddress = value;
    } else {
        m_config.pin = value;
        m_config.deviceAddress = 0;
    }

    m_config.updateInterval = 1000; // Default 1 detik

    // Inisialisasi data pembacaan terakhir
    m_lastReading.temperature = 0.0f;
    m_lastReading.humidity = 0.0f;
    m_lastReading.pressure = 0.0f;
    m_lastReading.timestamp = 0;
}

TemperatureSensor::~TemperatureSensor() {
    // Pembersihan khusus tidak diperlukan
}

core::Result<void> TemperatureSensor::init() {
    FMUS_LOG_INFO("Initializing temperature sensor: " +
                 temperatureSensorTypeToString(m_config.sensorType));

    // Implementasi inisialisasi yang berbeda untuk tipe sensor yang berbeda
    switch (m_config.sensorType) {
        case TemperatureSensorType::DHT11:
        case TemperatureSensorType::DHT22:
            // Untuk sensor DHT, pastikan pin dikonfigurasi dengan benar
            if (m_config.pin == 0) {
                return core::Error(core::ErrorCode::InvalidArgument,
                                "Pin number must be specified for DHT sensors");
            }
            // Inisialisasi pin dan konfigurasi untuk DHT
            // ...
            break;

        case TemperatureSensorType::DS18B20:
            // Untuk sensor DS18B20, pastikan pin dikonfigurasi dengan benar untuk 1-Wire
            if (m_config.pin == 0) {
                return core::Error(core::ErrorCode::InvalidArgument,
                                "Pin number must be specified for DS18B20 sensors");
            }
            // Inisialisasi pin dan protokol 1-Wire
            // ...
            break;

        case TemperatureSensorType::BME280:
        case TemperatureSensorType::SHT31:
            // Untuk sensor I2C, pastikan alamat dikonfigurasi dengan benar
            if (m_config.deviceAddress == 0) {
                return core::Error(core::ErrorCode::InvalidArgument,
                                "Device address must be specified for I2C sensors");
            }
            // Inisialisasi koneksi I2C
            // ...
            break;

        case TemperatureSensorType::LM35:
            // Untuk sensor analog, pastikan pin ADC dikonfigurasi dengan benar
            if (m_config.pin == 0) {
                return core::Error(core::ErrorCode::InvalidArgument,
                                "Pin number must be specified for analog sensors");
            }
            // Inisialisasi pin ADC
            // ...
            break;

        case TemperatureSensorType::Generic:
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

    FMUS_LOG_INFO("Temperature sensor initialized successfully");
    return {};
}

core::Result<std::unique_ptr<SensorData>> TemperatureSensor::read() {
    if (!m_initialized) {
        return core::Error(core::ErrorCode::SensorReadError,
                         "Temperature sensor not initialized");
    }

    // Cek apakah interval pembaruan sudah tercapai
    uint64_t currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();

    if (currentTime - m_lastReadTime < m_config.updateInterval) {
        // Jika belum waktunya membaca, kembalikan data terakhir
        auto cachedData = std::make_unique<TemperatureData>(m_lastReading);
        return core::makeOk<std::unique_ptr<SensorData>>(std::move(cachedData));
    }

    // Buat data sensor baru
    auto data = std::make_unique<TemperatureData>();
    data->timestamp = currentTime;

    // Dalam implementasi nyata, kita akan membaca dari hardware
    // Untuk simulasi, hasilkan nilai yang masuk akal

    // Buat simulasi pembacaan berdasarkan jenis sensor
    switch (m_config.sensorType) {
        case TemperatureSensorType::DHT11:
            // DHT11 memiliki presisi lebih rendah
            data->temperature = 20.0f + (static_cast<float>(rand() % 100) / 10.0f);
            data->humidity = 40.0f + (static_cast<float>(rand() % 200) / 10.0f);
            data->pressure = 0.0f; // DHT11 tidak mengukur tekanan
            break;

        case TemperatureSensorType::DHT22:
            // DHT22 memiliki presisi lebih tinggi
            data->temperature = 20.0f + (static_cast<float>(rand() % 100) / 100.0f);
            data->humidity = 40.0f + (static_cast<float>(rand() % 200) / 10.0f);
            data->pressure = 0.0f; // DHT22 tidak mengukur tekanan
            break;

        case TemperatureSensorType::BME280:
            // BME280 mengukur suhu, kelembaban, dan tekanan
            data->temperature = 20.0f + (static_cast<float>(rand() % 100) / 100.0f);
            data->humidity = 40.0f + (static_cast<float>(rand() % 200) / 10.0f);
            data->pressure = 1013.25f + (static_cast<float>(rand() % 200) / 10.0f - 10.0f);
            break;

        case TemperatureSensorType::DS18B20:
        case TemperatureSensorType::LM35:
        case TemperatureSensorType::SHT31:
        case TemperatureSensorType::Generic:
        default:
            // Sensor lain hanya mengukur suhu
            data->temperature = 20.0f + (static_cast<float>(rand() % 100) / 10.0f);
            data->humidity = 0.0f;
            data->pressure = 0.0f;
            break;
    }

    // Terapkan kalibrasi
    data->temperature += m_tempCalibrationOffset;
    data->humidity += m_humidityCalibrationOffset;

    // Pemeriksaan batas
    if (data->humidity < 0.0f) data->humidity = 0.0f;
    if (data->humidity > 100.0f) data->humidity = 100.0f;

    // Simpan pembacaan terakhir
    m_lastReading = *data;
    m_lastReadTime = currentTime;

    FMUS_LOG_DEBUG("Temperature reading: " + std::to_string(data->temperature) + "°C, " +
                  "Humidity: " + std::to_string(data->humidity) + "%");

    return core::makeOk<std::unique_ptr<SensorData>>(std::move(data));
}

core::Result<void> TemperatureSensor::calibrate() {
    if (!m_initialized) {
        return core::Error(core::ErrorCode::SensorCalibrationError,
                         "Temperature sensor not initialized");
    }

    FMUS_LOG_INFO("Calibrating temperature sensor");

    // Untuk simulasi, kita hanya mengatur offset kalibrasi ke nol
    m_tempCalibrationOffset = 0.0f;
    m_humidityCalibrationOffset = 0.0f;

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

    FMUS_LOG_INFO("Temperature sensor calibrated successfully");
    return {};
}

core::Result<void> TemperatureSensor::configure(const SensorConfig& config) {
    // Coba konversi ke konfigurasi suhu
    const TemperatureConfig* tempConfig = dynamic_cast<const TemperatureConfig*>(&config);
    if (!tempConfig) {
        return core::Error(core::ErrorCode::InvalidArgument,
                         "Invalid configuration type for temperature sensor");
    }

    bool requiresReinitialization = false;

    // Periksa apakah jenis sensor berubah
    if (tempConfig->sensorType != m_config.sensorType) {
        m_config.sensorType = tempConfig->sensorType;
        requiresReinitialization = true;
    }

    // Periksa apakah pin atau alamat berubah
    if (tempConfig->pin != m_config.pin) {
        m_config.pin = tempConfig->pin;
        requiresReinitialization = true;
    }

    if (tempConfig->deviceAddress != m_config.deviceAddress) {
        m_config.deviceAddress = tempConfig->deviceAddress;
        requiresReinitialization = true;
    }

    // Selalu perbarui interval pembaruan
    m_config.updateInterval = tempConfig->updateInterval;

    // Jika perlu inisialisasi ulang, lakukan
    if (requiresReinitialization && m_initialized) {
        m_initialized = false;
        return init();
    }

    return {};
}

SensorType TemperatureSensor::getType() const {
    return SensorType::Temperature;
}

std::string TemperatureSensor::getName() const {
    return "Temperature Sensor (" + temperatureSensorTypeToString(m_config.sensorType) + ")";
}

bool TemperatureSensor::isInitialized() const {
    return m_initialized;
}

TemperatureSensor& TemperatureSensor::setSensorType(TemperatureSensorType sensorType) {
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

TemperatureSensor& TemperatureSensor::setUpdateInterval(uint32_t interval) {
    m_config.updateInterval = interval;
    return *this;
}

TemperatureSensorType TemperatureSensor::getTemperatureSensorType() const {
    return m_config.sensorType;
}

uint32_t TemperatureSensor::getUpdateInterval() const {
    return m_config.updateInterval;
}

std::string temperatureSensorTypeToString(TemperatureSensorType sensorType) {
    switch (sensorType) {
        case TemperatureSensorType::DHT11:
            return "DHT11";
        case TemperatureSensorType::DHT22:
            return "DHT22";
        case TemperatureSensorType::DS18B20:
            return "DS18B20";
        case TemperatureSensorType::LM35:
            return "LM35";
        case TemperatureSensorType::BME280:
            return "BME280";
        case TemperatureSensorType::SHT31:
            return "SHT31";
        case TemperatureSensorType::Generic:
            return "Generic";
        default:
            return "Unknown";
    }
}

} // namespace sensors
} // namespace fmus
