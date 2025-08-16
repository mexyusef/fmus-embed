#pragma once

/**
 * @file fft.h
 * @brief Fast Fourier Transform implementation for the fmus-embed library
 *
 * This header provides FFT functionality optimized for embedded systems
 * including real and complex FFT, windowing functions, and spectral analysis.
 */

#include "../fmus_config.h"
#include "../core/result.h"
#include <vector>
#include <complex>
#include <cstdint>

namespace fmus {
namespace dsp {

/**
 * @brief Window function types for FFT preprocessing
 */
enum class WindowType : uint8_t {
    None = 0,       ///< No windowing (rectangular)
    Hanning = 1,    ///< Hanning window
    Hamming = 2,    ///< Hamming window
    Blackman = 3,   ///< Blackman window
    Kaiser = 4,     ///< Kaiser window
    Gaussian = 5,   ///< Gaussian window
    Tukey = 6       ///< Tukey window
};

/**
 * @brief FFT result structure containing frequency domain data
 */
template<typename T>
struct FFTResult {
    std::vector<std::complex<T>> data;  ///< Complex frequency domain data
    T sampleRate;                       ///< Sample rate used
    T frequencyResolution;              ///< Frequency resolution (Hz/bin)
    uint32_t size;                      ///< FFT size
    WindowType windowUsed;              ///< Window function used

    /**
     * @brief Get magnitude spectrum
     *
     * @return std::vector<T> Magnitude values
     */
    std::vector<T> getMagnitude() const;

    /**
     * @brief Get phase spectrum
     *
     * @return std::vector<T> Phase values in radians
     */
    std::vector<T> getPhase() const;

    /**
     * @brief Get power spectrum
     *
     * @return std::vector<T> Power values
     */
    std::vector<T> getPower() const;

    /**
     * @brief Get power spectral density
     *
     * @return std::vector<T> PSD values
     */
    std::vector<T> getPowerSpectralDensity() const;

    /**
     * @brief Get frequency bins
     *
     * @return std::vector<T> Frequency values for each bin
     */
    std::vector<T> getFrequencyBins() const;
};

/**
 * @brief Fast Fourier Transform class
 */
class FMUS_EMBED_API FFT {
public:
    /**
     * @brief Compute forward FFT of real signal
     *
     * @tparam T Data type (float or double)
     * @param input Real input signal
     * @param sampleRate Sample rate in Hz
     * @param window Window function to apply
     * @return core::Result<FFTResult<T>> FFT result or error
     */
    template<typename T>
    static core::Result<FFTResult<T>> forward(const std::vector<T>& input, 
                                             T sampleRate = 1.0, 
                                             WindowType window = WindowType::None);

    /**
     * @brief Compute forward FFT of complex signal
     *
     * @tparam T Data type (float or double)
     * @param input Complex input signal
     * @param sampleRate Sample rate in Hz
     * @param window Window function to apply
     * @return core::Result<FFTResult<T>> FFT result or error
     */
    template<typename T>
    static core::Result<FFTResult<T>> forward(const std::vector<std::complex<T>>& input,
                                             T sampleRate = 1.0,
                                             WindowType window = WindowType::None);

    /**
     * @brief Compute inverse FFT
     *
     * @tparam T Data type (float or double)
     * @param input Complex frequency domain data
     * @return core::Result<std::vector<T>> Real time domain signal or error
     */
    template<typename T>
    static core::Result<std::vector<T>> inverse(const std::vector<std::complex<T>>& input);

    /**
     * @brief Compute inverse FFT returning complex result
     *
     * @tparam T Data type (float or double)
     * @param input Complex frequency domain data
     * @return core::Result<std::vector<std::complex<T>>> Complex time domain signal or error
     */
    template<typename T>
    static core::Result<std::vector<std::complex<T>>> inverseComplex(const std::vector<std::complex<T>>& input);

    /**
     * @brief Apply window function to signal
     *
     * @tparam T Data type
     * @param signal Input signal
     * @param window Window type
     * @param parameter Window parameter (for Kaiser, Gaussian, etc.)
     * @return std::vector<T> Windowed signal
     */
    template<typename T>
    static std::vector<T> applyWindow(const std::vector<T>& signal, 
                                     WindowType window, 
                                     T parameter = 0);

    /**
     * @brief Generate window function coefficients
     *
     * @tparam T Data type
     * @param size Window size
     * @param window Window type
     * @param parameter Window parameter
     * @return std::vector<T> Window coefficients
     */
    template<typename T>
    static std::vector<T> generateWindow(uint32_t size, 
                                        WindowType window, 
                                        T parameter = 0);

    /**
     * @brief Check if size is valid for FFT (power of 2)
     *
     * @param size Size to check
     * @return bool True if valid
     */
    static bool isValidSize(uint32_t size);

    /**
     * @brief Get next power of 2 greater than or equal to n
     *
     * @param n Input value
     * @return uint32_t Next power of 2
     */
    static uint32_t nextPowerOf2(uint32_t n);

    /**
     * @brief Zero-pad signal to specified size
     *
     * @tparam T Data type
     * @param signal Input signal
     * @param targetSize Target size (should be power of 2)
     * @return std::vector<T> Zero-padded signal
     */
    template<typename T>
    static std::vector<T> zeroPad(const std::vector<T>& signal, uint32_t targetSize);

private:
    /**
     * @brief Internal radix-2 FFT implementation
     */
    template<typename T>
    static void radix2FFT(std::vector<std::complex<T>>& data, bool inverse);

