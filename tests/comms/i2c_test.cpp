#include <gtest/gtest.h>
#include "fmus/comms/i2c.h"

using namespace fmus::comms;
using namespace fmus::core;

class I2CTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(I2CTest, Configuration) {
    I2cConfig config;
    EXPECT_EQ(config.busNumber, 0);
    EXPECT_EQ(config.speed, I2cSpeed::Standard);
    EXPECT_TRUE(config.pullUpsEnabled);
    EXPECT_EQ(config.timeoutMs, 1000);
    
    I2cConfig customConfig(1, I2cSpeed::Fast, false, 2000);
    EXPECT_EQ(customConfig.busNumber, 1);
    EXPECT_EQ(customConfig.speed, I2cSpeed::Fast);
    EXPECT_FALSE(customConfig.pullUpsEnabled);
    EXPECT_EQ(customConfig.timeoutMs, 2000);
}

TEST_F(I2CTest, Construction) {
    I2cConfig config(0, I2cSpeed::Standard);
    I2cMaster i2c(config);
    
    const I2cConfig& retrievedConfig = i2c.getConfig();
    EXPECT_EQ(retrievedConfig.busNumber, 0);
    EXPECT_EQ(retrievedConfig.speed, I2cSpeed::Standard);
}

TEST_F(I2CTest, WriteWithoutInit) {
    I2cMaster i2c;
    std::vector<uint8_t> data = {0x01, 0x02};
    
    auto result = i2c.write(0x50, data);
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.error().code(), ErrorCode::NotInitialized);
}

TEST_F(I2CTest, ReadWithoutInit) {
    I2cMaster i2c;
    
    auto result = i2c.read(0x50, 2);
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.error().code(), ErrorCode::NotInitialized);
}

TEST_F(I2CTest, EmptyDataWrite) {
    I2cMaster i2c;
    std::vector<uint8_t> emptyData;
    
    auto result = i2c.write(0x50, emptyData);
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.error().code(), ErrorCode::NotInitialized);
}

TEST_F(I2CTest, ZeroLengthRead) {
    I2cMaster i2c;
    
    auto result = i2c.read(0x50, 0);
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.error().code(), ErrorCode::NotInitialized);
}

TEST_F(I2CTest, SpeedSettings) {
    std::vector<I2cSpeed> speeds = {
        I2cSpeed::Standard,
        I2cSpeed::Fast,
        I2cSpeed::FastPlus,
        I2cSpeed::HighSpeed
    };
    
    for (auto speed : speeds) {
        I2cConfig config(0, speed);
        I2cMaster i2c(config);
        EXPECT_EQ(i2c.getConfig().speed, speed);
    }
}
