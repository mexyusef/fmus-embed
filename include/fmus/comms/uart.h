#pragma once

/**
 * @file uart.h
 * @brief UART communication interface for the fmus-embed library
 *
 * This header provides UART (Universal Asynchronous Receiver-Transmitter) 
 * communication functionality with support for various baud rates, data formats,
 * and advanced features like DMA and interrupt-driven operation.
 */

#include "../fmus_config.h"
#include "../core/result.h"
#include <vector>
#include <cstdint>
#include <functional>
#include <string>

namespace fmus {
namespace comms {

/**
 * @brief UART parity options
 */
enum class UARTParity : uint8_t {
    None = 0,    ///< No parity
    Even = 1,    ///< Even parity
    Odd = 2,     ///< Odd parity
    Mark = 3,    ///< Mark parity (always 1)
    Space = 4    ///< Space parity (always 0)
};

/**
 * @brief UART stop bits options
 */
enum class UARTStopBits : uint8_t {
    One = 1,     ///< 1 stop bit
    OneAndHalf = 2, ///< 1.5 stop bits
    Two = 3      ///< 2 stop bits
};

/**
 * @brief UART data bits options
 */
enum class UARTDataBits : uint8_t {
    Five = 5,    ///< 5 data bits
    Six = 6,     ///< 6 data bits
    Seven = 7,   ///< 7 data bits
    Eight = 8    ///< 8 data bits
};

/**
 * @brief UART flow control options
 */
enum class UARTFlowControl : uint8_t {
    None = 0,    ///< No flow control
    RTS_CTS = 1, ///< RTS/CTS hardware flow control
    XON_XOFF = 2 ///< XON/XOFF software flow control
};

/**
 * @brief UART configuration structure
 */
struct UARTConfig {
    uint32_t baudRate;              ///< Baud rate (e.g., 9600, 115200)
    UARTDataBits dataBits;          ///< Number of data bits
    UARTParity parity;              ///< Parity setting
    UARTStopBits stopBits;          ///< Number of stop bits
    UARTFlowControl flowControl;    ///< Flow control method
    uint16_t rxBufferSize;          ///< Receive buffer size in bytes
    uint16_t txBufferSize;          ///< Transmit buffer size in bytes
    uint16_t timeoutMs;             ///< Timeout in milliseconds
    bool useDMA;                    ///< Use DMA for transfers
    bool useInterrupts;             ///< Use interrupt-driven operation

    /**
     * @brief Constructor with default values
     */
    UARTConfig(uint32_t baud = 115200,
               UARTDataBits data = UARTDataBits::Eight,
               UARTParity par = UARTParity::None,
               UARTStopBits stop = UARTStopBits::One,
               UARTFlowControl flow = UARTFlowControl::None,
               uint16_t rxBuf = 256,
               uint16_t txBuf = 256,
               uint16_t timeout = 1000,
               bool dma = false,
               bool interrupts = true)
        : baudRate(baud), dataBits(data), parity(par), stopBits(stop),
          flowControl(flow), rxBufferSize(rxBuf), txBufferSize(txBuf),
          timeoutMs(timeout), useDMA(dma), useInterrupts(interrupts) {}
};

/**
 * @brief Callback function type for asynchronous operations
 */
using UARTCallback = std::function<void(core::Result<void>)>;

/**
 * @brief Callback function type for data reception
 */
using UARTDataCallback = std::function<void(const std::vector<uint8_t>&)>;

/**
 * @brief UART communication interface
 *
 * This class provides UART communication functionality with support for
 * synchronous and asynchronous operations, buffered I/O, and various
 * configuration options.
 */
class FMUS_EMBED_API UART {
public:
    /**
     * @brief Construct a new UART instance
     *
     * @param portNumber The UART port number (0, 1, 2, etc.)
     */
    explicit UART(uint8_t portNumber);

    /**
     * @brief Destructor
     */
    ~UART();

    /**
     * @brief Initialize the UART port
     *
     * @param config The UART configuration
     * @return core::Result<void> Success or error
     */
    core::Result<void> init(const UARTConfig& config = UARTConfig());

    /**
     * @brief Check if the UART port is initialized
     *
     * @return bool True if initialized
     */
    bool isInitialized() const;

    /**
     * @brief Close the UART port
     *
     * @return core::Result<void> Success or error
     */
    core::Result<void> close();

