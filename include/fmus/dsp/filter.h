#pragma once

/**
 * @file filter.h
 * @brief Digital signal processing filters for the fmus-embed library
 *
 * This header provides comprehensive digital filtering capabilities including
 * FIR, IIR, and specialized filters for embedded signal processing applications.
 */

#include "../fmus_config.h"
#include "../core/result.h"
#include <vector>
#include <cstdint>
#include <memory>
#include <complex>

namespace fmus {
namespace dsp {

/**
 * @brief Filter types enumeration
 */
enum class FilterType : uint8_t {
    LowPass = 0,    ///< Low-pass filter
    HighPass = 1,   ///< High-pass filter
    BandPass = 2,   ///< Band-pass filter
    BandStop = 3,   ///< Band-stop (notch) filter
    AllPass = 4     ///< All-pass filter
};

/**
 * @brief Filter implementation types
 */
enum class FilterImplementation : uint8_t {
    FIR = 0,        ///< Finite Impulse Response
    IIR = 1,        ///< Infinite Impulse Response
    Butterworth = 2, ///< Butterworth IIR
    Chebyshev1 = 3, ///< Chebyshev Type I
    Chebyshev2 = 4, ///< Chebyshev Type II
    Elliptic = 5    ///< Elliptic (Cauer)
};

/**
 * @brief Base filter interface
 */
template<typename T>
class FMUS_EMBED_API Filter {
public:
    virtual ~Filter() = default;

    /**
     * @brief Process a single sample
     *
     * @param input Input sample
     * @return T Filtered output sample
     */
    virtual T process(T input) = 0;

    /**
     * @brief Process a vector of samples
     *
     * @param input Input samples
     * @return std::vector<T> Filtered output samples
     */
    virtual std::vector<T> process(const std::vector<T>& input) = 0;

    /**
     * @brief Reset filter state
     */
    virtual void reset() = 0;

    /**
     * @brief Get filter type
     *
     * @return FilterType The filter type
     */
    virtual FilterType getType() const = 0;

    /**
     * @brief Get filter implementation
     *
     * @return FilterImplementation The implementation type
     */
    virtual FilterImplementation getImplementation() const = 0;

    /**
     * @brief Get filter order
     *
     * @return uint32_t Filter order
     */
    virtual uint32_t getOrder() const = 0;
};

/**
 * @brief Low-pass filter
 */
template<typename T>
class FMUS_EMBED_API LowPassFilter : public Filter<T> {
public:
    /**
     * @brief Construct a simple RC low-pass filter
     *
     * @param alpha Filter coefficient (0 < alpha < 1)
     */
    explicit LowPassFilter(T alpha);

    /**
     * @brief Construct a Butterworth low-pass filter
     *
     * @param cutoffFreq Cutoff frequency (normalized, 0-1)
     * @param order Filter order
     */
    LowPassFilter(T cutoffFreq, uint32_t order);

    ~LowPassFilter() override;

    T process(T input) override;
    std::vector<T> process(const std::vector<T>& input) override;
    void reset() override;
    FilterType getType() const override { return FilterType::LowPass; }
    FilterImplementation getImplementation() const override;
    uint32_t getOrder() const override;

    /**
     * @brief Set filter coefficient
     *
     * @param alpha New coefficient
     */
    void setAlpha(T alpha);

    /**
     * @brief Get current coefficient
     *
     * @return T Current coefficient
     */
    T getAlpha() const;

private:
    T m_alpha;
    T m_previousOutput;
    uint32_t m_order;
    FilterImplementation m_implementation;
    void* m_impl; ///< Implementation details
};

/**
 * @brief High-pass filter
 */
template<typename T>
class FMUS_EMBED_API HighPassFilter : public Filter<T> {
public:
    /**
     * @brief Construct a simple RC high-pass filter
     *
     * @param alpha Filter coefficient (0 < alpha < 1)
     */
    explicit HighPassFilter(T alpha);

    /**
     * @brief Construct a Butterworth high-pass filter
     *
     * @param cutoffFreq Cutoff frequency (normalized, 0-1)
     * @param order Filter order
     */
    HighPassFilter(T cutoffFreq, uint32_t order);

    ~HighPassFilter() override;

    T process(T input) override;
    std::vector<T> process(const std::vector<T>& input) override;
    void reset() override;
    FilterType getType() const override { return FilterType::HighPass; }
    FilterImplementation getImplementation() const override;
    uint32_t getOrder() const override;

private:
    T m_alpha;
    T m_previousInput;
    T m_previousOutput;
    uint32_t m_order;
    FilterImplementation m_implementation;
    void* m_impl;
};

/**
 * @brief Band-pass filter
 */
template<typename T>
class FMUS_EMBED_API BandPassFilter : public Filter<T> {
public:
    /**
     * @brief Construct a band-pass filter
     *
     * @param lowCutoff Low cutoff frequency (normalized, 0-1)
     * @param highCutoff High cutoff frequency (normalized, 0-1)
     * @param order Filter order
     */
    BandPassFilter(T lowCutoff, T highCutoff, uint32_t order = 2);

    ~BandPassFilter() override;

