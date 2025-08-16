#include <gtest/gtest.h>
#include "fmus/comms/spi.h"

using namespace fmus::comms;
using namespace fmus::core;

class SPITest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(SPITest, Construction) {
    SPI spi(0);
    EXPECT_EQ(spi.getBusNumber(), 0);
    EXPECT_FALSE(spi.isInitialized());
}

TEST_F(SPITest, Configuration) {
    SPIConfig config;
    EXPECT_EQ(config.clockFrequency, 1000000u);
    EXPECT_EQ(config.mode, SPIMode::Mode0);
    EXPECT_EQ(config.bitOrder, SPIBitOrder::MSBFirst);
    EXPECT_EQ(config.dataBits, 8);
    EXPECT_FALSE(config.useDMA);
}

TEST_F(SPITest, ModeSettings) {
    SPI spi(0);
    
    spi.setMode(SPIMode::Mode1);
    spi.setClockFrequency(2000000);
    spi.setBitOrder(SPIBitOrder::LSBFirst);
    
    const SPIConfig& config = spi.getConfig();
    EXPECT_EQ(config.mode, SPIMode::Mode1);
    EXPECT_EQ(config.clockFrequency, 2000000u);
    EXPECT_EQ(config.bitOrder, SPIBitOrder::LSBFirst);
}

TEST_F(SPITest, StringConversions) {
    EXPECT_EQ(spiModeToString(SPIMode::Mode0), "Mode 0 (CPOL=0, CPHA=0)");
    EXPECT_EQ(spiModeToString(SPIMode::Mode1), "Mode 1 (CPOL=0, CPHA=1)");
    EXPECT_EQ(spiModeToString(SPIMode::Mode2), "Mode 2 (CPOL=1, CPHA=0)");
    EXPECT_EQ(spiModeToString(SPIMode::Mode3), "Mode 3 (CPOL=1, CPHA=1)");
}

TEST_F(SPITest, WriteWithoutInit) {
    SPI spi(0);
    std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    
    auto result = spi.write(data);
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.error().code(), ErrorCode::CommInitFailed);
}

TEST_F(SPITest, ChipSelectOperations) {
    SPI spi(0);
    
    // These should work even without initialization (just state tracking)
    EXPECT_NO_THROW(spi.select(0));
    EXPECT_NO_THROW(spi.deselect());
}

TEST_F(SPITest, MethodChaining) {
    SPI spi(0);
    
    // Test method chaining
    EXPECT_NO_THROW(
        spi.setMode(SPIMode::Mode2)
           .setClockFrequency(500000)
           .setBitOrder(SPIBitOrder::LSBFirst)
           .setDataBits(16)
    );
    
    const SPIConfig& config = spi.getConfig();
    EXPECT_EQ(config.mode, SPIMode::Mode2);
    EXPECT_EQ(config.clockFrequency, 500000u);
    EXPECT_EQ(config.bitOrder, SPIBitOrder::LSBFirst);
    EXPECT_EQ(config.dataBits, 16);
}
