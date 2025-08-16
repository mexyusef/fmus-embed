#include <gtest/gtest.h>
#include "fmus/core/result.h"

using namespace fmus::core;

class ResultTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(ResultTest, SuccessfulResult) {
    Result<int> result(42);
    
    EXPECT_TRUE(result.isOk());
    EXPECT_FALSE(result.isError());
    EXPECT_EQ(result.value(), 42);
    EXPECT_EQ(result.valueOr(0), 42);
}

TEST_F(ResultTest, ErrorResult) {
    Error error(ErrorCode::InvalidArgument, "Test error");
    Result<int> result(error);
    
    EXPECT_FALSE(result.isOk());
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.error().code(), ErrorCode::InvalidArgument);
    EXPECT_EQ(result.error().message(), "Test error");
    EXPECT_EQ(result.valueOr(99), 99);
}

TEST_F(ResultTest, MoveConstruction) {
    std::string testStr = "Hello World";
    Result<std::string> result(std::move(testStr));
    
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(result.value(), "Hello World");
}

TEST_F(ResultTest, VoidResult) {
    Result<void> successResult;
    EXPECT_TRUE(successResult.isOk());
    EXPECT_FALSE(successResult.isError());
    
    Error error(ErrorCode::Timeout, "Operation timed out");
    Result<void> errorResult(error);
    EXPECT_FALSE(errorResult.isOk());
    EXPECT_TRUE(errorResult.isError());
    EXPECT_EQ(errorResult.error().code(), ErrorCode::Timeout);
}

TEST_F(ResultTest, HelperFunctions) {
    auto okResult = makeOk(123);
    EXPECT_TRUE(okResult.isOk());
    EXPECT_EQ(okResult.value(), 123);
    
    auto voidOkResult = makeOk();
    EXPECT_TRUE(voidOkResult.isOk());
    
    auto errorResult = makeError<int>(ErrorCode::DataError, "Data corruption");
    EXPECT_TRUE(errorResult.isError());
    EXPECT_EQ(errorResult.error().code(), ErrorCode::DataError);
    
    auto voidErrorResult = makeError(ErrorCode::NotSupported, "Feature not supported");
    EXPECT_TRUE(voidErrorResult.isError());
    EXPECT_EQ(voidErrorResult.error().code(), ErrorCode::NotSupported);
}

TEST_F(ResultTest, OnSuccessCallback) {
    bool callbackCalled = false;
    int callbackValue = 0;
    
    Result<int> result(42);
    result.onSuccess([&](int value) {
        callbackCalled = true;
        callbackValue = value;
    });
    
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(callbackValue, 42);
}

TEST_F(ResultTest, OnErrorCallback) {
    bool callbackCalled = false;
    ErrorCode callbackCode = ErrorCode::Ok;
    
    Error error(ErrorCode::InvalidArgument, "Test error");
    Result<int> result(error);
    result.onError([&](const Error& err) {
        callbackCalled = true;
        callbackCode = err.code();
    });
    
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(callbackCode, ErrorCode::InvalidArgument);
}

TEST_F(ResultTest, VoidOnSuccessCallback) {
    bool callbackCalled = false;
    
    Result<void> result;
    result.onSuccess([&]() {
        callbackCalled = true;
    });
    
    EXPECT_TRUE(callbackCalled);
}

TEST_F(ResultTest, VoidOnErrorCallback) {
    bool callbackCalled = false;
    
    Error error(ErrorCode::Timeout, "Test timeout");
    Result<void> result(error);
    result.onError([&](const Error& err) {
        callbackCalled = true;
    });
    
    EXPECT_TRUE(callbackCalled);
}

TEST_F(ResultTest, ChainedCallbacks) {
    int successCount = 0;
    int errorCount = 0;
    
    Result<int> successResult(100);
    successResult.onSuccess([&](int) { successCount++; })
                 .onError([&](const Error&) { errorCount++; });
    
    EXPECT_EQ(successCount, 1);
    EXPECT_EQ(errorCount, 0);
    
    Error error(ErrorCode::DataError, "Test error");
    Result<int> errorResult(error);
    errorResult.onSuccess([&](int) { successCount++; })
               .onError([&](const Error&) { errorCount++; });
    
    EXPECT_EQ(successCount, 1);
    EXPECT_EQ(errorCount, 1);
}

TEST_F(ResultTest, ComplexTypes) {
    std::vector<int> data = {1, 2, 3, 4, 5};
    Result<std::vector<int>> result(data);
    
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(result.value().size(), 5);
    EXPECT_EQ(result.value()[0], 1);
    EXPECT_EQ(result.value()[4], 5);
}
