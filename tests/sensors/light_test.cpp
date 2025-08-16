#include <gtest/gtest.h>
#include "fmus/sensors/light.h"

using namespace fmus::sensors;

TEST(LightTest, DarknessDetection) {
    LightSensorData data;
    data.lux = 5.0f;
    
    EXPECT_TRUE(data.isDark(10.0f));
    EXPECT_FALSE(data.isDark(1.0f));
}

TEST(LightTest, LightLevelDescription) {
    LightSensorData data;
    
    data.lux = 1.0f;
    EXPECT_EQ(data.getLightLevelDescription(), "Dark");
    
    data.lux = 50.0f;
    EXPECT_EQ(data.getLightLevelDescription(), "Dim");
    
    data.lux = 500.0f;
    EXPECT_EQ(data.getLightLevelDescription(), "Normal");
}
