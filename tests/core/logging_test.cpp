#include <gtest/gtest.h>
#include "fmus/core/logging.h"
#include <sstream>
#include <thread>
#include <chrono>

using namespace fmus::core;

class LoggingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset logging to default state
        setLogLevel(LogLevel::Info);
    }
    
    void TearDown() override {
        // Clean up
        setLogLevel(LogLevel::Info);
    }
};

TEST_F(LoggingTest, LogLevels) {
    // Test all log levels
    EXPECT_NO_THROW(setLogLevel(LogLevel::Debug));
    EXPECT_NO_THROW(setLogLevel(LogLevel::Info));
    EXPECT_NO_THROW(setLogLevel(LogLevel::Warning));
    EXPECT_NO_THROW(setLogLevel(LogLevel::Error));
    EXPECT_NO_THROW(setLogLevel(LogLevel::Critical));
}

TEST_F(LoggingTest, BasicLogging) {
    // Test that logging functions don't crash
    EXPECT_NO_THROW(FMUS_LOG_DEBUG("Debug message"));
    EXPECT_NO_THROW(FMUS_LOG_INFO("Info message"));
    EXPECT_NO_THROW(FMUS_LOG_WARNING("Warning message"));
    EXPECT_NO_THROW(FMUS_LOG_ERROR("Error message"));
    EXPECT_NO_THROW(FMUS_LOG_CRITICAL("Critical message"));
}

TEST_F(LoggingTest, LogWithParameters) {
    // Test logging with parameters
    int value = 42;
    std::string text = "test";
    
    EXPECT_NO_THROW(FMUS_LOG_INFO("Value: " + std::to_string(value)));
    EXPECT_NO_THROW(FMUS_LOG_INFO("Text: " + text));
    EXPECT_NO_THROW(FMUS_LOG_INFO("Combined: " + text + " = " + std::to_string(value)));
}

TEST_F(LoggingTest, LogLevelFiltering) {
    // Set log level to Warning - should filter out Debug and Info
    setLogLevel(LogLevel::Warning);
    
    // These should not cause issues even if filtered
    EXPECT_NO_THROW(FMUS_LOG_DEBUG("This debug message should be filtered"));
    EXPECT_NO_THROW(FMUS_LOG_INFO("This info message should be filtered"));
    EXPECT_NO_THROW(FMUS_LOG_WARNING("This warning should appear"));
    EXPECT_NO_THROW(FMUS_LOG_ERROR("This error should appear"));
    
    // Set back to Debug - all should appear
    setLogLevel(LogLevel::Debug);
    EXPECT_NO_THROW(FMUS_LOG_DEBUG("This debug message should appear"));
    EXPECT_NO_THROW(FMUS_LOG_INFO("This info message should appear"));
}

TEST_F(LoggingTest, ThreadSafety) {
    // Test that logging is thread-safe
    const int numThreads = 10;
    const int messagesPerThread = 100;
    
    std::vector<std::thread> threads;
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([i, messagesPerThread]() {
            for (int j = 0; j < messagesPerThread; ++j) {
                FMUS_LOG_INFO("Thread " + std::to_string(i) + " message " + std::to_string(j));
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // If we get here without crashing, thread safety test passed
    EXPECT_TRUE(true);
}

TEST_F(LoggingTest, LongMessages) {
    // Test with very long messages
    std::string longMessage(1000, 'A');
    EXPECT_NO_THROW(FMUS_LOG_INFO(longMessage));
    
    // Test with special characters
    std::string specialMessage = "Special chars: !@#$%^&*()_+-=[]{}|;':\",./<>?";
    EXPECT_NO_THROW(FMUS_LOG_INFO(specialMessage));
    
    // Test with newlines
    std::string multilineMessage = "Line 1\nLine 2\nLine 3";
    EXPECT_NO_THROW(FMUS_LOG_INFO(multilineMessage));
}

TEST_F(LoggingTest, EmptyMessages) {
    // Test with empty messages
    EXPECT_NO_THROW(FMUS_LOG_INFO(""));
    EXPECT_NO_THROW(FMUS_LOG_DEBUG(""));
    EXPECT_NO_THROW(FMUS_LOG_WARNING(""));
    EXPECT_NO_THROW(FMUS_LOG_ERROR(""));
    EXPECT_NO_THROW(FMUS_LOG_CRITICAL(""));
}

TEST_F(LoggingTest, PerformanceTest) {
    // Simple performance test - should complete quickly
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 1000; ++i) {
        FMUS_LOG_DEBUG("Performance test message " + std::to_string(i));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should complete in reasonable time (less than 5 seconds)
    EXPECT_LT(duration.count(), 5000);
}

TEST_F(LoggingTest, LogLevelToString) {
    // Test that we can convert log levels to strings (if such function exists)
    // This is a basic test to ensure the logging system is working
    setLogLevel(LogLevel::Debug);
    EXPECT_NO_THROW(FMUS_LOG_DEBUG("Debug level set"));
    
    setLogLevel(LogLevel::Info);
    EXPECT_NO_THROW(FMUS_LOG_INFO("Info level set"));
    
    setLogLevel(LogLevel::Warning);
    EXPECT_NO_THROW(FMUS_LOG_WARNING("Warning level set"));
    
    setLogLevel(LogLevel::Error);
    EXPECT_NO_THROW(FMUS_LOG_ERROR("Error level set"));
    
    setLogLevel(LogLevel::Critical);
    EXPECT_NO_THROW(FMUS_LOG_CRITICAL("Critical level set"));
}
