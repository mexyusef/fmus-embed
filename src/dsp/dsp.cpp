#include "fmus/dsp/dsp.h"
#include "fmus/core/logging.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <random>
#include <sstream>

namespace fmus {
namespace dsp {

// Global state for DSP module
static bool g_dspInitialized = false;

core::Result<void> initDSP() {
    if (g_dspInitialized) {
        return core::makeOk(); // Already initialized
    }

    FMUS_LOG_INFO("Initializing DSP module");

    // Initialize any global DSP resources here
    // For now, just mark as initialized
    g_dspInitialized = true;

    FMUS_LOG_INFO("DSP module initialized successfully");
    return core::makeOk();
}

core::Result<void> shutdownDSP() {
    if (!g_dspInitialized) {
        return core::makeOk(); // Already shutdown
    }

    FMUS_LOG_INFO("Shutting down DSP module");

    // Clean up any global DSP resources here
    g_dspInitialized = false;

    FMUS_LOG_INFO("DSP module shutdown completed");
    return core::makeOk();
}

bool isDSPInitialized() {
    return g_dspInitialized;
}

//=============================================================================
// Signal Statistics Implementation
//=============================================================================

template<typename T>
SignalStats<T> calculateSignalStats(const std::vector<T>& signal) {
    SignalStats<T> stats = {};
    
    if (signal.empty()) {
        return stats;
    }
    
    stats.length = static_cast<uint32_t>(signal.size());
    
    // Calculate mean
    stats.mean = std::accumulate(signal.begin(), signal.end(), static_cast<T>(0)) / signal.size();
    
    // Find min and max
    auto minmax = std::minmax_element(signal.begin(), signal.end());
    stats.min = *minmax.first;
    stats.max = *minmax.second;
    stats.peakToPeak = stats.max - stats.min;
    stats.peak = std::max(std::abs(stats.min), std::abs(stats.max));
    
    // Calculate variance and standard deviation
    T sumSquaredDiff = 0;
    T sumSquares = 0;
    
    for (const T& sample : signal) {
        T diff = sample - stats.mean;
        sumSquaredDiff += diff * diff;
        sumSquares += sample * sample;
    }
    
    stats.variance = sumSquaredDiff / signal.size();
    stats.stdDev = std::sqrt(stats.variance);
    
    // Calculate RMS
    stats.rms = std::sqrt(sumSquares / signal.size());
    
    // Calculate crest factor
    stats.crestFactor = (stats.rms > 0) ? stats.peak / stats.rms : 0;
    
    return stats;
}

//=============================================================================
// Correlation Functions
//=============================================================================

template<typename T>
std::vector<T> crossCorrelation(const std::vector<T>& signal1, const std::vector<T>& signal2) {
    if (signal1.empty() || signal2.empty()) {
        return {};
    }
    
    size_t n1 = signal1.size();
    size_t n2 = signal2.size();
    size_t resultSize = n1 + n2 - 1;
    
    std::vector<T> result(resultSize, 0);
    
    for (size_t i = 0; i < resultSize; ++i) {
        for (size_t j = 0; j < n1; ++j) {
            if (i >= j && (i - j) < n2) {
                result[i] += signal1[j] * signal2[i - j];
            }
        }
    }
    
    return result;
}

template<typename T>
std::vector<T> autoCorrelation(const std::vector<T>& signal) {
    return crossCorrelation(signal, signal);
}

//=============================================================================
// Resampling Functions
//=============================================================================

template<typename T>
core::Result<std::vector<T>> resample(const std::vector<T>& signal, T originalRate, T targetRate) {
    if (signal.empty()) {
        return core::makeError<std::vector<T>>(core::ErrorCode::InvalidArgument, "Input signal is empty");
    }
    
    if (originalRate <= 0 || targetRate <= 0) {
        return core::makeError<std::vector<T>>(core::ErrorCode::InvalidArgument, "Sample rates must be positive");
    }
    
    T ratio = targetRate / originalRate;
    size_t newSize = static_cast<size_t>(signal.size() * ratio);
    
    std::vector<T> resampled;
    resampled.reserve(newSize);
    
    for (size_t i = 0; i < newSize; ++i) {
        T sourceIndex = static_cast<T>(i) / ratio;
        size_t index1 = static_cast<size_t>(sourceIndex);
        size_t index2 = std::min(index1 + 1, signal.size() - 1);
        
        if (index1 >= signal.size()) {
            break;
        }
        
        // Linear interpolation
        T fraction = sourceIndex - index1;
        T interpolated = signal[index1] * (1 - fraction) + signal[index2] * fraction;
        resampled.push_back(interpolated);
    }
    
    return core::makeOk<std::vector<T>>(std::move(resampled));
}

template<typename T>
std::vector<T> decimate(const std::vector<T>& signal, uint32_t factor, bool useFilter) {
    if (signal.empty() || factor == 0) {
        return {};
    }
    
    std::vector<T> decimated;
    decimated.reserve(signal.size() / factor);
    
    if (useFilter && factor > 1) {
        // Apply anti-aliasing filter before decimation
        LowPassFilter<T> antiAlias(static_cast<T>(0.4) / factor);
        auto filtered = antiAlias.process(signal);
        
        for (size_t i = 0; i < filtered.size(); i += factor) {
            decimated.push_back(filtered[i]);
        }
    } else {
        for (size_t i = 0; i < signal.size(); i += factor) {
            decimated.push_back(signal[i]);
        }
    }
    
    return decimated;
}

template<typename T>
std::vector<T> interpolate(const std::vector<T>& signal, uint32_t factor, bool useFilter) {
    if (signal.empty() || factor == 0) {
        return {};
    }
    
    std::vector<T> interpolated;
    interpolated.reserve(signal.size() * factor);
    
    // Zero-stuff (insert zeros between samples)
    for (const T& sample : signal) {
        interpolated.push_back(sample);
        for (uint32_t i = 1; i < factor; ++i) {
            interpolated.push_back(0);
        }
    }
    
    if (useFilter && factor > 1) {
        // Apply anti-imaging filter after interpolation
        LowPassFilter<T> antiImage(static_cast<T>(0.4) / factor);
        interpolated = antiImage.process(interpolated);
        
        // Scale by interpolation factor
        for (T& sample : interpolated) {
            sample *= factor;
        }
    }
    
    return interpolated;
}

//=============================================================================
// SignalGenerator Implementation
//=============================================================================

template<typename T>
std::vector<T> SignalGenerator::sine(T frequency, T amplitude, T sampleRate, T duration, T phase) {
    uint32_t numSamples = static_cast<uint32_t>(duration * sampleRate);
    std::vector<T> signal;
    signal.reserve(numSamples);
    
    T omega = 2 * M_PI * frequency / sampleRate;
    
    for (uint32_t i = 0; i < numSamples; ++i) {
        T sample = amplitude * std::sin(omega * i + phase);
        signal.push_back(sample);
    }
    
    return signal;
}

template<typename T>
std::vector<T> SignalGenerator::cosine(T frequency, T amplitude, T sampleRate, T duration, T phase) {
    uint32_t numSamples = static_cast<uint32_t>(duration * sampleRate);
    std::vector<T> signal;
    signal.reserve(numSamples);
    
    T omega = 2 * M_PI * frequency / sampleRate;
    
    for (uint32_t i = 0; i < numSamples; ++i) {
        T sample = amplitude * std::cos(omega * i + phase);
        signal.push_back(sample);
    }
    
    return signal;
}

template<typename T>
std::vector<T> SignalGenerator::square(T frequency, T amplitude, T sampleRate, T duration, T dutyCycle) {
    uint32_t numSamples = static_cast<uint32_t>(duration * sampleRate);
    std::vector<T> signal;
    signal.reserve(numSamples);
    
    T period = sampleRate / frequency;
    T highTime = period * dutyCycle;
    
    for (uint32_t i = 0; i < numSamples; ++i) {
        T phase = std::fmod(static_cast<T>(i), period);
        T sample = (phase < highTime) ? amplitude : -amplitude;
        signal.push_back(sample);
    }
    
    return signal;
}

template<typename T>
std::vector<T> SignalGenerator::sawtooth(T frequency, T amplitude, T sampleRate, T duration) {
    uint32_t numSamples = static_cast<uint32_t>(duration * sampleRate);
    std::vector<T> signal;
    signal.reserve(numSamples);
    
    T period = sampleRate / frequency;
    
    for (uint32_t i = 0; i < numSamples; ++i) {
        T phase = std::fmod(static_cast<T>(i), period);
        T sample = amplitude * (2 * phase / period - 1);
        signal.push_back(sample);
    }
    
    return signal;
}

template<typename T>
std::vector<T> SignalGenerator::triangle(T frequency, T amplitude, T sampleRate, T duration) {
    uint32_t numSamples = static_cast<uint32_t>(duration * sampleRate);
    std::vector<T> signal;
    signal.reserve(numSamples);
    
    T period = sampleRate / frequency;
    T halfPeriod = period / 2;
    
    for (uint32_t i = 0; i < numSamples; ++i) {
        T phase = std::fmod(static_cast<T>(i), period);
        T sample;
        
        if (phase < halfPeriod) {
            sample = amplitude * (2 * phase / halfPeriod - 1);
        } else {
            sample = amplitude * (3 - 2 * phase / halfPeriod);
        }
        
        signal.push_back(sample);
    }
    
    return signal;
}

template<typename T>
std::vector<T> SignalGenerator::whiteNoise(T amplitude, T sampleRate, T duration, uint32_t seed) {
    uint32_t numSamples = static_cast<uint32_t>(duration * sampleRate);
    std::vector<T> signal;
    signal.reserve(numSamples);
    
    std::mt19937 generator(seed == 0 ? std::random_device{}() : seed);
    std::normal_distribution<T> distribution(0, amplitude);
    
    for (uint32_t i = 0; i < numSamples; ++i) {
        signal.push_back(distribution(generator));
    }
    
    return signal;
}

template<typename T>
std::vector<T> SignalGenerator::chirp(T startFreq, T endFreq, T amplitude, T sampleRate, T duration) {
    uint32_t numSamples = static_cast<uint32_t>(duration * sampleRate);
    std::vector<T> signal;
    signal.reserve(numSamples);
    
    T k = (endFreq - startFreq) / duration; // Frequency sweep rate
    
    for (uint32_t i = 0; i < numSamples; ++i) {
        T t = static_cast<T>(i) / sampleRate;
        T instantFreq = startFreq + k * t;
        T phase = 2 * M_PI * (startFreq * t + 0.5 * k * t * t);
        T sample = amplitude * std::sin(phase);
        signal.push_back(sample);
    }
    
    return signal;
}

//=============================================================================
// RealTimeProcessor Implementation
//=============================================================================

template<typename T>
RealTimeProcessor<T>::RealTimeProcessor(uint32_t bufferSize, T sampleRate)
    : m_bufferSize(bufferSize), m_sampleRate(sampleRate), m_latency(0) {
}

template<typename T>
RealTimeProcessor<T>::~RealTimeProcessor() = default;

template<typename T>
core::Result<void> RealTimeProcessor<T>::addFilter(std::shared_ptr<Filter<T>> filter) {
    if (!filter) {
        return core::makeError<void>(core::ErrorCode::InvalidArgument, "Filter pointer is null");
    }

    m_filters.push_back(filter);
    m_latency += filter->getOrder();

    return core::makeOk();
}

template<typename T>
void RealTimeProcessor<T>::clearFilters() {
    m_filters.clear();
    m_latency = 0;
}

template<typename T>
T RealTimeProcessor<T>::processSample(T input) {
    T output = input;

    for (auto& filter : m_filters) {
        output = filter->process(output);
    }

    return output;
}

template<typename T>
std::vector<T> RealTimeProcessor<T>::processBuffer(const std::vector<T>& input) {
    std::vector<T> output;
    output.reserve(input.size());

    for (const T& sample : input) {
        output.push_back(processSample(sample));
    }

    return output;
}

template<typename T>
void RealTimeProcessor<T>::reset() {
    for (auto& filter : m_filters) {
        filter->reset();
    }
}

template<typename T>
uint32_t RealTimeProcessor<T>::getLatency() const {
    return m_latency;
}

std::string getDSPStatus() {
    std::ostringstream oss;
    oss << "DSP Module Status:\n";
    oss << "  Initialized: " << (g_dspInitialized ? "Yes" : "No") << "\n";
    oss << "  Available Filters: Low-pass, High-pass, Band-pass, Moving Average, Median, Kalman\n";
    oss << "  FFT Support: Radix-2, Real/Complex, Forward/Inverse\n";
    oss << "  Window Functions: Hanning, Hamming, Blackman, Kaiser, Gaussian, Tukey\n";
    oss << "  Signal Generation: Sine, Cosine, Square, Sawtooth, Triangle, White Noise, Chirp\n";
    oss << "  Analysis Tools: Spectral analysis, Peak detection, THD, SNR, Centroid";
    return oss.str();
}

//=============================================================================
// Explicit Template Instantiations
//=============================================================================

template SignalStats<float> calculateSignalStats<float>(const std::vector<float>&);
template SignalStats<double> calculateSignalStats<double>(const std::vector<double>&);

template std::vector<float> crossCorrelation<float>(const std::vector<float>&, const std::vector<float>&);
template std::vector<double> crossCorrelation<double>(const std::vector<double>&, const std::vector<double>&);
template std::vector<float> autoCorrelation<float>(const std::vector<float>&);
template std::vector<double> autoCorrelation<double>(const std::vector<double>&);

template core::Result<std::vector<float>> resample<float>(const std::vector<float>&, float, float);
template core::Result<std::vector<double>> resample<double>(const std::vector<double>&, double, double);
template std::vector<float> decimate<float>(const std::vector<float>&, uint32_t, bool);
template std::vector<double> decimate<double>(const std::vector<double>&, uint32_t, bool);
template std::vector<float> interpolate<float>(const std::vector<float>&, uint32_t, bool);
template std::vector<double> interpolate<double>(const std::vector<double>&, uint32_t, bool);

template std::vector<float> SignalGenerator::sine<float>(float, float, float, float, float);
template std::vector<double> SignalGenerator::sine<double>(double, double, double, double, double);
template std::vector<float> SignalGenerator::cosine<float>(float, float, float, float, float);
template std::vector<double> SignalGenerator::cosine<double>(double, double, double, double, double);
template std::vector<float> SignalGenerator::square<float>(float, float, float, float, float);
template std::vector<double> SignalGenerator::square<double>(double, double, double, double, double);
template std::vector<float> SignalGenerator::sawtooth<float>(float, float, float, float);
template std::vector<double> SignalGenerator::sawtooth<double>(double, double, double, double);
template std::vector<float> SignalGenerator::triangle<float>(float, float, float, float);
template std::vector<double> SignalGenerator::triangle<double>(double, double, double, double);
template std::vector<float> SignalGenerator::whiteNoise<float>(float, float, float, uint32_t);
template std::vector<double> SignalGenerator::whiteNoise<double>(double, double, double, uint32_t);
template std::vector<float> SignalGenerator::chirp<float>(float, float, float, float, float);
template std::vector<double> SignalGenerator::chirp<double>(double, double, double, double, double);

template class RealTimeProcessor<float>;
template class RealTimeProcessor<double>;

} // namespace dsp
} // namespace fmus
