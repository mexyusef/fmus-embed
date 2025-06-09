#include <fmus/fmus.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <array>

// Definisikan pin untuk lampu lalu lintas
const unsigned int RED_PIN = 13;
const unsigned int YELLOW_PIN = 12;
const unsigned int GREEN_PIN = 11;

// Definisikan pin tombol pejalan kaki
const unsigned int PEDESTRIAN_BUTTON_PIN = 10;

// Definisikan durasi lampu
const auto RED_DURATION = std::chrono::seconds(10);
const auto YELLOW_DURATION = std::chrono::seconds(3);
const auto GREEN_DURATION = std::chrono::seconds(7);
const auto PEDESTRIAN_CROSSING_DURATION = std::chrono::seconds(15);

// Flag untuk kondisi berjalan
volatile bool g_running = true;

// Handler untuk menangkap sinyal
void signalHandler(int signum) {
    g_running = false;
    std::cout << "Interrupt received, stopping traffic light..." << std::endl;
}

// State mesin untuk lampu lalu lintas
enum class TrafficLightState {
    Red,
    Yellow,
    Green,
    PedestrianCrossing
};

// Fungsi untuk mematikan semua lampu
void turnOffAllLights(const std::array<std::unique_ptr<fmus::gpio::GPIO>, 3>& lights) {
    for (const auto& light : lights) {
        auto result = light->write(false);
        if (!result.isOk()) {
            std::cerr << "Error turning off light: " << result.error().toString() << std::endl;
        }
    }
}

int main() {
    // Set up signal handler
    signal(SIGINT, signalHandler);

    std::cout << "Starting traffic light controller example..." << std::endl;

    // Initialize fmus library
    if (!fmus::init()) {
        std::cerr << "Failed to initialize fmus library" << std::endl;
        return 1;
    }

    // Create GPIO pins for traffic lights
    std::array<std::unique_ptr<fmus::gpio::GPIO>, 3> trafficLights;
    trafficLights[0] = std::make_unique<fmus::gpio::GPIO>(RED_PIN);      // Red
    trafficLights[1] = std::make_unique<fmus::gpio::GPIO>(YELLOW_PIN);   // Yellow
    trafficLights[2] = std::make_unique<fmus::gpio::GPIO>(GREEN_PIN);    // Green

    // Create GPIO pin for pedestrian button
    std::unique_ptr<fmus::gpio::GPIO> pedestrianButton =
        std::make_unique<fmus::gpio::GPIO>(PEDESTRIAN_BUTTON_PIN);

    // Initialize traffic light pins as output
    for (auto& light : trafficLights) {
        auto result = light->init(fmus::gpio::GPIODirection::Output);
        if (!result.isOk()) {
            std::cerr << "Error initializing traffic light pin: " << result.error().toString() << std::endl;
            fmus::shutdown();
            return 1;
        }
    }

    // Initialize pedestrian button pin as input with pull-up resistor
    auto result = pedestrianButton->init(fmus::gpio::GPIODirection::Input);
    if (!result.isOk()) {
        std::cerr << "Error initializing pedestrian button pin: " << result.error().toString() << std::endl;
        fmus::shutdown();
        return 1;
    }

    // Set pull-up resistor for button
    result = pedestrianButton->setPull(fmus::gpio::GPIOPull::Up);
    if (!result.isOk()) {
        std::cerr << "Error setting pull-up resistor: " << result.error().toString() << std::endl;
        // Continue anyway, as this may not be critical
    }

    // Set edge detection for pedestrian button
    result = pedestrianButton->setEdge(fmus::gpio::GPIOEdge::Falling);
    if (!result.isOk()) {
        std::cerr << "Error setting edge detection: " << result.error().toString() << std::endl;
        // Continue anyway, as polling can be used as fallback
    }

    // Start with all lights off
    turnOffAllLights(trafficLights);

    // Initial state
    TrafficLightState state = TrafficLightState::Red;
    auto stateStartTime = std::chrono::steady_clock::now();
    bool pedestrianRequested = false;

    // Turn on red light initially
    result = trafficLights[0]->write(true);
    if (!result.isOk()) {
        std::cerr << "Error turning on red light: " << result.error().toString() << std::endl;
    }

    std::cout << "Traffic light controller is running. Press Ctrl+C to exit." << std::endl;

    // Main loop
    while (g_running) {
        // Check for pedestrian button press (active low with pull-up resistor)
        auto buttonResult = pedestrianButton->read();
        if (buttonResult.isOk() && !buttonResult.value() && !pedestrianRequested) {
            std::cout << "Pedestrian button pressed!" << std::endl;
            pedestrianRequested = true;
        }

        // Current time
        auto now = std::chrono::steady_clock::now();
        auto elapsedTime = now - stateStartTime;

        // State machine logic
        switch (state) {
            case TrafficLightState::Red: {
                // Check if it's time to change state
                if (elapsedTime >= RED_DURATION) {
                    // Turn off red, turn on green
                    trafficLights[0]->write(false);
                    trafficLights[2]->write(true);

                    // Update state
                    state = TrafficLightState::Green;
                    stateStartTime = now;
                    std::cout << "Changing to GREEN" << std::endl;
                }
                break;
            }

            case TrafficLightState::Yellow: {
                // Check if it's time to change state
                if (elapsedTime >= YELLOW_DURATION) {
                    // Turn off yellow, turn on red
                    trafficLights[1]->write(false);
                    trafficLights[0]->write(true);

                    // Update state
                    state = TrafficLightState::Red;
                    stateStartTime = now;
                    std::cout << "Changing to RED" << std::endl;
                }
                break;
            }

            case TrafficLightState::Green: {
                // Check if pedestrian requested crossing or if green time elapsed
                if ((pedestrianRequested || elapsedTime >= GREEN_DURATION)) {
                    // Turn off green, turn on yellow
                    trafficLights[2]->write(false);
                    trafficLights[1]->write(true);

                    // Update state
                    state = TrafficLightState::Yellow;
                    stateStartTime = now;
                    std::cout << "Changing to YELLOW" << std::endl;

                    if (pedestrianRequested) {
                        std::cout << "Preparing for pedestrian crossing..." << std::endl;
                    }
                }
                break;
            }

            case TrafficLightState::PedestrianCrossing: {
                // Blink red light for pedestrian crossing (not implemented in this simple example)
                // In a more complex example, we would have a dedicated pedestrian light

                // Check if pedestrian crossing time elapsed
                if (elapsedTime >= PEDESTRIAN_CROSSING_DURATION) {
                    // Reset pedestrian request
                    pedestrianRequested = false;

                    // Go back to red state
                    state = TrafficLightState::Red;
                    stateStartTime = now;
                    std::cout << "Pedestrian crossing complete, changing to RED" << std::endl;
                }
                break;
            }
        }

        // Small delay to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // Turn off all lights before exiting
    turnOffAllLights(trafficLights);

    // Clean up
    std::cout << "Shutting down traffic light controller..." << std::endl;
    fmus::shutdown();

    return 0;
}
