#include <gtest/gtest.h>
#include "fmus/core/error.h"

using namespace fmus::core;

class ErrorTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(ErrorTest, ErrorCodeToString) {
    EXPECT_EQ(errorCodeToString(ErrorCode::Ok), "Ok");
    EXPECT_EQ(errorCodeToString(ErrorCode::Unknown), "Unknown");
    EXPECT_EQ(errorCodeToString(ErrorCode::InvalidArgument), "Invalid argument");
    EXPECT_EQ(errorCodeToString(ErrorCode::NotImplemented), "Not implemented");
    EXPECT_EQ(errorCodeToString(ErrorCode::Timeout), "Timeout");
}

TEST_F(ErrorTest, ErrorConstruction) {
    Error error(ErrorCode::InvalidArgument, "Test error message");
    
    EXPECT_EQ(error.code(), ErrorCode::InvalidArgument);
    EXPECT_EQ(error.message(), "Test error message");
    EXPECT_FALSE(error.isOk());
}

TEST_F(ErrorTest, ErrorOkConstruction) {
    Error okError(ErrorCode::Ok, "");
    
    EXPECT_EQ(okError.code(), ErrorCode::Ok);
    EXPECT_TRUE(okError.isOk());
}

TEST_F(ErrorTest, ErrorToString) {
    Error error(ErrorCode::CommInitFailed, "Failed to initialize communication");
    std::string errorStr = error.toString();
    
    EXPECT_TRUE(errorStr.find("Communication initialization failed") != std::string::npos);
    EXPECT_TRUE(errorStr.find("Failed to initialize communication") != std::string::npos);
}

TEST_F(ErrorTest, AllErrorCodes) {
    // Test that all error codes have string representations
    std::vector<ErrorCode> codes = {
        ErrorCode::Ok,
        ErrorCode::Unknown,
        ErrorCode::InvalidArgument,
        ErrorCode::NotImplemented,
        ErrorCode::NotSupported,
        ErrorCode::Timeout,
        ErrorCode::ResourceUnavailable,
        ErrorCode::InsufficientMemory,
        ErrorCode::NotInitialized,
        ErrorCode::DataError,
        ErrorCode::McuInitFailed,
        ErrorCode::PinConfigError,
        ErrorCode::TimerError,
        ErrorCode::AdcError,
        ErrorCode::SensorInitFailed,
        ErrorCode::SensorReadError,
        ErrorCode::SensorCalibrationError,
        ErrorCode::ActuatorInitFailed,
        ErrorCode::ActuatorSetValueError,
        ErrorCode::CommInitFailed,
        ErrorCode::CommTransmitError,
        ErrorCode::CommReceiveError,
        ErrorCode::CommConnectionError,
        ErrorCode::DspInitFailed,
        ErrorCode::DspComputationError,
        ErrorCode::AiInitFailed,
        ErrorCode::AiModelError,
        ErrorCode::NetworkInitFailed,
        ErrorCode::NetworkConnectionError,
        ErrorCode::NetworkProtocolError,
        ErrorCode::GPIOError,
        ErrorCode::GPIOInitFailed,
        ErrorCode::GPIOWriteError,
        ErrorCode::GPIOReadError,
        ErrorCode::GPIOInterruptError
    };
    
    for (auto code : codes) {
        std::string str = errorCodeToString(code);
        EXPECT_FALSE(str.empty()) << "Error code " << static_cast<int>(code) << " has no string representation";
        EXPECT_NE(str, "Unknown") << "Error code " << static_cast<int>(code) << " returns 'Unknown'";
    }
}

TEST_F(ErrorTest, ErrorCategories) {
    // Test error code ranges
    EXPECT_LT(static_cast<uint32_t>(ErrorCode::McuInitFailed), 2000u);
    EXPECT_GE(static_cast<uint32_t>(ErrorCode::McuInitFailed), 1000u);
    
    EXPECT_LT(static_cast<uint32_t>(ErrorCode::SensorInitFailed), 3000u);
    EXPECT_GE(static_cast<uint32_t>(ErrorCode::SensorInitFailed), 2000u);
    
    EXPECT_LT(static_cast<uint32_t>(ErrorCode::ActuatorInitFailed), 4000u);
    EXPECT_GE(static_cast<uint32_t>(ErrorCode::ActuatorInitFailed), 3000u);
    
    EXPECT_LT(static_cast<uint32_t>(ErrorCode::CommInitFailed), 5000u);
    EXPECT_GE(static_cast<uint32_t>(ErrorCode::CommInitFailed), 4000u);
}
