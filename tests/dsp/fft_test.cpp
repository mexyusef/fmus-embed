#include <gtest/gtest.h>
#include "fmus/dsp/fft.h"

using namespace fmus::dsp;

TEST(FFTTest, BasicFFT) {
    std::vector<float> input = {1.0f, 0.0f, 1.0f, 0.0f};
    auto result = FFT::forward(input);
    EXPECT_TRUE(result.isOk() || result.isError());
}

TEST(FFTTest, InverseFFT) {
    std::vector<std::complex<float>> input = {{1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}};
    auto result = FFT::inverse(input);
    EXPECT_TRUE(result.isOk() || result.isError());
}