    T process(T input) override;
    std::vector<T> process(const std::vector<T>& input) override;
    void reset() override;
    FilterType getType() const override { return FilterType::BandPass; }
    FilterImplementation getImplementation() const override;
    uint32_t getOrder() const override;

private:
    T m_lowCutoff;
    T m_highCutoff;
    uint32_t m_order;
    std::unique_ptr<LowPassFilter<T>> m_lowPass;
    std::unique_ptr<HighPassFilter<T>> m_highPass;
};

/**
 * @brief Moving average filter
 */
template<typename T>
class FMUS_EMBED_API MovingAverageFilter : public Filter<T> {
public:
    /**
     * @brief Construct a moving average filter
     *
     * @param windowSize Size of the averaging window
     */
    explicit MovingAverageFilter(uint32_t windowSize);

    ~MovingAverageFilter() override;

    T process(T input) override;
    std::vector<T> process(const std::vector<T>& input) override;
    void reset() override;
    FilterType getType() const override { return FilterType::LowPass; }
    FilterImplementation getImplementation() const override { return FilterImplementation::FIR; }
    uint32_t getOrder() const override;

    /**
     * @brief Set window size
     *
     * @param windowSize New window size
     */
    void setWindowSize(uint32_t windowSize);

    /**
     * @brief Get current window size
     *
     * @return uint32_t Current window size
     */
    uint32_t getWindowSize() const;

private:
    uint32_t m_windowSize;
    std::vector<T> m_buffer;
    uint32_t m_index;
    T m_sum;
    bool m_bufferFull;
};

/**
 * @brief Median filter
 */
template<typename T>
class FMUS_EMBED_API MedianFilter : public Filter<T> {
public:
    /**
     * @brief Construct a median filter
     *
     * @param windowSize Size of the median window (should be odd)
     */
    explicit MedianFilter(uint32_t windowSize);

    ~MedianFilter() override;

    T process(T input) override;
    std::vector<T> process(const std::vector<T>& input) override;
    void reset() override;
    FilterType getType() const override { return FilterType::LowPass; }
    FilterImplementation getImplementation() const override { return FilterImplementation::FIR; }
    uint32_t getOrder() const override;

private:
    uint32_t m_windowSize;
    std::vector<T> m_buffer;
    uint32_t m_index;
    bool m_bufferFull;

    T calculateMedian();
};

/**
 * @brief Kalman filter for state estimation
 */
template<typename T>
class FMUS_EMBED_API KalmanFilter {
public:
    /**
     * @brief Construct a simple 1D Kalman filter
     *
     * @param processNoise Process noise covariance
     * @param measurementNoise Measurement noise covariance
     * @param initialEstimate Initial state estimate
     * @param initialCovariance Initial error covariance
     */
    KalmanFilter(T processNoise, T measurementNoise, T initialEstimate = 0, T initialCovariance = 1);

    /**
     * @brief Update filter with new measurement
     *
     * @param measurement New measurement
     * @return T Filtered estimate
     */
    T update(T measurement);

    /**
     * @brief Predict next state (without measurement)
     *
     * @return T Predicted state
     */
    T predict();

    /**
     * @brief Reset filter state
     */
    void reset();

    /**
     * @brief Get current estimate
     *
     * @return T Current state estimate
     */
    T getEstimate() const;

    /**
     * @brief Get current covariance
     *
     * @return T Current error covariance
     */
    T getCovariance() const;

private:
    T m_processNoise;      ///< Process noise covariance (Q)
    T m_measurementNoise;  ///< Measurement noise covariance (R)
    T m_estimate;          ///< Current state estimate
    T m_covariance;        ///< Current error covariance
    T m_initialEstimate;   ///< Initial estimate
    T m_initialCovariance; ///< Initial covariance
};

/**
 * @brief Create a filter of specified type
 *
 * @tparam T Data type (float, double)
 * @param type Filter type
 * @param cutoffFreq Cutoff frequency (normalized, 0-1)
 * @param order Filter order
 * @return std::unique_ptr<Filter<T>> Filter instance
 */
template<typename T>
FMUS_EMBED_API std::unique_ptr<Filter<T>> createFilter(FilterType type, T cutoffFreq, uint32_t order = 2);

/**
 * @brief Create a band-pass filter
 *
 * @tparam T Data type
 * @param lowCutoff Low cutoff frequency
 * @param highCutoff High cutoff frequency
 * @param order Filter order
 * @return std::unique_ptr<Filter<T>> Filter instance
 */
template<typename T>
FMUS_EMBED_API std::unique_ptr<Filter<T>> createBandPassFilter(T lowCutoff, T highCutoff, uint32_t order = 2);

/**
 * @brief Get string representation of filter type
 *
 * @param type Filter type
 * @return std::string String representation
 */
FMUS_EMBED_API std::string filterTypeToString(FilterType type);

/**
 * @brief Get string representation of filter implementation
 *
 * @param impl Filter implementation
 * @return std::string String representation
 */
FMUS_EMBED_API std::string filterImplementationToString(FilterImplementation impl);

// Explicit template instantiations
extern template class FMUS_EMBED_API LowPassFilter<float>;
extern template class FMUS_EMBED_API LowPassFilter<double>;
extern template class FMUS_EMBED_API HighPassFilter<float>;
extern template class FMUS_EMBED_API HighPassFilter<double>;
extern template class FMUS_EMBED_API MovingAverageFilter<float>;
extern template class FMUS_EMBED_API MovingAverageFilter<double>;
extern template class FMUS_EMBED_API MedianFilter<float>;
extern template class FMUS_EMBED_API MedianFilter<double>;
extern template class FMUS_EMBED_API KalmanFilter<float>;
extern template class FMUS_EMBED_API KalmanFilter<double>;

} // namespace dsp
} // namespace fmus
