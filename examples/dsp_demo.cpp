#include <fmus/fmus.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace fmus;
using namespace fmus::dsp;

void demonstrateFilters() {
    std::cout << "\n=== Digital Filters Demo ===" << std::endl;
    
    // Generate test signal: sine wave + noise
    auto sampleRate = 1000.0f;
    auto duration = 1.0f;
    auto signalFreq = 50.0f;
    auto noiseAmp = 0.2f;
    
    auto cleanSignal = SignalGenerator::sine(signalFreq, 1.0f, sampleRate, duration);
    auto noise = SignalGenerator::whiteNoise(noiseAmp, sampleRate, duration, 42);
    
    // Add noise to signal
    std::vector<float> noisySignal;
    for (size_t i = 0; i < cleanSignal.size(); ++i) {
        noisySignal.push_back(cleanSignal[i] + noise[i]);
    }
    
    std::cout << "Generated noisy sine wave: " << noisySignal.size() << " samples" << std::endl;
    
    // Test different filters
    std::cout << "\nTesting Low-Pass Filter..." << std::endl;
    LowPassFilter<float> lowPass(0.1f);
    auto filteredLP = lowPass.process(noisySignal);
    
    std::cout << "Testing High-Pass Filter..." << std::endl;
    HighPassFilter<float> highPass(0.05f);
    auto filteredHP = highPass.process(noisySignal);
    
    std::cout << "Testing Moving Average Filter..." << std::endl;
    MovingAverageFilter<float> movingAvg(10);
    auto filteredMA = movingAvg.process(noisySignal);
    
    std::cout << "Testing Median Filter..." << std::endl;
    MedianFilter<float> median(5);
    auto filteredMed = median.process(noisySignal);
    
    std::cout << "Testing Kalman Filter..." << std::endl;
    KalmanFilter<float> kalman(0.01f, 0.1f);
    std::vector<float> filteredKalman;
    for (float sample : noisySignal) {
        filteredKalman.push_back(kalman.update(sample));
    }
    
    // Calculate signal statistics
    auto originalStats = calculateSignalStats(noisySignal);
    auto lpStats = calculateSignalStats(filteredLP);
    auto maStats = calculateSignalStats(filteredMA);
    
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "\nSignal Statistics:" << std::endl;
    std::cout << "Original - RMS: " << originalStats.rms << ", StdDev: " << originalStats.stdDev << std::endl;
    std::cout << "Low-Pass - RMS: " << lpStats.rms << ", StdDev: " << lpStats.stdDev << std::endl;
    std::cout << "MovingAvg - RMS: " << maStats.rms << ", StdDev: " << maStats.stdDev << std::endl;
}

