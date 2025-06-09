#include <fmus/fmus.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <vector>
#include <string>
#include <csignal>

using namespace fmus;
using namespace fmus::sensors;

// Flag untuk menandakan program berjalan
volatile bool g_running = true;

// Handler untuk menangkap sinyal Ctrl+C
void signalHandler(int signal) {
    g_running = false;
    std::cout << "Interrupt received, stopping temperature monitoring..." << std::endl;
}

// Kelas pembantu untuk memformat output
class TemperatureDisplay {
public:
    static void printHeader() {
        std::cout << "╔════════════════════════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║            Advanced Temperature Monitoring Example                  ║" << std::endl;
        std::cout << "╚════════════════════════════════════════════════════════════════════╝" << std::endl;
    }

    static void printSensorInfo(const TemperatureSensor& sensor) {
        std::cout << "┌────────────────────────────────────────────────────────────────────┐" << std::endl;
        std::cout << "│ Sensor: " << std::left << std::setw(59) << sensor.getName() << "│" << std::endl;
        std::cout << "│ Type: " << std::left << std::setw(62) << temperatureSensorTypeToString(sensor.getTemperatureSensorType()) << "│" << std::endl;
        std::cout << "│ Update Interval: " << std::left << std::setw(52) << (sensor.getUpdateInterval() / 1000.0) << " seconds │" << std::endl;
        std::cout << "└────────────────────────────────────────────────────────────────────┘" << std::endl;
    }

    static void printReadingHeader() {
        std::cout << "┌────────────────────────┬────────────────┬────────────────┬────────────────┐" << std::endl;
        std::cout << "│ Timestamp              │ Temperature    │ Humidity       │ Pressure       │" << std::endl;
        std::cout << "├────────────────────────┼────────────────┼────────────────┼────────────────┤" << std::endl;
    }

    static void printReading(const TemperatureData& data) {
        auto time = std::time(nullptr);
        auto tm = std::localtime(&time);

        std::cout << "│ " << std::put_time(tm, "%Y-%m-%d %H:%M:%S") << " │ ";

        // Temperature with multiple units
        std::cout << std::fixed << std::setprecision(2) << std::setw(6) << data.temperature << "°C / ";
        std::cout << std::fixed << std::setprecision(2) << std::setw(6) << data.getFahrenheit() << "°F │ ";

        // Humidity
        if (data.humidity > 0.0f) {
            std::cout << std::fixed << std::setprecision(1) << std::setw(6) << data.humidity << "% ";

            // Add comfort indicator
            if (data.isHumidityComfortable()) {
                std::cout << "(Optimal) │ ";
            } else if (data.humidity < 30.0f) {
                std::cout << "(Too dry) │ ";
            } else {
                std::cout << "(Too humid)│ ";
            }
        } else {
            std::cout << "     N/A       │ ";
        }

        // Pressure
        if (data.pressure > 0.0f) {
            std::cout << std::fixed << std::setprecision(2) << std::setw(6) << data.pressure << " hPa    │";
        } else {
            std::cout << "     N/A       │";
        }

        std::cout << std::endl;
    }

    static void printReadingFooter() {
        std::cout << "└────────────────────────┴────────────────┴────────────────┴────────────────┘" << std::endl;
    }

