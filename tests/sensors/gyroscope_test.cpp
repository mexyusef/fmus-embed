#include <gtest/gtest.h>
#include "fmus/sensors/gyroscope.h"

using namespace fmus::sensors;

TEST(GyroscopeTest, DataMagnitude) {
    GyroscopeData data;
    data.x = 30.0f;
    data.y = 40.0f;
    data.z = 0.0f;
    
    EXPECT_FLOAT_EQ(data.getMagnitude(), 50.0f);
}

TEST(GyroscopeTest, StationaryDetection) {
    GyroscopeData data;
    data.x = 0.5f;
    data.y = 0.3f;
    data.z = 0.2f;
    
    EXPECT_TRUE(data.isStationary(1.0f));
    EXPECT_FALSE(data.isStationary(0.1f));
}
