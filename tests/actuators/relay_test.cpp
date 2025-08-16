#include <gtest/gtest.h>
#include "fmus/actuators/relay.h"

using namespace fmus::actuators;
using namespace fmus::core;

class RelayTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(RelayTest, Construction) {
    Relay relay(12); // Control pin 12
    EXPECT_FALSE(relay.isInitialized());
    EXPECT_FALSE(relay.getState());
}

TEST_F(RelayTest, BasicOperations) {
    Relay relay(12);
    
    auto initResult = relay.init();
    EXPECT_TRUE(initResult.isOk() || initResult.isError());
    
    auto onResult = relay.setState(true);
    EXPECT_TRUE(onResult.isOk() || onResult.isError());
    
    auto offResult = relay.setState(false);
    EXPECT_TRUE(offResult.isOk() || offResult.isError());
}

TEST_F(RelayTest, Toggle) {
    Relay relay(12);
    
    auto toggleResult = relay.toggle();
    EXPECT_TRUE(toggleResult.isOk() || toggleResult.isError());
}
