#include <fmus/fmus.h>
#include <iostream>
#include <thread>
#include <chrono>

using namespace fmus;
using namespace fmus::sensors;

// Simple helper to check results
template<typename T>
void checkResult(const core::Result<T>& result) {
    if (result.isError()) {
        std::cerr << "Error: " << result.error().toString() << std::endl;
        exit(1);
    }
}

int main() {
    // Initialize the fmus-embed library
    if (!fmus::init()) {
        std::cerr << "Failed to initialize fmus-embed library" << std::endl;
        return 1;
    }

    std::cout << "fmus-embed library version: " << core::getVersionString() << std::endl;

    // Create a temperature sensor
    auto tempSensor = createSensor(SensorType::Temperature);
    if (!tempSensor) {
        std::cerr << "Failed to create temperature sensor" << std::endl;
        return 1;
    }

    std::cout << "Created sensor: " << tempSensor->getName() << std::endl;

    // Initialize the sensor
    checkResult(tempSensor->init());
    std::cout << "Sensor initialized" << std::endl;

    // Calibrate the sensor
    checkResult(tempSensor->calibrate());
    std::cout << "Sensor calibrated" << std::endl;

    // Read data from the sensor in a loop
    std::cout << "Reading temperature data (press Ctrl+C to exit):" << std::endl;
    for (int i = 0; i < 10; ++i) {
        // Read data
        auto result = tempSensor->read();
        checkResult(result);

        // Cast to the appropriate type
        auto* tempData = dynamic_cast<TemperatureData*>(result.value().get());
        if (tempData) {
            std::cout << "Temperature: " << tempData->temperature << " °C" << std::endl;
            std::cout << "Humidity: " << tempData->humidity << " %" << std::endl;
            std::cout << "Timestamp: " << tempData->timestamp << " ms" << std::endl;
        } else {
            std::cerr << "Failed to cast sensor data to TemperatureData" << std::endl;
        }

        // Wait a second before next reading
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Shutdown the library
    fmus::shutdown();
    std::cout << "Library shutdown" << std::endl;

    return 0;
}
