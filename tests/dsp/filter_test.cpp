#include <gtest/gtest.h>
#include "fmus/dsp/filter.h"

using namespace fmus::dsp;

TEST(FilterTest, LowPassFilter) {
    LowPassFilter filter(0.1f);
    std::vector<float> input = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    auto output = filter.process(input);
    EXPECT_EQ(output.size(), input.size());
}

TEST(FilterTest, MovingAverage) {
    MovingAverageFilter filter(3);
    std::vector<float> input = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    auto output = filter.process(input);
    EXPECT_EQ(output.size(), input.size());
}
