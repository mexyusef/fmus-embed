#include <gtest/gtest.h>
#include "fmus/comms/uart.h"
#include <thread>
#include <chrono>

using namespace fmus::comms;
using namespace fmus::core;

class UARTTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(UARTTest, Construction) {
    UART uart(0);
    EXPECT_EQ(uart.getPortNumber(), 0);
    EXPECT_FALSE(uart.isInitialized());
}

TEST_F(UARTTest, Configuration) {
    UARTConfig config;
    EXPECT_EQ(config.baudRate, 115200u);
    EXPECT_EQ(config.dataBits, UARTDataBits::Eight);
    EXPECT_EQ(config.parity, UARTParity::None);
    EXPECT_EQ(config.stopBits, UARTStopBits::One);
    EXPECT_EQ(config.flowControl, UARTFlowControl::None);
    
    // Test custom configuration
    UARTConfig customConfig(9600, UARTDataBits::Seven, UARTParity::Even, 
                           UARTStopBits::Two, UARTFlowControl::RTS_CTS);
    EXPECT_EQ(customConfig.baudRate, 9600u);
    EXPECT_EQ(customConfig.dataBits, UARTDataBits::Seven);
    EXPECT_EQ(customConfig.parity, UARTParity::Even);
    EXPECT_EQ(customConfig.stopBits, UARTStopBits::Two);
    EXPECT_EQ(customConfig.flowControl, UARTFlowControl::RTS_CTS);
}

TEST_F(UARTTest, InitializationFailure) {
    // On systems without UART hardware, initialization should fail gracefully
    UART uart(0);
    UARTConfig config;
    
    auto result = uart.init(config);
    // Either succeeds (if hardware available) or fails gracefully
    if (result.isError()) {
        EXPECT_EQ(result.error().code(), ErrorCode::CommInitFailed);
        EXPECT_FALSE(uart.isInitialized());
    } else {
        EXPECT_TRUE(uart.isInitialized());
        uart.close();
    }
}

TEST_F(UARTTest, WriteWithoutInitialization) {
    UART uart(0);
    std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    
    auto result = uart.write(data);
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.error().code(), ErrorCode::CommInitFailed);
}

TEST_F(UARTTest, ReadWithoutInitialization) {
    UART uart(0);
    
    auto result = uart.read(10);
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.error().code(), ErrorCode::CommInitFailed);
}

TEST_F(UARTTest, StringConversions) {
    EXPECT_EQ(uartParityToString(UARTParity::None), "None");
    EXPECT_EQ(uartParityToString(UARTParity::Even), "Even");
    EXPECT_EQ(uartParityToString(UARTParity::Odd), "Odd");
    EXPECT_EQ(uartParityToString(UARTParity::Mark), "Mark");
    EXPECT_EQ(uartParityToString(UARTParity::Space), "Space");
    
    EXPECT_EQ(uartStopBitsToString(UARTStopBits::One), "1");
    EXPECT_EQ(uartStopBitsToString(UARTStopBits::OneAndHalf), "1.5");
    EXPECT_EQ(uartStopBitsToString(UARTStopBits::Two), "2");
    
    EXPECT_EQ(uartFlowControlToString(UARTFlowControl::None), "None");
    EXPECT_EQ(uartFlowControlToString(UARTFlowControl::RTS_CTS), "RTS/CTS");
    EXPECT_EQ(uartFlowControlToString(UARTFlowControl::XON_XOFF), "XON/XOFF");
}

TEST_F(UARTTest, ConfigurationRetrieval) {
    UART uart(1);
    UARTConfig config(9600, UARTDataBits::Seven, UARTParity::Odd);
    
    // Try to initialize (may fail on systems without hardware)
    uart.init(config);
    
    const UARTConfig& retrievedConfig = uart.getConfig();
    EXPECT_EQ(retrievedConfig.baudRate, 9600u);
    EXPECT_EQ(retrievedConfig.dataBits, UARTDataBits::Seven);
    EXPECT_EQ(retrievedConfig.parity, UARTParity::Odd);
}

TEST_F(UARTTest, EmptyDataWrite) {
    UART uart(0);
    std::vector<uint8_t> emptyData;
    
    // Should fail because UART is not initialized
    auto result = uart.write(emptyData);
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.error().code(), ErrorCode::CommInitFailed);
}

TEST_F(UARTTest, ZeroSizeRead) {
    UART uart(0);
    
    // Should fail because UART is not initialized
    auto result = uart.read(0);
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.error().code(), ErrorCode::CommInitFailed);
}

TEST_F(UARTTest, CallbackSetup) {
    UART uart(0);
    bool callbackSet = false;
    
    auto result = uart.setDataCallback([&](const std::vector<uint8_t>& data) {
        callbackSet = true;
    });
    
    EXPECT_TRUE(result.isOk());
}

TEST_F(UARTTest, Statistics) {
    UART uart(0);
    std::string stats = uart.getStatistics();
    
    // Should contain port information
    EXPECT_TRUE(stats.find("UART Port 0") != std::string::npos);
    EXPECT_TRUE(stats.find("Bytes Transmitted") != std::string::npos);
    EXPECT_TRUE(stats.find("Bytes Received") != std::string::npos);
}

TEST_F(UARTTest, TransmissionStatus) {
    UART uart(0);
    
    // Should return false when not initialized
    EXPECT_FALSE(uart.isTransmitting());
}

TEST_F(UARTTest, MultipleUARTInstances) {
    UART uart0(0);
    UART uart1(1);
    UART uart2(2);
    
    EXPECT_EQ(uart0.getPortNumber(), 0);
    EXPECT_EQ(uart1.getPortNumber(), 1);
    EXPECT_EQ(uart2.getPortNumber(), 2);
    
    EXPECT_FALSE(uart0.isInitialized());
    EXPECT_FALSE(uart1.isInitialized());
    EXPECT_FALSE(uart2.isInitialized());
}

TEST_F(UARTTest, ConfigurationValidation) {
    // Test various configuration combinations
    std::vector<uint32_t> baudRates = {9600, 19200, 38400, 57600, 115200, 230400};
    std::vector<UARTDataBits> dataBits = {UARTDataBits::Five, UARTDataBits::Six, 
                                         UARTDataBits::Seven, UARTDataBits::Eight};
    std::vector<UARTParity> parities = {UARTParity::None, UARTParity::Even, UARTParity::Odd};
    std::vector<UARTStopBits> stopBits = {UARTStopBits::One, UARTStopBits::Two};
    
    for (auto baud : baudRates) {
        for (auto data : dataBits) {
            for (auto parity : parities) {
                for (auto stop : stopBits) {
                    UARTConfig config(baud, data, parity, stop);
                    EXPECT_EQ(config.baudRate, baud);
                    EXPECT_EQ(config.dataBits, data);
                    EXPECT_EQ(config.parity, parity);
                    EXPECT_EQ(config.stopBits, stop);
                }
            }
        }
    }
}

TEST_F(UARTTest, AsyncOperations) {
    UART uart(0);
    std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    bool callbackCalled = false;
    
    auto result = uart.writeAsync(data, [&](Result<void> result) {
        callbackCalled = true;
    });
    
    // Should fail because UART is not initialized, but callback mechanism should work
    EXPECT_TRUE(result.isOk()); // The async setup should succeed
    
    // Give some time for callback to be called
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_TRUE(callbackCalled);
}
