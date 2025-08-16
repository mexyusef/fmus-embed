#pragma once

/**
 * @file dsp.h
 * @brief Main DSP header for the fmus-embed library
 *
 * This header includes all digital signal processing functionality
 * including filters, FFT, and signal analysis tools.
 */

#include "filter.h"
#include "fft.h"
#include "../core/result.h"
#include <vector>
#include <cstdint>

namespace fmus {
namespace dsp {

/**
 * @brief Initialize the DSP module
 *
 * @return core::Result<void> Success or error
 */
FMUS_EMBED_API core::Result<void> initDSP();

/**
 * @brief Shutdown the DSP module
 *
 * @return core::Result<void> Success or error
 */
FMUS_EMBED_API core::Result<void> shutdownDSP();

/**
 * @brief Check if DSP module is initialized
 *
 * @return bool True if initialized
 */
FMUS_EMBED_API bool isDSPInitialized();

/**
 * @brief Signal statistics calculation
 */
template<typename T>
struct SignalStats {
    T mean;         ///< Mean value
    T variance;     ///< Variance
    T stdDev;       ///< Standard deviation
    T rms;          ///< RMS value
    T min;          ///< Minimum value
    T max;          ///< Maximum value
    T peak;         ///< Peak value (max absolute)
    T peakToPeak;   ///< Peak-to-peak value
    T crestFactor;  ///< Crest factor (peak/RMS)
    uint32_t length; ///< Signal length
};

/**
 * @brief Calculate comprehensive signal statistics
 *
 * @tparam T Data type
 * @param signal Input signal
 * @return SignalStats<T> Calculated statistics
 */
template<typename T>
FMUS_EMBED_API SignalStats<T> calculateSignalStats(const std::vector<T>& signal);

/**
 * @brief Calculate cross-correlation between two signals
 *
 * @tparam T Data type
 * @param signal1 First signal
 * @param signal2 Second signal
 * @return std::vector<T> Cross-correlation result
 */
template<typename T>
FMUS_EMBED_API std::vector<T> crossCorrelation(const std::vector<T>& signal1, 
                                               const std::vector<T>& signal2);

/**
 * @brief Calculate auto-correlation of a signal
 *
 * @tparam T Data type
 * @param signal Input signal
 * @return std::vector<T> Auto-correlation result
 */
template<typename T>
FMUS_EMBED_API std::vector<T> autoCorrelation(const std::vector<T>& signal);

/**
 * @brief Resample signal to new sample rate
 *
 * @tparam T Data type
 * @param signal Input signal
 * @param originalRate Original sample rate
 * @param targetRate Target sample rate
 * @return core::Result<std::vector<T>> Resampled signal or error
 */
template<typename T>
FMUS_EMBED_API core::Result<std::vector<T>> resample(const std::vector<T>& signal, 
                                                     T originalRate, 
                                                     T targetRate);

/**
 * @brief Decimate signal by integer factor
 *
 * @tparam T Data type
 * @param signal Input signal
 * @param factor Decimation factor
 * @param useFilter Apply anti-aliasing filter
 * @return std::vector<T> Decimated signal
 */
template<typename T>
FMUS_EMBED_API std::vector<T> decimate(const std::vector<T>& signal, 
                                       uint32_t factor, 
                                       bool useFilter = true);

/**
 * @brief Interpolate signal by integer factor
 *
 * @tparam T Data type
 * @param signal Input signal
 * @param factor Interpolation factor
 * @param useFilter Apply anti-imaging filter
 * @return std::vector<T> Interpolated signal
 */
template<typename T>
FMUS_EMBED_API std::vector<T> interpolate(const std::vector<T>& signal, 
                                          uint32_t factor, 
                                          bool useFilter = true);

/**
 * @brief Generate test signals for DSP development
 */
class FMUS_EMBED_API SignalGenerator {
public:
    /**
     * @brief Generate sine wave
     *
     * @tparam T Data type
     * @param frequency Frequency in Hz
     * @param amplitude Amplitude
     * @param sampleRate Sample rate in Hz
     * @param duration Duration in seconds
     * @param phase Phase offset in radians
     * @return std::vector<T> Generated sine wave
     */
    template<typename T>
    static std::vector<T> sine(T frequency, T amplitude, T sampleRate, T duration, T phase = 0);

