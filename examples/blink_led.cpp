#include <fmus/fmus.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>

using namespace fmus;
using namespace fmus::mcu;

volatile bool running = true;

void signalHandler(int signal) {
    running = false;
}

int main() {
    // Register signal handler for Ctrl+C
    std::signal(SIGINT, signalHandler);

    // Initialize the fmus-embed library
    if (!fmus::init()) {
        std::cerr << "Failed to initialize fmus-embed library" << std::endl;
        return 1;
    }

    std::cout << "fmus-embed library version: " << core::getVersionString() << std::endl;

    // Create a GPIO pin for the LED
    GPIO led(13, PinMode::Output);  // Assuming pin 13 is connected to an LED

    // Initialize the pin
    auto result = led.init();
    if (result.isError()) {
        std::cerr << "Error initializing LED pin: " << result.error().toString() << std::endl;
        return 1;
    }

    std::cout << "LED pin initialized" << std::endl;
    std::cout << "Blinking LED (press Ctrl+C to exit)..." << std::endl;

    // Blink the LED
    bool state = false;
    while (running) {
        // Toggle state
        state = !state;

        // Set the pin state
        result = led.write(state);
        if (result.isError()) {
            std::cerr << "Error writing to LED pin: " << result.error().toString() << std::endl;
            break;
        }

        // Print state
        std::cout << "LED is " << (state ? "ON" : "OFF") << std::endl;

        // Wait a second
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Turn off the LED before exiting
    led.write(false);

    // Shutdown the library
    fmus::shutdown();
    std::cout << "Library shutdown" << std::endl;

    return 0;
}
