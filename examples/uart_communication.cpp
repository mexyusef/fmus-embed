#include <fmus/fmus.h>
#include <iostream>
#include <thread>
#include <chrono>

using namespace fmus;
using namespace fmus::comms;

// Simple helper to check results
template<typename T>
void checkResult(const core::Result<T>& result, const std::string& operation) {
    if (result.isError()) {
        std::cerr << "Error in " << operation << ": " << result.error().toString() << std::endl;
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
    std::cout << "UART Communication Example" << std::endl;
    std::cout << "=========================" << std::endl;

    // Create UART instance
    UART uart(0);  // Use UART port 0

    // Configure UART
    UARTConfig config;
    config.baudRate = 115200;
    config.dataBits = UARTDataBits::Eight;
    config.parity = UARTParity::None;
    config.stopBits = UARTStopBits::One;
    config.flowControl = UARTFlowControl::None;
    config.timeoutMs = 1000;
    config.useInterrupts = true;

    std::cout << "Initializing UART with configuration:" << std::endl;
    std::cout << "  Baud Rate: " << config.baudRate << std::endl;
    std::cout << "  Data Bits: " << static_cast<int>(config.dataBits) << std::endl;
    std::cout << "  Parity: " << uartParityToString(config.parity) << std::endl;
    std::cout << "  Stop Bits: " << uartStopBitsToString(config.stopBits) << std::endl;
    std::cout << "  Flow Control: " << uartFlowControlToString(config.flowControl) << std::endl;

    // Initialize UART
    auto initResult = uart.init(config);
    if (initResult.isError()) {
        std::cout << "Note: UART initialization failed (expected on systems without UART hardware)" << std::endl;
        std::cout << "Error: " << initResult.error().toString() << std::endl;
        std::cout << "This is normal when running on a system without physical UART ports." << std::endl;
        
        // Show what the API would look like
        std::cout << "\nDemonstrating UART API usage (simulation):" << std::endl;
        
        // Simulate some operations
        std::string testMessage = "Hello, UART World!";
        std::cout << "Would send: \"" << testMessage << "\"" << std::endl;
        
        std::cout << "Configuration details:" << std::endl;
        std::cout << "  Port Number: " << static_cast<int>(uart.getPortNumber()) << std::endl;
        std::cout << "  Initialized: " << (uart.isInitialized() ? "Yes" : "No") << std::endl;
        
        fmus::shutdown();
        return 0;
    }

    std::cout << "UART initialized successfully!" << std::endl;

    // Set up data callback for incoming data
    uart.setDataCallback([](const std::vector<uint8_t>& data) {
        std::cout << "Received " << data.size() << " bytes: ";
        for (uint8_t byte : data) {
            if (byte >= 32 && byte <= 126) {
                std::cout << static_cast<char>(byte);
            } else {
                std::cout << "[0x" << std::hex << static_cast<int>(byte) << std::dec << "]";
            }
        }
        std::cout << std::endl;
    });

    // Send some test data
    std::string testMessage = "Hello, UART World!\n";
    std::cout << "Sending: \"" << testMessage << "\"" << std::endl;
    
    auto writeResult = uart.write(testMessage);
    checkResult(writeResult, "write");

    // Send individual bytes
    std::cout << "Sending individual bytes..." << std::endl;
    for (char c : "Test123") {
        auto byteResult = uart.write(static_cast<uint8_t>(c));
        checkResult(byteResult, "write byte");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    uart.write(static_cast<uint8_t>('\n'));

    // Try to read some data
    std::cout << "Checking for available data..." << std::endl;
    auto availableResult = uart.available();
    if (availableResult.isOk()) {
        std::cout << "Available bytes: " << availableResult.value() << std::endl;
        
        if (availableResult.value() > 0) {
            auto readResult = uart.read(availableResult.value());
            if (readResult.isOk()) {
                std::cout << "Read " << readResult.value().size() << " bytes: ";
                for (uint8_t byte : readResult.value()) {
                    if (byte >= 32 && byte <= 126) {
                        std::cout << static_cast<char>(byte);
                    } else {
                        std::cout << "[0x" << std::hex << static_cast<int>(byte) << std::dec << "]";
                    }
                }
                std::cout << std::endl;
            }
        }
    }

    // Demonstrate configuration changes
    std::cout << "Changing baud rate to 9600..." << std::endl;
    auto baudResult = uart.setBaudRate(9600);
    checkResult(baudResult, "setBaudRate");

    // Show statistics
    std::cout << "\nUART Statistics:" << std::endl;
    std::cout << uart.getStatistics() << std::endl;

    // Test timeout functionality
    std::cout << "Testing read with timeout..." << std::endl;
    auto timeoutResult = uart.readLine();
    if (timeoutResult.isError()) {
        std::cout << "Read timeout (expected): " << timeoutResult.error().toString() << std::endl;
    } else {
        std::cout << "Read line: \"" << timeoutResult.value() << "\"" << std::endl;
    }

    // Flush and close
    std::cout << "Flushing and closing UART..." << std::endl;
    uart.flush();
    uart.close();

    std::cout << "UART example completed successfully!" << std::endl;

    // Clean up
    fmus::shutdown();
    return 0;
}
