#pragma once

#include "../fmus_config.h"
#include "../core/result.h"
#include <cstdint>
#include <vector>
#include <string>

namespace fmus {
namespace comms {

/**
 * @brief SPI clock polarity options
 */
enum class SPIClockPolarity : uint8_t {
    IdleLow = 0,   ///< Clock is low when idle (CPOL=0)
    IdleHigh = 1   ///< Clock is high when idle (CPOL=1)
};

/**
 * @brief SPI clock phase options
 */
enum class SPIClockPhase : uint8_t {
    FirstEdge = 0, ///< Data sampled on first edge (CPHA=0)
    SecondEdge = 1 ///< Data sampled on second edge (CPHA=1)
};

/**
 * @brief SPI bit order options
 */
enum class SPIBitOrder : uint8_t {
    MSBFirst = 0,  ///< Most significant bit first
    LSBFirst = 1   ///< Least significant bit first
};

/**
 * @brief SPI mode combinations (CPOL and CPHA)
 */
enum class SPIMode : uint8_t {
    Mode0 = 0,     ///< CPOL=0, CPHA=0
    Mode1 = 1,     ///< CPOL=0, CPHA=1
    Mode2 = 2,     ///< CPOL=1, CPHA=0
    Mode3 = 3      ///< CPOL=1, CPHA=1
};

/**
 * @brief SPI chip select mode
 */
enum class SPIChipSelectMode : uint8_t {
    ActiveLow = 0,  ///< CS is active low (default)
    ActiveHigh = 1  ///< CS is active high
};

/**
 * @brief SPI configuration
 */
struct SPIConfig {
    uint32_t clockFrequency;       ///< Clock frequency in Hz
    SPIMode mode;                  ///< SPI mode
    SPIBitOrder bitOrder;          ///< Bit order
    SPIChipSelectMode csMode;      ///< Chip select mode
    uint8_t dataBits;              ///< Data bits per transfer (typically 8)
    bool useDMA;                   ///< Use DMA for transfers

    /**
     * @brief Construct a new SPI Config with default settings
     */
    SPIConfig();
};

/**
 * @brief SPI communication interface
 */
class FMUS_EMBED_API SPI {
public:
    /**
     * @brief Construct a new SPI instance
     *
     * @param busNumber The SPI bus number
     */
    explicit SPI(uint8_t busNumber);

    /**
     * @brief Destructor
     */
    ~SPI();

    /**
     * @brief Initialize the SPI bus
     *
     * @param config The SPI configuration
     * @return core::Result<void> Success or error
     */
    core::Result<void> init(const SPIConfig& config = SPIConfig());

    /**
     * @brief Check if the SPI bus is initialized
     *
     * @return bool True if initialized
     */
    bool isInitialized() const;

    /**
     * @brief Set the SPI mode
     *
     * @param mode The SPI mode
     * @return SPI& This SPI instance for method chaining
     */
    SPI& setMode(SPIMode mode);

    /**
     * @brief Set the clock frequency
     *
     * @param frequency The clock frequency in Hz
     * @return SPI& This SPI instance for method chaining
     */
    SPI& setClockFreq(uint32_t frequency);

    /**
     * @brief Set the bit order
     *
     * @param bitOrder The bit order
     * @return SPI& This SPI instance for method chaining
     */
    SPI& setBitOrder(SPIBitOrder bitOrder);

    /**
     * @brief Set the chip select mode
     *
     * @param csMode The chip select mode
     * @return SPI& This SPI instance for method chaining
     */
    SPI& setChipSelectMode(SPIChipSelectMode csMode);

    /**
     * @brief Set the data bits per transfer
     *
     * @param dataBits The number of data bits
     * @return SPI& This SPI instance for method chaining
     */
    SPI& setDataBits(uint8_t dataBits);

    /**
     * @brief Enable or disable DMA for transfers
     *
     * @param enable True to enable, false to disable
     * @return SPI& This SPI instance for method chaining
     */
    SPI& setUseDMA(bool enable);

    /**
     * @brief Select a chip select line
     *
     * @param csPin The chip select pin number
     * @return SPI& This SPI instance for method chaining
     */
    SPI& select(uint8_t csPin);

    /**
     * @brief Deselect the current chip select line
     *
     * @return SPI& This SPI instance for method chaining
     */
    SPI& deselect();

    /**
     * @brief Write data to the SPI bus
     *
     * @param data The data to write
     * @param size The size of the data in bytes
     * @return core::Result<void> Success or error
     */
    core::Result<void> write(const uint8_t* data, size_t size);

    /**
     * @brief Write a single byte to the SPI bus
     *
     * @param data The byte to write
     * @return core::Result<void> Success or error
     */
    core::Result<void> write(uint8_t data);

    /**
     * @brief Read data from the SPI bus
     *
     * @param data Buffer to store the read data
     * @param size The size of the buffer in bytes
     * @return core::Result<void> Success or error
     */
    core::Result<void> read(uint8_t* data, size_t size);

    /**
     * @brief Read a single byte from the SPI bus
     *
     * @return core::Result<uint8_t> The read byte or error
     */
    core::Result<uint8_t> read();

    /**
     * @brief Transfer data (simultaneous read and write)
     *
     * @param txData The data to write
     * @param rxData Buffer to store the read data
     * @param size The size of the data in bytes
     * @return core::Result<void> Success or error
     */
    core::Result<void> transfer(const uint8_t* txData, uint8_t* rxData, size_t size);

    /**
     * @brief Transfer a single byte
     *
     * @param data The byte to write
     * @return core::Result<uint8_t> The read byte or error
     */
    core::Result<uint8_t> transfer(uint8_t data);

    /**
     * @brief Get the bus number
     *
     * @return uint8_t The bus number
     */
    uint8_t getBusNumber() const;

    /**
     * @brief Get the current SPI configuration
     *
     * @return const SPIConfig& The current configuration
     */
    const SPIConfig& getConfig() const;

private:
    uint8_t m_busNumber;           ///< SPI bus number
    bool m_initialized;            ///< Initialization state
    SPIConfig m_config;            ///< Current configuration
    uint8_t m_currentCSPin;        ///< Current chip select pin
    void* m_impl;                  ///< Platform-specific implementation

    /**
     * @brief Apply configuration changes to the hardware
     *
     * @return core::Result<void> Success or error
     */
    core::Result<void> applyConfig();
};

/**
 * @brief Get a string representation of an SPI mode
 *
 * @param mode The SPI mode
 * @return std::string The string representation
 */
FMUS_EMBED_API std::string spiModeToString(SPIMode mode);

/**
 * @brief Get a string representation of an SPI bit order
 *
 * @param bitOrder The SPI bit order
 * @return std::string The string representation
 */
FMUS_EMBED_API std::string spiBitOrderToString(SPIBitOrder bitOrder);

/**
 * @brief Get a string representation of an SPI chip select mode
 *
 * @param csMode The SPI chip select mode
 * @return std::string The string representation
 */
FMUS_EMBED_API std::string spiChipSelectModeToString(SPIChipSelectMode csMode);

} // namespace comms
} // namespace fmus
