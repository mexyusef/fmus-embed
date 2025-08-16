#include "fmus/dsp/fft.h"
#include "fmus/core/logging.h"
#include <cmath>
#include <algorithm>
#include <numeric>

namespace fmus {
namespace dsp {

//=============================================================================
// FFTResult Implementation
//=============================================================================

template<typename T>
std::vector<T> FFTResult<T>::getMagnitude() const {
    std::vector<T> magnitude;
    magnitude.reserve(data.size());
    
    for (const auto& sample : data) {
        magnitude.push_back(std::abs(sample));
    }
    
    return magnitude;
}

template<typename T>
std::vector<T> FFTResult<T>::getPhase() const {
    std::vector<T> phase;
    phase.reserve(data.size());
    
    for (const auto& sample : data) {
        phase.push_back(std::arg(sample));
    }
    
    return phase;
}

template<typename T>
std::vector<T> FFTResult<T>::getPower() const {
    std::vector<T> power;
    power.reserve(data.size());
    
    for (const auto& sample : data) {
        T magnitude = std::abs(sample);
        power.push_back(magnitude * magnitude);
    }
    
    return power;
}

template<typename T>
std::vector<T> FFTResult<T>::getPowerSpectralDensity() const {
    auto power = getPower();
    T scaleFactor = static_cast<T>(2.0) / (sampleRate * size);
    
    for (auto& p : power) {
        p *= scaleFactor;
    }
    
    // DC and Nyquist components should not be doubled
    if (!power.empty()) {
        power[0] /= 2;
        if (power.size() % 2 == 0) {
            power[power.size() / 2] /= 2;
        }
    }
    
    return power;
}

template<typename T>
std::vector<T> FFTResult<T>::getFrequencyBins() const {
    std::vector<T> frequencies;
    frequencies.reserve(data.size());
    
    for (uint32_t i = 0; i < data.size(); ++i) {
        T freq = static_cast<T>(i) * frequencyResolution;
        frequencies.push_back(freq);
    }
    
    return frequencies;
}

//=============================================================================
// FFT Implementation
//=============================================================================

template<typename T>
core::Result<FFTResult<T>> FFT::forward(const std::vector<T>& input, T sampleRate, WindowType window) {
    if (input.empty()) {
        return core::makeError<FFTResult<T>>(core::ErrorCode::InvalidArgument, "Input signal is empty");
    }
    
    // Ensure input size is power of 2
    uint32_t fftSize = nextPowerOf2(static_cast<uint32_t>(input.size()));
    std::vector<T> paddedInput = zeroPad(input, fftSize);
    
    // Apply window function
    if (window != WindowType::None) {
        auto windowCoeffs = generateWindow<T>(fftSize, window);
        for (size_t i = 0; i < paddedInput.size(); ++i) {
            paddedInput[i] *= windowCoeffs[i];
        }
    }
    
    // Convert to complex
    std::vector<std::complex<T>> complexData;
    complexData.reserve(fftSize);
    for (const T& sample : paddedInput) {
        complexData.emplace_back(sample, 0);
    }
    
    // Perform FFT
    radix2FFT(complexData, false);
    
    // Create result
    FFTResult<T> result;
    result.data = std::move(complexData);
    result.sampleRate = sampleRate;
    result.frequencyResolution = sampleRate / static_cast<T>(fftSize);
    result.size = fftSize;
    result.windowUsed = window;
    
    return core::makeOk<FFTResult<T>>(std::move(result));
}

template<typename T>
core::Result<FFTResult<T>> FFT::forward(const std::vector<std::complex<T>>& input, T sampleRate, WindowType window) {
    if (input.empty()) {
        return core::makeError<FFTResult<T>>(core::ErrorCode::InvalidArgument, "Input signal is empty");
    }
    
    // Ensure input size is power of 2
    uint32_t fftSize = nextPowerOf2(static_cast<uint32_t>(input.size()));
    std::vector<std::complex<T>> paddedInput = input;
    paddedInput.resize(fftSize, std::complex<T>(0, 0));
    
    // Apply window function to real part (simplified)
    if (window != WindowType::None) {
        auto windowCoeffs = generateWindow<T>(fftSize, window);
        for (size_t i = 0; i < paddedInput.size(); ++i) {
            paddedInput[i] *= windowCoeffs[i];
        }
    }
    
    // Perform FFT
    radix2FFT(paddedInput, false);
    
    // Create result
    FFTResult<T> result;
    result.data = std::move(paddedInput);
    result.sampleRate = sampleRate;
    result.frequencyResolution = sampleRate / static_cast<T>(fftSize);
    result.size = fftSize;
    result.windowUsed = window;
    
    return core::makeOk<FFTResult<T>>(std::move(result));
}

template<typename T>
core::Result<std::vector<T>> FFT::inverse(const std::vector<std::complex<T>>& input) {
    if (input.empty()) {
        return core::makeError<std::vector<T>>(core::ErrorCode::InvalidArgument, "Input is empty");
    }
    
    std::vector<std::complex<T>> data = input;
    radix2FFT(data, true);
    
    // Extract real part
    std::vector<T> result;
    result.reserve(data.size());
    for (const auto& sample : data) {
        result.push_back(sample.real());
    }
    
    return core::makeOk<std::vector<T>>(std::move(result));
}

template<typename T>
core::Result<std::vector<std::complex<T>>> FFT::inverseComplex(const std::vector<std::complex<T>>& input) {
    if (input.empty()) {
        return core::makeError<std::vector<std::complex<T>>>(core::ErrorCode::InvalidArgument, "Input is empty");
    }
    
    std::vector<std::complex<T>> data = input;
    radix2FFT(data, true);
    
    return core::makeOk<std::vector<std::complex<T>>>(std::move(data));
}

bool FFT::isValidSize(uint32_t size) {
    return size > 0 && (size & (size - 1)) == 0;
}

uint32_t FFT::nextPowerOf2(uint32_t n) {
    if (n == 0) return 1;
    
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    
    return n;
}

template<typename T>
std::vector<T> FFT::zeroPad(const std::vector<T>& signal, uint32_t targetSize) {
    std::vector<T> padded = signal;
    padded.resize(targetSize, 0);
    return padded;
}

template<typename T>
std::vector<T> FFT::applyWindow(const std::vector<T>& signal, WindowType window, T parameter) {
    auto windowCoeffs = generateWindow<T>(static_cast<uint32_t>(signal.size()), window, parameter);
    std::vector<T> windowed = signal;
    
    for (size_t i = 0; i < signal.size(); ++i) {
        windowed[i] *= windowCoeffs[i];
    }
    
    return windowed;
}

template<typename T>
std::vector<T> FFT::generateWindow(uint32_t size, WindowType window, T parameter) {
    std::vector<T> coeffs(size);
    
    switch (window) {
        case WindowType::None:
            std::fill(coeffs.begin(), coeffs.end(), 1);
            break;
            
        case WindowType::Hanning:
            for (uint32_t i = 0; i < size; ++i) {
                coeffs[i] = static_cast<T>(0.5) * (1 - std::cos(2 * M_PI * i / (size - 1)));
            }
            break;
            
        case WindowType::Hamming:
            for (uint32_t i = 0; i < size; ++i) {
                coeffs[i] = static_cast<T>(0.54) - static_cast<T>(0.46) * std::cos(2 * M_PI * i / (size - 1));
            }
            break;
            
        case WindowType::Blackman:
            for (uint32_t i = 0; i < size; ++i) {
                T n = static_cast<T>(i) / (size - 1);
                coeffs[i] = static_cast<T>(0.42) - static_cast<T>(0.5) * std::cos(2 * M_PI * n) + 
                           static_cast<T>(0.08) * std::cos(4 * M_PI * n);
            }
            break;
            
        case WindowType::Kaiser:
            // Simplified Kaiser window (parameter is beta)
            if (parameter == 0) parameter = 5; // Default beta
            for (uint32_t i = 0; i < size; ++i) {
                T n = static_cast<T>(i) / (size - 1);
                T arg = parameter * std::sqrt(1 - (2 * n - 1) * (2 * n - 1));
                // Simplified Bessel function approximation
                coeffs[i] = std::exp(arg) / std::exp(parameter);
            }
            break;
            
        case WindowType::Gaussian:
            // Gaussian window (parameter is sigma)
            if (parameter == 0) parameter = static_cast<T>(0.4); // Default sigma
            for (uint32_t i = 0; i < size; ++i) {
                T n = (static_cast<T>(i) - (size - 1) / 2.0) / ((size - 1) / 2.0);
                coeffs[i] = std::exp(-0.5 * (n / parameter) * (n / parameter));
            }
            break;
            
        case WindowType::Tukey:
            // Tukey window (parameter is alpha)
            if (parameter == 0) parameter = static_cast<T>(0.5); // Default alpha
            for (uint32_t i = 0; i < size; ++i) {
                T n = static_cast<T>(i) / (size - 1);
                if (n < parameter / 2) {
                    coeffs[i] = static_cast<T>(0.5) * (1 + std::cos(M_PI * (2 * n / parameter - 1)));
                } else if (n > 1 - parameter / 2) {
                    coeffs[i] = static_cast<T>(0.5) * (1 + std::cos(M_PI * (2 * n / parameter - 2 / parameter + 1)));
                } else {
                    coeffs[i] = 1;
                }
            }
            break;
            
        default:
            std::fill(coeffs.begin(), coeffs.end(), 1);
            break;
    }
    
    return coeffs;
}

template<typename T>
void FFT::radix2FFT(std::vector<std::complex<T>>& data, bool inverse) {
    uint32_t n = static_cast<uint32_t>(data.size());
    
    if (!isValidSize(n)) {
        FMUS_LOG_ERROR("FFT size must be power of 2");
        return;
    }
    
    // Bit-reverse permutation
    bitReverse(data);
    
    // FFT computation
    for (uint32_t len = 2; len <= n; len <<= 1) {
        T angle = (inverse ? 2 : -2) * M_PI / len;
        std::complex<T> wlen(std::cos(angle), std::sin(angle));
        
        for (uint32_t i = 0; i < n; i += len) {
            std::complex<T> w(1, 0);
            for (uint32_t j = 0; j < len / 2; ++j) {
                std::complex<T> u = data[i + j];
                std::complex<T> v = data[i + j + len / 2] * w;
                data[i + j] = u + v;
                data[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
    
    // Scale for inverse transform
    if (inverse) {
        T scale = static_cast<T>(1.0) / n;
        for (auto& sample : data) {
            sample *= scale;
        }
    }
}

template<typename T>
void FFT::bitReverse(std::vector<std::complex<T>>& data) {
    uint32_t n = static_cast<uint32_t>(data.size());
    uint32_t j = 0;
    
    for (uint32_t i = 1; i < n; ++i) {
        uint32_t bit = n >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        
        if (i < j) {
            std::swap(data[i], data[j]);
        }
    }
}

//=============================================================================
// SpectralAnalysis Implementation
//=============================================================================

template<typename T>
core::Result<T> SpectralAnalysis::findPeakFrequency(const FFTResult<T>& fftResult, T minFreq, T maxFreq) {
    if (fftResult.data.empty()) {
        return core::makeError<T>(core::ErrorCode::InvalidArgument, "FFT result is empty");
    }

    auto magnitude = fftResult.getMagnitude();
    auto frequencies = fftResult.getFrequencyBins();

    if (maxFreq < 0) {
        maxFreq = fftResult.sampleRate / 2; // Nyquist frequency
    }

    T peakFreq = 0;
    T peakMagnitude = 0;

    for (size_t i = 0; i < magnitude.size() && i < frequencies.size(); ++i) {
        if (frequencies[i] >= minFreq && frequencies[i] <= maxFreq) {
            if (magnitude[i] > peakMagnitude) {
                peakMagnitude = magnitude[i];
                peakFreq = frequencies[i];
            }
        }
    }

    if (peakMagnitude == 0) {
        return core::makeError<T>(core::ErrorCode::DataError, "No peak found in specified frequency range");
    }

    return core::makeOk<T>(std::move(peakFreq));
}

template<typename T>
std::vector<T> SpectralAnalysis::findPeaks(const FFTResult<T>& fftResult, uint32_t numPeaks, T minDistance) {
    auto magnitude = fftResult.getMagnitude();
    auto frequencies = fftResult.getFrequencyBins();

    std::vector<std::pair<T, T>> peakPairs; // (magnitude, frequency)

    // Find all local maxima
    for (size_t i = 1; i < magnitude.size() - 1; ++i) {
        if (magnitude[i] > magnitude[i-1] && magnitude[i] > magnitude[i+1]) {
            peakPairs.emplace_back(magnitude[i], frequencies[i]);
        }
    }

    // Sort by magnitude (descending)
    std::sort(peakPairs.begin(), peakPairs.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    // Select peaks with minimum distance constraint
    std::vector<T> selectedPeaks;
    for (const auto& peak : peakPairs) {
        if (selectedPeaks.size() >= numPeaks) break;

        bool tooClose = false;
        for (T existingPeak : selectedPeaks) {
            if (std::abs(peak.second - existingPeak) < minDistance) {
                tooClose = true;
                break;
            }
        }

        if (!tooClose) {
            selectedPeaks.push_back(peak.second);
        }
    }

    // Sort by frequency
    std::sort(selectedPeaks.begin(), selectedPeaks.end());

    return selectedPeaks;
}

template<typename T>
T SpectralAnalysis::calculateSpectralCentroid(const FFTResult<T>& fftResult) {
    auto magnitude = fftResult.getMagnitude();
    auto frequencies = fftResult.getFrequencyBins();

    T numerator = 0;
    T denominator = 0;

    for (size_t i = 0; i < magnitude.size() && i < frequencies.size(); ++i) {
        numerator += frequencies[i] * magnitude[i];
        denominator += magnitude[i];
    }

    return (denominator > 0) ? numerator / denominator : 0;
}

//=============================================================================
// Helper Functions
//=============================================================================

std::string windowTypeToString(WindowType window) {
    switch (window) {
        case WindowType::None: return "None (Rectangular)";
        case WindowType::Hanning: return "Hanning";
        case WindowType::Hamming: return "Hamming";
        case WindowType::Blackman: return "Blackman";
        case WindowType::Kaiser: return "Kaiser";
        case WindowType::Gaussian: return "Gaussian";
        case WindowType::Tukey: return "Tukey";
        default: return "Unknown";
    }
}

//=============================================================================
// Explicit Template Instantiations
//=============================================================================

template struct FFTResult<float>;
template struct FFTResult<double>;

template core::Result<FFTResult<float>> FFT::forward<float>(const std::vector<float>&, float, WindowType);
template core::Result<FFTResult<double>> FFT::forward<double>(const std::vector<double>&, double, WindowType);
template core::Result<FFTResult<float>> FFT::forward<float>(const std::vector<std::complex<float>>&, float, WindowType);
template core::Result<FFTResult<double>> FFT::forward<double>(const std::vector<std::complex<double>>&, double, WindowType);

template core::Result<std::vector<float>> FFT::inverse<float>(const std::vector<std::complex<float>>&);
template core::Result<std::vector<double>> FFT::inverse<double>(const std::vector<std::complex<double>>&);
template core::Result<std::vector<std::complex<float>>> FFT::inverseComplex<float>(const std::vector<std::complex<float>>&);
template core::Result<std::vector<std::complex<double>>> FFT::inverseComplex<double>(const std::vector<std::complex<double>>&);

template std::vector<float> FFT::applyWindow<float>(const std::vector<float>&, WindowType, float);
template std::vector<double> FFT::applyWindow<double>(const std::vector<double>&, WindowType, double);
template std::vector<float> FFT::generateWindow<float>(uint32_t, WindowType, float);
template std::vector<double> FFT::generateWindow<double>(uint32_t, WindowType, double);
template std::vector<float> FFT::zeroPad<float>(const std::vector<float>&, uint32_t);
template std::vector<double> FFT::zeroPad<double>(const std::vector<double>&, uint32_t);

template void FFT::radix2FFT<float>(std::vector<std::complex<float>>&, bool);
template void FFT::radix2FFT<double>(std::vector<std::complex<double>>&, bool);
template void FFT::bitReverse<float>(std::vector<std::complex<float>>&);
template void FFT::bitReverse<double>(std::vector<std::complex<double>>&);

template core::Result<float> SpectralAnalysis::findPeakFrequency<float>(const FFTResult<float>&, float, float);
template core::Result<double> SpectralAnalysis::findPeakFrequency<double>(const FFTResult<double>&, double, double);
template std::vector<float> SpectralAnalysis::findPeaks<float>(const FFTResult<float>&, uint32_t, float);
template std::vector<double> SpectralAnalysis::findPeaks<double>(const FFTResult<double>&, uint32_t, double);
template float SpectralAnalysis::calculateSpectralCentroid<float>(const FFTResult<float>&);
template double SpectralAnalysis::calculateSpectralCentroid<double>(const FFTResult<double>&);

} // namespace dsp
} // namespace fmus
