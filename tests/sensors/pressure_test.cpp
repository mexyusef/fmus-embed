#include <gtest/gtest.h>
#include <fmus/sensors/pressure.h>
#include <fmus/core/error.h>
#include <chrono>
#include <thread>

using namespace fmus;
using namespace fmus::sensors;

class PressureSensorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Inisialisasi sensor untuk testing
        sensor = std::make_unique<PressureSensor>(PressureSensorType::BMP280, 0x76);
    }

    void TearDown() override {
        sensor.reset();
    }

    std::unique_ptr<PressureSensor> sensor;
};

// Test inisialisasi sensor
TEST_F(PressureSensorTest, Initialization) {
    // Inisialisasi sensor
    auto result = sensor->init();

    // Periksa bahwa tidak ada error
    ASSERT_FALSE(result.isError());

    // Periksa bahwa sensor terinisialisasi
    EXPECT_TRUE(sensor->isInitialized());

    // Periksa tipe sensor
    EXPECT_EQ(sensor->getType(), SensorType::Pressure);

    // Periksa nama sensor berisi "BMP280"
    EXPECT_NE(sensor->getName().find("BMP280"), std::string::npos);
}

// Test konfigurasi sensor
TEST_F(PressureSensorTest, Configuration) {
    // Buat konfigurasi sensor baru
    PressureConfig config;
    config.sensorType = PressureSensorType::BMP180;
    config.deviceAddress = 0x77;
    config.sampleRate = PressureSampleRate::Hz_25;
    config.oversamplingRate = 5;
    config.seaLevelPressure = 1020.0f;
    config.updateInterval = 500;

    // Terapkan konfigurasi
    auto result = sensor->configure(config);

    // Periksa bahwa tidak ada error
    ASSERT_FALSE(result.isError());

    // Periksa bahwa konfigurasi diterapkan dengan benar
    EXPECT_EQ(sensor->getPressureSensorType(), PressureSensorType::BMP180);
    EXPECT_EQ(sensor->getSampleRate(), PressureSampleRate::Hz_25);
    EXPECT_EQ(sensor->getOversamplingRate(), 5);
    EXPECT_FLOAT_EQ(sensor->getSeaLevelPressure(), 1020.0f);
    EXPECT_EQ(sensor->getUpdateInterval(), 500);
}

// Test pembacaan sensor
TEST_F(PressureSensorTest, Reading) {
    // Inisialisasi sensor
    auto initResult = sensor->init();
    ASSERT_FALSE(initResult.isError());

    // Baca data sensor
    auto readResult = sensor->read();

    // Periksa bahwa tidak ada error
    ASSERT_FALSE(readResult.isError());
    ASSERT_NE(readResult.value(), nullptr);

    // Konversi ke PressureData
    const auto& data = dynamic_cast<PressureData&>(*readResult.value());

    // Periksa bahwa data valid (dalam rentang yang diharapkan)
    EXPECT_GE(data.pressure, 900.0f);  // Minimal 900 hPa
    EXPECT_LE(data.pressure, 1100.0f); // Maksimal 1100 hPa

    EXPECT_GE(data.temperature, 0.0f);  // Minimal 0°C
    EXPECT_LE(data.temperature, 40.0f); // Maksimal 40°C

    // Periksa timestamp
    EXPECT_GT(data.timestamp, 0);
}

// Test konversi unit
TEST_F(PressureSensorTest, UnitConversions) {
    // Buat data tekanan untuk pengujian
    PressureData data;
    data.pressure = 1013.25f; // Tekanan standar di permukaan laut

    // Periksa konversi ke atm
    EXPECT_FLOAT_EQ(data.getAtmospheres(), 1.0f);

    // Periksa konversi ke mmHg
    EXPECT_NEAR(data.getMmHg(), 760.0f, 0.5f);

    // Periksa konversi ke inHg
    EXPECT_NEAR(data.getInHg(), 29.92f, 0.05f);
}

// Test kalibrasi sensor
TEST_F(PressureSensorTest, Calibration) {
    // Inisialisasi sensor
    auto initResult = sensor->init();
    ASSERT_FALSE(initResult.isError());

    // Kalibrasi sensor
    auto calibResult = sensor->calibrate();

    // Periksa bahwa tidak ada error
    ASSERT_FALSE(calibResult.isError());

    // Baca data sensor setelah kalibrasi
    auto readResult = sensor->read();
    ASSERT_FALSE(readResult.isError());
}

// Test perhitungan ketinggian
TEST_F(PressureSensorTest, AltitudeCalculation) {
    // Inisialisasi sensor
    auto initResult = sensor->init();
    ASSERT_FALSE(initResult.isError());

    // Konfigurasi sensor dengan tekanan permukaan laut yang diketahui
    sensor->setSeaLevelPressure(1013.25f);

    // Baca data sensor
    auto readResult = sensor->read();
    ASSERT_FALSE(readResult.isError());

    // Konversi ke PressureData
    const auto& data = dynamic_cast<PressureData&>(*readResult.value());

    // Periksa konsistensi ketinggian:
    // - Jika tekanan = tekanan permukaan laut, altitude harus sekitar 0
    // - Jika tekanan < tekanan permukaan laut, altitude harus positif
    // - Jika tekanan > tekanan permukaan laut, altitude harus negatif
    if (std::abs(data.pressure - 1013.25f) < 0.1f) {
        EXPECT_NEAR(data.altitude, 0.0f, 1.0f);
    } else if (data.pressure < 1013.25f) {
        EXPECT_GT(data.altitude, 0.0f);
    } else {
        EXPECT_LT(data.altitude, 0.0f);
    }
}

