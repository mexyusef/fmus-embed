#include "fmus/dsp/filter.h"
#include "fmus/core/logging.h"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace fmus {
namespace dsp {

//=============================================================================
// LowPassFilter Implementation
//=============================================================================

template<typename T>
LowPassFilter<T>::LowPassFilter(T alpha)
    : m_alpha(alpha), m_previousOutput(0), m_order(1), 
      m_implementation(FilterImplementation::IIR), m_impl(nullptr) {
    if (alpha <= 0 || alpha >= 1) {
        FMUS_LOG_WARNING("LowPassFilter alpha should be between 0 and 1, clamping to valid range");
        m_alpha = std::clamp(alpha, static_cast<T>(0.001), static_cast<T>(0.999));
    }
}

template<typename T>
LowPassFilter<T>::LowPassFilter(T cutoffFreq, uint32_t order)
    : m_order(order), m_implementation(FilterImplementation::Butterworth), m_impl(nullptr) {
    // Simple approximation for Butterworth filter using RC equivalent
    // For more accurate implementation, would use proper Butterworth design
    m_alpha = static_cast<T>(1.0) - std::exp(-2.0 * M_PI * cutoffFreq);
    m_alpha = std::clamp(m_alpha, static_cast<T>(0.001), static_cast<T>(0.999));
    m_previousOutput = 0;
}

template<typename T>
LowPassFilter<T>::~LowPassFilter() = default;

template<typename T>
T LowPassFilter<T>::process(T input) {
    // Simple first-order IIR low-pass filter: y[n] = α*x[n] + (1-α)*y[n-1]
    m_previousOutput = m_alpha * input + (1 - m_alpha) * m_previousOutput;
    return m_previousOutput;
}

template<typename T>
std::vector<T> LowPassFilter<T>::process(const std::vector<T>& input) {
    std::vector<T> output;
    output.reserve(input.size());
    
    for (const T& sample : input) {
        output.push_back(process(sample));
    }
    
    return output;
}

template<typename T>
void LowPassFilter<T>::reset() {
    m_previousOutput = 0;
}

template<typename T>
FilterImplementation LowPassFilter<T>::getImplementation() const {
    return m_implementation;
}

template<typename T>
uint32_t LowPassFilter<T>::getOrder() const {
    return m_order;
}

template<typename T>
void LowPassFilter<T>::setAlpha(T alpha) {
    m_alpha = std::clamp(alpha, static_cast<T>(0.001), static_cast<T>(0.999));
}

template<typename T>
T LowPassFilter<T>::getAlpha() const {
    return m_alpha;
}

//=============================================================================
// HighPassFilter Implementation
//=============================================================================

template<typename T>
HighPassFilter<T>::HighPassFilter(T alpha)
    : m_alpha(alpha), m_previousInput(0), m_previousOutput(0), m_order(1),
      m_implementation(FilterImplementation::IIR), m_impl(nullptr) {
    if (alpha <= 0 || alpha >= 1) {
        FMUS_LOG_WARNING("HighPassFilter alpha should be between 0 and 1, clamping to valid range");
        m_alpha = std::clamp(alpha, static_cast<T>(0.001), static_cast<T>(0.999));
    }
}

template<typename T>
HighPassFilter<T>::HighPassFilter(T cutoffFreq, uint32_t order)
    : m_order(order), m_implementation(FilterImplementation::Butterworth), m_impl(nullptr) {
    // Simple approximation for high-pass filter
    m_alpha = static_cast<T>(1.0) - std::exp(-2.0 * M_PI * cutoffFreq);
    m_alpha = std::clamp(m_alpha, static_cast<T>(0.001), static_cast<T>(0.999));
    m_previousInput = 0;
    m_previousOutput = 0;
}

template<typename T>
HighPassFilter<T>::~HighPassFilter() = default;

template<typename T>
T HighPassFilter<T>::process(T input) {
    // First-order IIR high-pass filter: y[n] = α*(y[n-1] + x[n] - x[n-1])
    m_previousOutput = m_alpha * (m_previousOutput + input - m_previousInput);
    m_previousInput = input;
    return m_previousOutput;
}

template<typename T>
std::vector<T> HighPassFilter<T>::process(const std::vector<T>& input) {
    std::vector<T> output;
    output.reserve(input.size());
    
    for (const T& sample : input) {
        output.push_back(process(sample));
    }
    
    return output;
}

template<typename T>
void HighPassFilter<T>::reset() {
    m_previousInput = 0;
    m_previousOutput = 0;
}

template<typename T>
FilterImplementation HighPassFilter<T>::getImplementation() const {
    return m_implementation;
}

template<typename T>
uint32_t HighPassFilter<T>::getOrder() const {
    return m_order;
}

//=============================================================================
// BandPassFilter Implementation
//=============================================================================

template<typename T>
BandPassFilter<T>::BandPassFilter(T lowCutoff, T highCutoff, uint32_t order)
    : m_lowCutoff(lowCutoff), m_highCutoff(highCutoff), m_order(order) {
    
    if (lowCutoff >= highCutoff) {
        FMUS_LOG_ERROR("BandPassFilter: low cutoff must be less than high cutoff");
        std::swap(m_lowCutoff, m_highCutoff);
    }
    
    // Create cascade of high-pass and low-pass filters
    m_highPass = std::make_unique<HighPassFilter<T>>(m_lowCutoff, order / 2);
    m_lowPass = std::make_unique<LowPassFilter<T>>(m_highCutoff, order / 2);
}

template<typename T>
BandPassFilter<T>::~BandPassFilter() = default;

template<typename T>
T BandPassFilter<T>::process(T input) {
    // Process through high-pass then low-pass
    T highPassOutput = m_highPass->process(input);
    return m_lowPass->process(highPassOutput);
}

template<typename T>
std::vector<T> BandPassFilter<T>::process(const std::vector<T>& input) {
    std::vector<T> output;
    output.reserve(input.size());
    
    for (const T& sample : input) {
        output.push_back(process(sample));
    }
    
    return output;
}

template<typename T>
void BandPassFilter<T>::reset() {
    m_highPass->reset();
    m_lowPass->reset();
}

template<typename T>
FilterImplementation BandPassFilter<T>::getImplementation() const {
    return FilterImplementation::IIR;
}

template<typename T>
uint32_t BandPassFilter<T>::getOrder() const {
    return m_order;
}

//=============================================================================
// MovingAverageFilter Implementation
//=============================================================================

template<typename T>
MovingAverageFilter<T>::MovingAverageFilter(uint32_t windowSize)
    : m_windowSize(windowSize), m_index(0), m_sum(0), m_bufferFull(false) {
    if (windowSize == 0) {
        FMUS_LOG_ERROR("MovingAverageFilter: window size cannot be zero, setting to 1");
        m_windowSize = 1;
    }
    m_buffer.resize(m_windowSize, 0);
}

template<typename T>
MovingAverageFilter<T>::~MovingAverageFilter() = default;

template<typename T>
T MovingAverageFilter<T>::process(T input) {
    // Remove old value from sum
    m_sum -= m_buffer[m_index];
    
    // Add new value
    m_buffer[m_index] = input;
    m_sum += input;
    
    // Update index
    m_index = (m_index + 1) % m_windowSize;
    if (m_index == 0) {
        m_bufferFull = true;
    }
    
    // Calculate average
    uint32_t divisor = m_bufferFull ? m_windowSize : (m_index == 0 ? m_windowSize : m_index);
    return m_sum / static_cast<T>(divisor);
}

template<typename T>
std::vector<T> MovingAverageFilter<T>::process(const std::vector<T>& input) {
    std::vector<T> output;
    output.reserve(input.size());
    
    for (const T& sample : input) {
        output.push_back(process(sample));
    }
    
    return output;
}

template<typename T>
void MovingAverageFilter<T>::reset() {
    std::fill(m_buffer.begin(), m_buffer.end(), 0);
    m_index = 0;
    m_sum = 0;
    m_bufferFull = false;
}

template<typename T>
uint32_t MovingAverageFilter<T>::getOrder() const {
    return m_windowSize;
}

template<typename T>
void MovingAverageFilter<T>::setWindowSize(uint32_t windowSize) {
    if (windowSize == 0) {
        FMUS_LOG_ERROR("MovingAverageFilter: window size cannot be zero");
        return;
    }
    
    m_windowSize = windowSize;
    m_buffer.resize(m_windowSize, 0);
    reset();
}

template<typename T>
uint32_t MovingAverageFilter<T>::getWindowSize() const {
    return m_windowSize;
}

//=============================================================================
// MedianFilter Implementation
//=============================================================================

template<typename T>
MedianFilter<T>::MedianFilter(uint32_t windowSize)
    : m_windowSize(windowSize), m_index(0), m_bufferFull(false) {
    if (windowSize == 0) {
        FMUS_LOG_ERROR("MedianFilter: window size cannot be zero, setting to 1");
        m_windowSize = 1;
    }
    if (windowSize % 2 == 0) {
        FMUS_LOG_WARNING("MedianFilter: window size should be odd for best results");
    }
    m_buffer.resize(m_windowSize, 0);
}

template<typename T>
MedianFilter<T>::~MedianFilter() = default;

template<typename T>
T MedianFilter<T>::process(T input) {
    // Add new sample to buffer
    m_buffer[m_index] = input;
    m_index = (m_index + 1) % m_windowSize;
    if (m_index == 0) {
        m_bufferFull = true;
    }
    
    return calculateMedian();
}

template<typename T>
std::vector<T> MedianFilter<T>::process(const std::vector<T>& input) {
    std::vector<T> output;
    output.reserve(input.size());
    
    for (const T& sample : input) {
        output.push_back(process(sample));
    }
    
    return output;
}

template<typename T>
void MedianFilter<T>::reset() {
    std::fill(m_buffer.begin(), m_buffer.end(), 0);
    m_index = 0;
    m_bufferFull = false;
}

template<typename T>
uint32_t MedianFilter<T>::getOrder() const {
    return m_windowSize;
}

template<typename T>
T MedianFilter<T>::calculateMedian() {
    // Create a copy of the relevant portion of the buffer
    std::vector<T> sortBuffer;
    if (m_bufferFull) {
        sortBuffer = m_buffer;
    } else {
        sortBuffer.assign(m_buffer.begin(), m_buffer.begin() + m_index);
    }
    
    if (sortBuffer.empty()) {
        return 0;
    }
    
    // Sort the buffer
    std::sort(sortBuffer.begin(), sortBuffer.end());
    
    // Return median
    size_t size = sortBuffer.size();
    if (size % 2 == 0) {
        return (sortBuffer[size / 2 - 1] + sortBuffer[size / 2]) / 2;
    } else {
        return sortBuffer[size / 2];
    }
}

//=============================================================================
// KalmanFilter Implementation
//=============================================================================

template<typename T>
KalmanFilter<T>::KalmanFilter(T processNoise, T measurementNoise, T initialEstimate, T initialCovariance)
    : m_processNoise(processNoise), m_measurementNoise(measurementNoise),
      m_estimate(initialEstimate), m_covariance(initialCovariance),
      m_initialEstimate(initialEstimate), m_initialCovariance(initialCovariance) {
}

template<typename T>
T KalmanFilter<T>::update(T measurement) {
    // Prediction step
    // x_k|k-1 = x_k-1|k-1 (no state transition for simple 1D case)
    // P_k|k-1 = P_k-1|k-1 + Q
    m_covariance += m_processNoise;

    // Update step
    // K_k = P_k|k-1 / (P_k|k-1 + R)
    T kalmanGain = m_covariance / (m_covariance + m_measurementNoise);

    // x_k|k = x_k|k-1 + K_k * (z_k - x_k|k-1)
    m_estimate = m_estimate + kalmanGain * (measurement - m_estimate);

    // P_k|k = (1 - K_k) * P_k|k-1
    m_covariance = (1 - kalmanGain) * m_covariance;

    return m_estimate;
}

template<typename T>
T KalmanFilter<T>::predict() {
    // For simple 1D case, prediction is just the current estimate
    // In more complex cases, this would apply state transition model
    return m_estimate;
}

template<typename T>
void KalmanFilter<T>::reset() {
    m_estimate = m_initialEstimate;
    m_covariance = m_initialCovariance;
}

template<typename T>
T KalmanFilter<T>::getEstimate() const {
    return m_estimate;
}

template<typename T>
T KalmanFilter<T>::getCovariance() const {
    return m_covariance;
}

//=============================================================================
// Factory Functions
//=============================================================================

template<typename T>
std::unique_ptr<Filter<T>> createFilter(FilterType type, T cutoffFreq, uint32_t order) {
    switch (type) {
        case FilterType::LowPass:
            return std::make_unique<LowPassFilter<T>>(cutoffFreq, order);
        case FilterType::HighPass:
            return std::make_unique<HighPassFilter<T>>(cutoffFreq, order);
        default:
            FMUS_LOG_ERROR("Unsupported filter type in createFilter");
            return nullptr;
    }
}

template<typename T>
std::unique_ptr<Filter<T>> createBandPassFilter(T lowCutoff, T highCutoff, uint32_t order) {
    return std::make_unique<BandPassFilter<T>>(lowCutoff, highCutoff, order);
}

//=============================================================================
// Helper Functions
//=============================================================================

std::string filterTypeToString(FilterType type) {
    switch (type) {
        case FilterType::LowPass: return "Low-Pass";
        case FilterType::HighPass: return "High-Pass";
        case FilterType::BandPass: return "Band-Pass";
        case FilterType::BandStop: return "Band-Stop";
        case FilterType::AllPass: return "All-Pass";
        default: return "Unknown";
    }
}

std::string filterImplementationToString(FilterImplementation impl) {
    switch (impl) {
        case FilterImplementation::FIR: return "FIR";
        case FilterImplementation::IIR: return "IIR";
        case FilterImplementation::Butterworth: return "Butterworth";
        case FilterImplementation::Chebyshev1: return "Chebyshev Type I";
        case FilterImplementation::Chebyshev2: return "Chebyshev Type II";
        case FilterImplementation::Elliptic: return "Elliptic";
        default: return "Unknown";
    }
}

//=============================================================================
// Explicit Template Instantiations
//=============================================================================

template class LowPassFilter<float>;
template class LowPassFilter<double>;
template class HighPassFilter<float>;
template class HighPassFilter<double>;
template class BandPassFilter<float>;
template class BandPassFilter<double>;
template class MovingAverageFilter<float>;
template class MovingAverageFilter<double>;
template class MedianFilter<float>;
template class MedianFilter<double>;
template class KalmanFilter<float>;
template class KalmanFilter<double>;

template std::unique_ptr<Filter<float>> createFilter<float>(FilterType, float, uint32_t);
template std::unique_ptr<Filter<double>> createFilter<double>(FilterType, double, uint32_t);
template std::unique_ptr<Filter<float>> createBandPassFilter<float>(float, float, uint32_t);
template std::unique_ptr<Filter<double>> createBandPassFilter<double>(double, double, uint32_t);

} // namespace dsp
} // namespace fmus