    /**
     * @brief Generate cosine wave
     *
     * @tparam T Data type
     * @param frequency Frequency in Hz
     * @param amplitude Amplitude
     * @param sampleRate Sample rate in Hz
     * @param duration Duration in seconds
     * @param phase Phase offset in radians
     * @return std::vector<T> Generated cosine wave
     */
    template<typename T>
    static std::vector<T> cosine(T frequency, T amplitude, T sampleRate, T duration, T phase = 0);

    /**
     * @brief Generate square wave
     *
     * @tparam T Data type
     * @param frequency Frequency in Hz
     * @param amplitude Amplitude
     * @param sampleRate Sample rate in Hz
     * @param duration Duration in seconds
     * @param dutyCycle Duty cycle (0.0 to 1.0)
     * @return std::vector<T> Generated square wave
     */
    template<typename T>
    static std::vector<T> square(T frequency, T amplitude, T sampleRate, T duration, T dutyCycle = 0.5);

    /**
     * @brief Generate sawtooth wave
     *
     * @tparam T Data type
     * @param frequency Frequency in Hz
     * @param amplitude Amplitude
     * @param sampleRate Sample rate in Hz
     * @param duration Duration in seconds
     * @return std::vector<T> Generated sawtooth wave
     */
    template<typename T>
    static std::vector<T> sawtooth(T frequency, T amplitude, T sampleRate, T duration);

    /**
     * @brief Generate triangle wave
     *
     * @tparam T Data type
     * @param frequency Frequency in Hz
     * @param amplitude Amplitude
     * @param sampleRate Sample rate in Hz
     * @param duration Duration in seconds
     * @return std::vector<T> Generated triangle wave
     */
    template<typename T>
    static std::vector<T> triangle(T frequency, T amplitude, T sampleRate, T duration);

    /**
     * @brief Generate white noise
     *
     * @tparam T Data type
     * @param amplitude Amplitude
     * @param sampleRate Sample rate in Hz
     * @param duration Duration in seconds
     * @param seed Random seed
     * @return std::vector<T> Generated white noise
     */
    template<typename T>
    static std::vector<T> whiteNoise(T amplitude, T sampleRate, T duration, uint32_t seed = 0);

    /**
     * @brief Generate chirp signal (frequency sweep)
     *
     * @tparam T Data type
     * @param startFreq Start frequency in Hz
     * @param endFreq End frequency in Hz
     * @param amplitude Amplitude
     * @param sampleRate Sample rate in Hz
     * @param duration Duration in seconds
     * @return std::vector<T> Generated chirp signal
     */
    template<typename T>
    static std::vector<T> chirp(T startFreq, T endFreq, T amplitude, T sampleRate, T duration);
};

/**
 * @brief Real-time signal processor for streaming applications
 */
template<typename T>
class FMUS_EMBED_API RealTimeProcessor {
public:
    /**
     * @brief Constructor
     *
     * @param bufferSize Processing buffer size
     * @param sampleRate Sample rate in Hz
     */
    RealTimeProcessor(uint32_t bufferSize, T sampleRate);

    /**
     * @brief Destructor
     */
    ~RealTimeProcessor();

    /**
     * @brief Add filter to processing chain
     *
     * @param filter Filter to add
     * @return core::Result<void> Success or error
     */
    core::Result<void> addFilter(std::shared_ptr<Filter<T>> filter);

    /**
     * @brief Remove all filters
     */
    void clearFilters();

    /**
     * @brief Process single sample
     *
     * @param input Input sample
     * @return T Processed output sample
     */
    T processSample(T input);

    /**
     * @brief Process buffer of samples
     *
     * @param input Input samples
     * @return std::vector<T> Processed output samples
     */
    std::vector<T> processBuffer(const std::vector<T>& input);

    /**
     * @brief Reset all filters
     */
    void reset();

    /**
     * @brief Get current latency in samples
     *
     * @return uint32_t Latency in samples
     */
    uint32_t getLatency() const;

private:
    uint32_t m_bufferSize;
    T m_sampleRate;
    std::vector<std::shared_ptr<Filter<T>>> m_filters;
    uint32_t m_latency;
};

/**
 * @brief Get DSP module status
 *
 * @return std::string Status information
 */
FMUS_EMBED_API std::string getDSPStatus();

// Explicit template instantiations
extern template struct FMUS_EMBED_API SignalStats<float>;
extern template struct FMUS_EMBED_API SignalStats<double>;
extern template class FMUS_EMBED_API RealTimeProcessor<float>;
extern template class FMUS_EMBED_API RealTimeProcessor<double>;

} // namespace dsp
} // namespace fmus