    static void printComfortStatus(const TemperatureData& data) {
        std::cout << "┌────────────────────────────────────────────────────────────────────┐" << std::endl;
        std::cout << "│ Comfort Analysis:                                                   │" << std::endl;

        bool tempComfortable = data.isTemperatureComfortable();
        bool humidityComfortable = data.isHumidityComfortable();

        std::cout << "│ Temperature: " << (tempComfortable ? "Comfortable" : "Uncomfortable");
        std::cout << std::string(44 - (tempComfortable ? 11 : 13), ' ') << "│" << std::endl;

        if (data.humidity > 0.0f) {
            std::cout << "│ Humidity: " << (humidityComfortable ? "Comfortable" : "Uncomfortable");
            std::cout << std::string(47 - (humidityComfortable ? 11 : 13), ' ') << "│" << std::endl;
        }

        // Overall comfort status
        std::cout << "│ Overall: ";
        if (tempComfortable && (data.humidity <= 0.0f || humidityComfortable)) {
            std::cout << "Optimal environmental conditions";
            std::cout << std::string(29, ' ') << "│" << std::endl;
        } else if (!tempComfortable && (data.humidity <= 0.0f || !humidityComfortable)) {
            std::cout << "Poor environmental conditions";
            std::cout << std::string(31, ' ') << "│" << std::endl;
        } else {
            std::cout << "Partially comfortable conditions";
            std::cout << std::string(27, ' ') << "│" << std::endl;
        }

        std::cout << "└────────────────────────────────────────────────────────────────────┘" << std::endl;
    }
};

int main() {
    // Register signal handler for Ctrl+C
    std::signal(SIGINT, signalHandler);

    // Display header
    TemperatureDisplay::printHeader();

    std::cout << "Initializing fmus-embed library..." << std::endl;

    // Initialize the fmus-embed library
    if (!fmus::init()) {
        std::cerr << "Failed to initialize fmus-embed library" << std::endl;
        return 1;
    }

    std::cout << "fmus-embed library version: " << core::getVersionString() << std::endl;

    // Create multiple temperature sensors of different types
    std::vector<std::unique_ptr<TemperatureSensor>> sensors;

    try {
        // Create a DHT22 sensor
        auto dht22Sensor = std::make_unique<TemperatureSensor>(TemperatureSensorType::DHT22, 4);
        dht22Sensor->setUpdateInterval(2000); // Update every 2 seconds
        auto result = dht22Sensor->init();
        if (result.isOk()) {
            sensors.push_back(std::move(dht22Sensor));
        } else {
            std::cerr << "Failed to initialize DHT22 sensor: " << result.error().toString() << std::endl;
        }

        // Create a BME280 sensor
        auto bme280Sensor = std::make_unique<TemperatureSensor>(TemperatureSensorType::BME280, 0x76);
        bme280Sensor->setUpdateInterval(1000); // Update every 1 second
        result = bme280Sensor->init();
        if (result.isOk()) {
            sensors.push_back(std::move(bme280Sensor));
        } else {
            std::cerr << "Failed to initialize BME280 sensor: " << result.error().toString() << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error creating sensors: " << e.what() << std::endl;
        fmus::shutdown();
        return 1;
    }

    if (sensors.empty()) {
        std::cerr << "No sensors were successfully initialized" << std::endl;
        fmus::shutdown();
        return 1;
    }

    std::cout << "Successfully initialized " << sensors.size() << " temperature sensors." << std::endl;
    std::cout << "Press Ctrl+C to exit." << std::endl;

    // Main loop for reading sensors
    while (g_running) {
        for (auto& sensor : sensors) {
            // Display sensor info
            TemperatureDisplay::printSensorInfo(*sensor);

            // Read the temperature data
            auto result = sensor->readTyped();
            if (result.isOk()) {
                const TemperatureData& data = result.value();

                // Display the reading in a table
                TemperatureDisplay::printReadingHeader();
                TemperatureDisplay::printReading(data);
                TemperatureDisplay::printReadingFooter();

                // Display comfort analysis
                TemperatureDisplay::printComfortStatus(data);
            }
            else {
                std::cerr << "Error reading temperature: " << result.error().toString() << std::endl;
            }

            std::cout << std::endl;
        }

        // Wait before next reading
        for (int i = 0; i < 50 && g_running; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Clear the screen for next update
        if (g_running) {
            // Use ANSI escape sequence to clear screen
            std::cout << "\033[2J\033[1;1H";

            // Print the header again
            TemperatureDisplay::printHeader();
        }
    }

    // Shutdown the library
    fmus::shutdown();
    std::cout << "Library shutdown" << std::endl;

    return 0;
}
