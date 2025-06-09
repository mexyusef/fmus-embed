#pragma once

/**
 * @file adc.h
 * @brief MCU-specific ADC interface for the fmus-embed library
 *
 * This header provides MCU-specific Analog-to-Digital Converter functionality
 * that abstracts hardware-specific implementations.
 */

#include "../fmus_config.h"
#include "../core/result.h"

namespace fmus {
namespace mcu {

/**
 * @brief ADC resolution options
 */
enum class AdcResolution {
    Bits8  = 8,   ///< 8-bit resolution (0-255)
    Bits10 = 10,  ///< 10-bit resolution (0-1023)
    Bits12 = 12,  ///< 12-bit resolution (0-4095)
    Bits16 = 16   ///< 16-bit resolution (0-65535)
};

/**
 * @brief ADC reference voltage options
 */
enum class AdcReference {
    Default,      ///< Default reference voltage
    Internal,     ///< Internal reference voltage
    External      ///< External reference voltage
};

/**
 * @brief ADC sampling rate options
 */
enum class AdcSamplingRate {
    Low,          ///< Low sampling rate
    Medium,       ///< Medium sampling rate
    High,         ///< High sampling rate
    VeryHigh      ///< Very high sampling rate
};

/**
 * @brief Initialize the ADC subsystem
 *
 * @return Result indicating success or failure
 */
FMUS_EMBED_API core::Result<void> initAdc();

/**
 * @brief Configure ADC settings
 *
 * @param resolution ADC resolution
 * @param reference ADC reference voltage
 * @param samplingRate ADC sampling rate
 * @return Result indicating success or failure
 */
FMUS_EMBED_API core::Result<void> configureAdc(
    AdcResolution resolution = AdcResolution::Bits10,
    AdcReference reference = AdcReference::Default,
    AdcSamplingRate samplingRate = AdcSamplingRate::Medium
);

/**
 * @brief Read analog value from an ADC channel
 *
 * @param channel ADC channel number
 * @return Result containing the analog value or an error
 */
FMUS_EMBED_API core::Result<uint16_t> readAdc(uint8_t channel);

/**
 * @brief Read analog value from an ADC channel with averaging
 *
 * @param channel ADC channel number
 * @param samples Number of samples to average
 * @return Result containing the averaged analog value or an error
 */
FMUS_EMBED_API core::Result<uint16_t> readAdcAverage(uint8_t channel, uint8_t samples);

/**
 * @brief Convert ADC value to voltage
 *
 * @param adcValue Raw ADC value
 * @return Voltage in millivolts
 */
FMUS_EMBED_API uint32_t adcToMillivolts(uint16_t adcValue);

/**
 * @brief Get the maximum value for the current ADC resolution
 *
 * @return Maximum ADC value
 */
FMUS_EMBED_API uint16_t getAdcMaxValue();

} // namespace mcu
} // namespace fmus