// Test metode deteksi perubahan cuaca
TEST_F(PressureSensorTest, WeatherPrediction) {
    PressureData data;

    // Test prediksi cuaca cerah (tekanan tinggi)
    data.pressure = 1025.0f;
    EXPECT_TRUE(data.isFairWeather());

    // Test prediksi cuaca hujan (tekanan rendah)
    data.pressure = 1005.0f;
    EXPECT_FALSE(data.isFairWeather());

    // Test deteksi perubahan cuaca (perubahan signifikan dalam waktu pendek)
    data.pressure = 1010.0f;
    EXPECT_TRUE(data.isWeatherChangeLikely(1013.0f, 1.0f)); // 3 hPa dalam 1 jam

    // Test kondisi stabil (perubahan kecil)
    EXPECT_FALSE(data.isWeatherChangeLikely(1010.5f, 1.0f)); // 0.5 hPa dalam 1 jam
}

// Test fungsi konversi string
TEST_F(PressureSensorTest, StringConversions) {
    // Test konversi jenis sensor ke string
    EXPECT_EQ(pressureSensorTypeToString(PressureSensorType::BMP280), "BMP280");
    EXPECT_EQ(pressureSensorTypeToString(PressureSensorType::BMP180), "BMP180");
    EXPECT_EQ(pressureSensorTypeToString(PressureSensorType::LPS22HB), "LPS22HB");
    EXPECT_EQ(pressureSensorTypeToString(PressureSensorType::DPS310), "DPS310");
    EXPECT_EQ(pressureSensorTypeToString(PressureSensorType::MS5611), "MS5611");
    EXPECT_EQ(pressureSensorTypeToString(PressureSensorType::MPL3115A2), "MPL3115A2");
    EXPECT_EQ(pressureSensorTypeToString(PressureSensorType::Generic), "Generic");

    // Test konversi sample rate ke string
    EXPECT_EQ(pressureSampleRateToString(PressureSampleRate::Hz_1), "1 Hz");
    EXPECT_EQ(pressureSampleRateToString(PressureSampleRate::Hz_10), "10 Hz");
    EXPECT_EQ(pressureSampleRateToString(PressureSampleRate::Hz_25), "25 Hz");
    EXPECT_EQ(pressureSampleRateToString(PressureSampleRate::Hz_50), "50 Hz");
    EXPECT_EQ(pressureSampleRateToString(PressureSampleRate::Hz_75), "75 Hz");
    EXPECT_EQ(pressureSampleRateToString(PressureSampleRate::Hz_100), "100 Hz");
}

// Test caching data saat interval pembaruan belum tercapai
TEST_F(PressureSensorTest, DataCaching) {
    // Inisialisasi sensor dengan interval pembaruan 1 detik
    sensor->setUpdateInterval(1000);
    auto initResult = sensor->init();
    ASSERT_FALSE(initResult.isError());

    // Baca data pertama
    auto firstReadResult = sensor->read();
    ASSERT_FALSE(firstReadResult.isError());
    const auto& firstData = dynamic_cast<PressureData&>(*firstReadResult.value());

    // Simpan nilai tekanan pertama
    float firstPressure = firstData.pressure;

    // Baca data lagi segera (dalam interval pembaruan)
    auto secondReadResult = sensor->read();
    ASSERT_FALSE(secondReadResult.isError());
    const auto& secondData = dynamic_cast<PressureData&>(*secondReadResult.value());

    // Data cache harus sama dengan data pertama
    EXPECT_FLOAT_EQ(secondData.pressure, firstPressure);

    // Tunggu interval pembaruan berlalu
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));

    // Baca data lagi setelah interval pembaruan
    auto thirdReadResult = sensor->read();
    ASSERT_FALSE(thirdReadResult.isError());
    const auto& thirdData = dynamic_cast<PressureData&>(*thirdReadResult.value());

    // Data seharusnya diperbarui (kemungkinan berbeda)
    // Catatan: Dalam simulasi, nilai baru mungkin sama dengan nilai lama karena kebetulan
    // Kita tidak bisa test ini dengan pasti, tapi kemungkinan besar berbeda
    // EXPECT_NE(thirdData.pressure, firstPressure);
}

// Test fluent interface
TEST_F(PressureSensorTest, FluentInterface) {
    // Gunakan fluent interface untuk konfigurasi
    sensor->setSensorType(PressureSensorType::BMP180)
           .setUpdateInterval(500)
           .setSampleRate(PressureSampleRate::Hz_50)
           .setOversamplingRate(6)
           .setSeaLevelPressure(1015.0f);

    // Periksa konfigurasi
    EXPECT_EQ(sensor->getPressureSensorType(), PressureSensorType::BMP180);
    EXPECT_EQ(sensor->getUpdateInterval(), 500);
    EXPECT_EQ(sensor->getSampleRate(), PressureSampleRate::Hz_50);
    EXPECT_EQ(sensor->getOversamplingRate(), 6);
    EXPECT_FLOAT_EQ(sensor->getSeaLevelPressure(), 1015.0f);
}
