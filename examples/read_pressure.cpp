#include <fmus/fmus.h>
#include <fmus/sensors/pressure.h>
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <csignal>

// Flag untuk menandakan program berjalan
volatile sig_atomic_t running = true;

// Handler untuk sinyal interupsi (Ctrl+C)
void signalHandler(int signal) {
    running = false;
    std::cout << "\nShutting down..." << std::endl;
}

int main(int argc, char* argv[]) {
    // Register handler untuk Ctrl+C
    std::signal(SIGINT, signalHandler);

    // Inisialisasi library FMUS
    auto result = fmus::init();
    if (result.isError()) {
        std::cerr << "Failed to initialize FMUS: " << result.error().message() << std::endl;
        return 1;
    }

    std::cout << "FMUS Pressure Sensor Example" << std::endl;
    std::cout << "----------------------------" << std::endl;

    // Buat sensor tekanan (default BMP280)
    fmus::sensors::PressureSensor pressureSensor(fmus::sensors::PressureSensorType::BMP280, 0x76);

    // Konfigurasi sensor
    pressureSensor.setUpdateInterval(1000)        // Update setiap 1 detik
                  .setSampleRate(fmus::sensors::PressureSampleRate::Hz_10)
                  .setOversamplingRate(4)         // Oversampling x8
                  .setSeaLevelPressure(1013.25f); // Standar tekanan permukaan laut

    // Inisialisasi sensor
    auto initResult = pressureSensor.init();
    if (initResult.isError()) {
        std::cerr << "Failed to initialize pressure sensor: " << initResult.error().message() << std::endl;
        fmus::shutdown();
        return 1;
    }

    std::cout << "Sensor initialized: " << pressureSensor.getName() << std::endl;
    std::cout << "Press Ctrl+C to exit" << std::endl << std::endl;

    // Kalibrasi sensor (opsional)
    pressureSensor.calibrate();

    // Variabel untuk tracking perubahan tekanan
    float previousPressure = 0.0f;
    const float timeIntervalHours = 1.0f / 60.0f; // 1 menit dalam jam

    // Pembacaan sensor dalam loop
    while (running) {
        auto readResult = pressureSensor.read();
        if (readResult.isError()) {
            std::cerr << "Error reading pressure sensor: " << readResult.error().message() << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        // Convert SensorData ke PressureData
        const auto& data = static_cast<fmus::sensors::PressureData&>(*readResult.value());

        // Format output dengan 2 desimal
        std::cout << std::fixed << std::setprecision(2);

        // Tampilkan data tekanan dalam berbagai unit
        std::cout << "Pressure: " << data.pressure << " hPa ("
                  << data.getAtmospheres() << " atm, "
                  << data.getMmHg() << " mmHg, "
                  << data.getInHg() << " inHg)" << std::endl;

        // Tampilkan data suhu dan ketinggian
        std::cout << "Temperature: " << data.temperature << " °C" << std::endl;
        std::cout << "Altitude: " << data.altitude << " meters" << std::endl;

        // Prediksi cuaca berdasarkan tekanan
        if (data.isFairWeather()) {
            std::cout << "Weather prediction: Fair weather (high pressure)" << std::endl;
        } else {
            std::cout << "Weather prediction: Possible precipitation (low pressure)" << std::endl;
        }

        // Periksa tren perubahan tekanan
        if (previousPressure > 0.0f) {
            if (data.isWeatherChangeLikely(previousPressure, timeIntervalHours)) {
                std::cout << "Weather trend: Significant pressure change detected!" << std::endl;

                if (data.pressure > previousPressure) {
                    std::cout << "Pressure is rising: Weather may be improving" << std::endl;
                } else {
                    std::cout << "Pressure is falling: Weather may be deteriorating" << std::endl;
                }
            } else {
                std::cout << "Weather trend: Stable" << std::endl;
            }
        }

        // Simpan tekanan saat ini untuk perbandingan berikutnya
        previousPressure = data.pressure;

        std::cout << "----------------------------" << std::endl;

        // Tunggu 2 detik sebelum pembacaan berikutnya
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    // Shutdown library FMUS
    fmus::shutdown();

    return 0;
}