    /**
     * @brief Bit-reverse permutation
     */
    template<typename T>
    static void bitReverse(std::vector<std::complex<T>>& data);
};

/**
 * @brief Spectral analysis utilities
 */
class FMUS_EMBED_API SpectralAnalysis {
public:
    /**
     * @brief Find peak frequency in spectrum
     *
     * @tparam T Data type
     * @param fftResult FFT result
     * @param minFreq Minimum frequency to search (Hz)
     * @param maxFreq Maximum frequency to search (Hz)
     * @return core::Result<T> Peak frequency or error
     */
    template<typename T>
    static core::Result<T> findPeakFrequency(const FFTResult<T>& fftResult, 
                                            T minFreq = 0, 
                                            T maxFreq = -1);

    /**
     * @brief Find multiple peaks in spectrum
     *
     * @tparam T Data type
     * @param fftResult FFT result
     * @param numPeaks Number of peaks to find
     * @param minDistance Minimum distance between peaks (Hz)
     * @return std::vector<T> Peak frequencies
     */
    template<typename T>
    static std::vector<T> findPeaks(const FFTResult<T>& fftResult, 
                                   uint32_t numPeaks = 5, 
                                   T minDistance = 0);

    /**
     * @brief Calculate total harmonic distortion
     *
     * @tparam T Data type
     * @param fftResult FFT result
     * @param fundamentalFreq Fundamental frequency (Hz)
     * @param numHarmonics Number of harmonics to consider
     * @return core::Result<T> THD percentage or error
     */
    template<typename T>
    static core::Result<T> calculateTHD(const FFTResult<T>& fftResult, 
                                       T fundamentalFreq, 
                                       uint32_t numHarmonics = 5);

    /**
     * @brief Calculate signal-to-noise ratio
     *
     * @tparam T Data type
     * @param fftResult FFT result
     * @param signalFreq Signal frequency (Hz)
     * @param bandwidth Bandwidth around signal frequency (Hz)
     * @return core::Result<T> SNR in dB or error
     */
    template<typename T>
    static core::Result<T> calculateSNR(const FFTResult<T>& fftResult, 
                                       T signalFreq, 
                                       T bandwidth);

    /**
     * @brief Calculate spectral centroid (brightness measure)
     *
     * @tparam T Data type
     * @param fftResult FFT result
     * @return T Spectral centroid frequency (Hz)
     */
    template<typename T>
    static T calculateSpectralCentroid(const FFTResult<T>& fftResult);

    /**
     * @brief Calculate spectral rolloff
     *
     * @tparam T Data type
     * @param fftResult FFT result
     * @param rolloffPercent Rolloff percentage (e.g., 0.85 for 85%)
     * @return T Rolloff frequency (Hz)
     */
    template<typename T>
    static T calculateSpectralRolloff(const FFTResult<T>& fftResult, T rolloffPercent = 0.85);
};

/**
 * @brief Real-time FFT processor for streaming data
 */
template<typename T>
class FMUS_EMBED_API RealTimeFFT {
public:
    /**
     * @brief Construct real-time FFT processor
     *
     * @param fftSize FFT size (must be power of 2)
     * @param sampleRate Sample rate in Hz
     * @param overlapFactor Overlap factor (0.0 to 0.75)
     * @param window Window function
     */
    RealTimeFFT(uint32_t fftSize, T sampleRate, T overlapFactor = 0.5, WindowType window = WindowType::Hanning);

    /**
     * @brief Destructor
     */
    ~RealTimeFFT();

    /**
     * @brief Process new samples
     *
     * @param samples New input samples
     * @return std::vector<FFTResult<T>> FFT results (may be empty if not enough samples)
     */
    std::vector<FFTResult<T>> processSamples(const std::vector<T>& samples);

    /**
     * @brief Process single sample
     *
     * @param sample Input sample
     * @return core::Result<FFTResult<T>> FFT result or empty if not ready
     */
    core::Result<FFTResult<T>> processSample(T sample);

    /**
     * @brief Reset processor state
     */
    void reset();

    /**
     * @brief Get FFT size
     *
     * @return uint32_t FFT size
     */
    uint32_t getFFTSize() const;

    /**
     * @brief Get sample rate
     *
     * @return T Sample rate
     */
    T getSampleRate() const;

private:
    uint32_t m_fftSize;
    T m_sampleRate;
    T m_overlapFactor;
    WindowType m_window;
    std::vector<T> m_buffer;
    std::vector<T> m_windowCoeffs;
    uint32_t m_bufferIndex;
    uint32_t m_hopSize;
    bool m_bufferReady;
};

/**
 * @brief Get string representation of window type
 *
 * @param window Window type
 * @return std::string String representation
 */
FMUS_EMBED_API std::string windowTypeToString(WindowType window);

// Explicit template instantiations
extern template struct FMUS_EMBED_API FFTResult<float>;
extern template struct FMUS_EMBED_API FFTResult<double>;
extern template class FMUS_EMBED_API RealTimeFFT<float>;
extern template class FMUS_EMBED_API RealTimeFFT<double>;

} // namespace dsp
} // namespace fmus