void demonstrateFFT() {
    std::cout << "\n=== FFT Analysis Demo ===" << std::endl;
    
    // Generate composite signal: 50Hz + 120Hz + 200Hz
    auto sampleRate = 1000.0f;
    auto duration = 1.0f;
    
    auto signal1 = SignalGenerator::sine(50.0f, 1.0f, sampleRate, duration);
    auto signal2 = SignalGenerator::sine(120.0f, 0.5f, sampleRate, duration);
    auto signal3 = SignalGenerator::sine(200.0f, 0.3f, sampleRate, duration);
    
    std::vector<float> compositeSignal;
    for (size_t i = 0; i < signal1.size(); ++i) {
        compositeSignal.push_back(signal1[i] + signal2[i] + signal3[i]);
    }
    
    std::cout << "Generated composite signal with frequencies: 50Hz, 120Hz, 200Hz" << std::endl;
    
    // Perform FFT with different windows
    std::cout << "\nPerforming FFT with Hanning window..." << std::endl;
    auto fftResult = FFT::forward(compositeSignal, sampleRate, WindowType::Hanning);
    
    if (fftResult.isOk()) {
        auto result = fftResult.value();
        auto magnitude = result.getMagnitude();
        auto frequencies = result.getFrequencyBins();
        
        std::cout << "FFT completed - " << result.size << " bins, resolution: " 
                  << result.frequencyResolution << " Hz/bin" << std::endl;
        
        // Find peaks in spectrum
        auto peaks = SpectralAnalysis::findPeaks(result, 5, 10.0f);
        std::cout << "Found " << peaks.size() << " spectral peaks:" << std::endl;
        for (size_t i = 0; i < peaks.size(); ++i) {
            std::cout << "  Peak " << (i+1) << ": " << peaks[i] << " Hz" << std::endl;
        }
        
        // Calculate spectral centroid
        auto centroid = SpectralAnalysis::calculateSpectralCentroid(result);
        std::cout << "Spectral centroid: " << centroid << " Hz" << std::endl;
        
        // Test inverse FFT
        std::cout << "\nTesting inverse FFT..." << std::endl;
        auto inverseResult = FFT::inverse(result.data);
        if (inverseResult.isOk()) {
            auto reconstructed = inverseResult.value();
            std::cout << "Inverse FFT successful - reconstructed " << reconstructed.size() << " samples" << std::endl;
            
            // Calculate reconstruction error
            float maxError = 0;
            for (size_t i = 0; i < std::min(compositeSignal.size(), reconstructed.size()); ++i) {
                float error = std::abs(compositeSignal[i] - reconstructed[i]);
                maxError = std::max(maxError, error);
            }
            std::cout << "Maximum reconstruction error: " << maxError << std::endl;
        }
    } else {
        std::cout << "FFT failed: " << fftResult.error().toString() << std::endl;
    }
}

void demonstrateSignalGeneration() {
    std::cout << "\n=== Signal Generation Demo ===" << std::endl;
    
    auto sampleRate = 1000.0f;
    auto duration = 0.1f; // Short duration for demo
    auto frequency = 100.0f;
    auto amplitude = 1.0f;
    
    // Generate different waveforms
    std::cout << "Generating various waveforms at " << frequency << " Hz..." << std::endl;
    
    auto sine = SignalGenerator::sine(frequency, amplitude, sampleRate, duration);
    auto cosine = SignalGenerator::cosine(frequency, amplitude, sampleRate, duration);
    auto square = SignalGenerator::square(frequency, amplitude, sampleRate, duration, 0.5f);
    auto sawtooth = SignalGenerator::sawtooth(frequency, amplitude, sampleRate, duration);
    auto triangle = SignalGenerator::triangle(frequency, amplitude, sampleRate, duration);
    auto noise = SignalGenerator::whiteNoise(amplitude * 0.1f, sampleRate, duration, 123);
    auto chirp = SignalGenerator::chirp(50.0f, 200.0f, amplitude, sampleRate, duration);
    
    std::cout << "Generated signals:" << std::endl;
    std::cout << "  Sine wave: " << sine.size() << " samples" << std::endl;
    std::cout << "  Cosine wave: " << cosine.size() << " samples" << std::endl;
    std::cout << "  Square wave: " << square.size() << " samples" << std::endl;
    std::cout << "  Sawtooth wave: " << sawtooth.size() << " samples" << std::endl;
    std::cout << "  Triangle wave: " << triangle.size() << " samples" << std::endl;
    std::cout << "  White noise: " << noise.size() << " samples" << std::endl;
    std::cout << "  Chirp (50-200Hz): " << chirp.size() << " samples" << std::endl;
    
    // Calculate and display statistics for each signal
    auto sineStats = calculateSignalStats(sine);
    auto squareStats = calculateSignalStats(square);
    auto noiseStats = calculateSignalStats(noise);
    
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "\nSignal Statistics:" << std::endl;
    std::cout << "Sine - RMS: " << sineStats.rms << ", Peak: " << sineStats.peak << ", Crest Factor: " << sineStats.crestFactor << std::endl;
    std::cout << "Square - RMS: " << squareStats.rms << ", Peak: " << squareStats.peak << ", Crest Factor: " << squareStats.crestFactor << std::endl;
    std::cout << "Noise - RMS: " << noiseStats.rms << ", Peak: " << noiseStats.peak << ", Crest Factor: " << noiseStats.crestFactor << std::endl;
}