    /**
     * @brief Write data to the UART port (synchronous)
     *
     * @param data The data to write
     * @return core::Result<void> Success or error
     */
    core::Result<void> write(const std::vector<uint8_t>& data);

    /**
     * @brief Write a string to the UART port (synchronous)
     *
     * @param str The string to write
     * @return core::Result<void> Success or error
     */
    core::Result<void> write(const std::string& str);

    /**
     * @brief Write a single byte to the UART port (synchronous)
     *
     * @param byte The byte to write
     * @return core::Result<void> Success or error
     */
    core::Result<void> write(uint8_t byte);

    /**
     * @brief Read data from the UART port (synchronous)
     *
     * @param maxBytes Maximum number of bytes to read
     * @return core::Result<std::vector<uint8_t>> The read data or error
     */
    core::Result<std::vector<uint8_t>> read(size_t maxBytes);

    /**
     * @brief Read a line from the UART port (synchronous)
     *
     * Reads until a newline character or timeout
     *
     * @param delimiter Line delimiter (default: '\n')
     * @return core::Result<std::string> The read line or error
     */
    core::Result<std::string> readLine(char delimiter = '\n');

    /**
     * @brief Check how many bytes are available to read
     *
     * @return core::Result<size_t> Number of available bytes or error
     */
    core::Result<size_t> available() const;

    /**
     * @brief Write data to the UART port (asynchronous)
     *
     * @param data The data to write
     * @param callback Callback function called when operation completes
     * @return core::Result<void> Success or error (immediate)
     */
    core::Result<void> writeAsync(const std::vector<uint8_t>& data, UARTCallback callback);

    /**
     * @brief Set a callback for incoming data
     *
     * @param callback Callback function called when data is received
     * @return core::Result<void> Success or error
     */
    core::Result<void> setDataCallback(UARTDataCallback callback);

    /**
     * @brief Flush the transmit buffer
     *
     * @return core::Result<void> Success or error
     */
    core::Result<void> flush();

    /**
     * @brief Clear the receive buffer
     *
     * @return core::Result<void> Success or error
     */
    core::Result<void> clearRxBuffer();

    /**
     * @brief Set the baud rate
     *
     * @param baudRate The new baud rate
     * @return core::Result<void> Success or error
     */
    core::Result<void> setBaudRate(uint32_t baudRate);

    /**
     * @brief Set the timeout for read operations
     *
     * @param timeoutMs Timeout in milliseconds
     * @return core::Result<void> Success or error
     */
    core::Result<void> setTimeout(uint16_t timeoutMs);

    /**
     * @brief Get the current UART configuration
     *
     * @return const UARTConfig& The current configuration
     */
    const UARTConfig& getConfig() const;

    /**
     * @brief Get the port number
     *
     * @return uint8_t The port number
     */
    uint8_t getPortNumber() const;

    /**
     * @brief Check if data transmission is in progress
     *
     * @return bool True if transmitting
     */
    bool isTransmitting() const;

    /**
     * @brief Get transmission statistics
     *
     * @return std::string Statistics as a formatted string
     */
    std::string getStatistics() const;

private:
    uint8_t m_portNumber;           ///< UART port number
    bool m_initialized;             ///< Initialization state
    UARTConfig m_config;            ///< Current configuration
    void* m_impl;                   ///< Platform-specific implementation
    UARTDataCallback m_dataCallback; ///< Data reception callback

    /**
     * @brief Apply configuration changes to the hardware
     *
     * @return core::Result<void> Success or error
     */
    core::Result<void> applyConfig();

    /**
     * @brief Internal data reception handler
     */
    void handleDataReception();
};

/**
 * @brief Get a string representation of UART parity
 *
 * @param parity The parity setting
 * @return std::string The string representation
 */
FMUS_EMBED_API std::string uartParityToString(UARTParity parity);

/**
 * @brief Get a string representation of UART stop bits
 *
 * @param stopBits The stop bits setting
 * @return std::string The string representation
 */
FMUS_EMBED_API std::string uartStopBitsToString(UARTStopBits stopBits);

/**
 * @brief Get a string representation of UART flow control
 *
 * @param flowControl The flow control setting
 * @return std::string The string representation
 */
FMUS_EMBED_API std::string uartFlowControlToString(UARTFlowControl flowControl);

} // namespace comms
} // namespace fmus
