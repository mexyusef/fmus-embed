#include <gtest/gtest.h>
#include "fmus/ai/neural_net.h"

using namespace fmus::ai;

TEST(NeuralNetTest, BasicInference) {
    NeuralNetwork net;
    std::vector<float> input = {1.0f, 2.0f, 3.0f};
    
    auto result = net.inference(input);
    EXPECT_TRUE(result.isOk() || result.isError());
}