void demonstrateRealTimeProcessing() {
    std::cout << "\n=== Real-Time Processing Demo ===" << std::endl;
    
    // Create real-time processor
    RealTimeProcessor<float> processor(256, 1000.0f);
    
    // Add filters to processing chain
    auto lowPass = std::make_shared<LowPassFilter<float>>(0.1f);
    auto movingAvg = std::make_shared<MovingAverageFilter<float>>(5);
    
    processor.addFilter(lowPass);
    processor.addFilter(movingAvg);
    
    std::cout << "Created real-time processor with " << processor.getLatency() << " samples latency" << std::endl;
    
    // Generate test signal
    auto testSignal = SignalGenerator::sine(50.0f, 1.0f, 1000.0f, 0.5f);
    auto noise = SignalGenerator::whiteNoise(0.3f, 1000.0f, 0.5f, 456);
    
    // Add noise
    for (size_t i = 0; i < testSignal.size(); ++i) {
        testSignal[i] += noise[i];
    }
    
    // Process signal in real-time simulation
    std::cout << "Processing " << testSignal.size() << " samples through filter chain..." << std::endl;
    
    auto processedSignal = processor.processBuffer(testSignal);
    
    // Compare input and output statistics
    auto inputStats = calculateSignalStats(testSignal);
    auto outputStats = calculateSignalStats(processedSignal);
    
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Input - RMS: " << inputStats.rms << ", StdDev: " << inputStats.stdDev << std::endl;
    std::cout << "Output - RMS: " << outputStats.rms << ", StdDev: " << outputStats.stdDev << std::endl;
    std::cout << "Noise reduction: " << ((inputStats.stdDev - outputStats.stdDev) / inputStats.stdDev * 100) << "%" << std::endl;
}

int main() {
    // Initialize the fmus-embed library
    if (!fmus::init()) {
        std::cerr << "Failed to initialize fmus-embed library" << std::endl;
        return 1;
    }

    std::cout << "fmus-embed library version: " << core::getVersionString() << std::endl;
    std::cout << "DSP Module Comprehensive Demo" << std::endl;
    std::cout << "=============================" << std::endl;
    
    // Initialize DSP module
    auto dspInitResult = initDSP();
    if (dspInitResult.isError()) {
        std::cout << "DSP module initialization failed: " 
                  << dspInitResult.error().toString() << std::endl;
        return 1;
    }
    
    std::cout << "DSP module initialized successfully!" << std::endl;
    std::cout << getDSPStatus() << std::endl;
    
    try {
        // Demonstrate all DSP functionality
        demonstrateFilters();
        demonstrateFFT();
        demonstrateSignalGeneration();
        demonstrateRealTimeProcessing();
        
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return 1;
    }
    
    // Shutdown DSP module
    auto shutdownResult = shutdownDSP();
    if (shutdownResult.isOk()) {
        std::cout << "\nDSP module shutdown successfully!" << std::endl;
    }
    
    std::cout << "\nDSP demo completed successfully!" << std::endl;
    std::cout << "This demonstrates comprehensive digital signal processing capabilities:" << std::endl;
    std::cout << "- Multiple filter types (Low-pass, High-pass, Moving Average, Median, Kalman)" << std::endl;
    std::cout << "- FFT analysis with windowing and spectral analysis" << std::endl;
    std::cout << "- Signal generation (Sine, Square, Sawtooth, Triangle, Noise, Chirp)" << std::endl;
    std::cout << "- Real-time processing chains" << std::endl;
    std::cout << "- Signal statistics and analysis tools" << std::endl;

    // Clean up
    fmus::shutdown();
    return 0;
}
