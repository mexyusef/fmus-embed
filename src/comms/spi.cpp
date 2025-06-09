#include <fmus/comms/spi.h>
#include <fmus/core/logging.h>
#include <fmus/core/error.h>
#include <unordered_map>
#include <cstring>

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
// SPI simulation for Windows
#elif defined(__linux__)
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#endif

namespace fmus {
namespace comms {

// Mapping of SPI modes to string descriptions
static const std::unordered_map<SPIMode, const char*> modeStrings = {
    { SPIMode::Mode0, "Mode 0 (CPOL=0, CPHA=0)" },
    { SPIMode::Mode1, "Mode 1 (CPOL=0, CPHA=1)" },
    { SPIMode::Mode2, "Mode 2 (CPOL=1, CPHA=0)" },
    { SPIMode::Mode3, "Mode 3 (CPOL=1, CPHA=1)" }
};

// Mapping of SPI bit orders to string descriptions
static const std::unordered_map<SPIBitOrder, const char*> bitOrderStrings = {
    { SPIBitOrder::MSBFirst, "MSB First" },
    { SPIBitOrder::LSBFirst, "LSB First" }
};

// Mapping of SPI chip select modes to string descriptions
static const std::unordered_map<SPIChipSelectMode, const char*> csModeStrings = {
    { SPIChipSelectMode::ActiveLow, "Active Low" },
    { SPIChipSelectMode::ActiveHigh, "Active High" }
};

// Platform-specific implementation structs
struct SPIImpl {
#if defined(_WIN32) || defined(_WIN64)
    // Simulation data for Windows
    std::vector<uint8_t> txBuffer;
    std::vector<uint8_t> rxBuffer;
#elif defined(__linux__)
    int fd;  // File descriptor for SPI device
#endif
};

SPIConfig::SPIConfig()
    : clockFrequency(1000000),  // 1 MHz default
      mode(SPIMode::Mode0),
      bitOrder(SPIBitOrder::MSBFirst),
      csMode(SPIChipSelectMode::ActiveLow),
      dataBits(8),
      useDMA(false) {
}

SPI::SPI(uint8_t busNumber)
    : m_busNumber(busNumber),
      m_initialized(false),
      m_currentCSPin(0xFF),  // Invalid CS pin
      m_impl(nullptr) {
}

SPI::~SPI() {
    if (m_initialized && m_impl) {
        SPIImpl* impl = static_cast<SPIImpl*>(m_impl);

#if defined(_WIN32) || defined(_WIN64)
        // No cleanup needed for Windows simulation
        delete impl;
#elif defined(__linux__)
        // Close SPI device
        if (impl->fd >= 0) {
            close(impl->fd);
        }
        delete impl;
#endif

        m_impl = nullptr;
        m_initialized = false;
    }
}

core::Result<void> SPI::init(const SPIConfig& config) {
    // Store configuration
    m_config = config;

    // Create implementation if it doesn't exist
    if (!m_impl) {
        m_impl = new SPIImpl();
    }

    SPIImpl* impl = static_cast<SPIImpl*>(m_impl);

#if defined(_WIN32) || defined(_WIN64)
    // Simulate SPI for Windows
    impl->txBuffer.clear();
    impl->rxBuffer.clear();
    m_initialized = true;

    FMUS_LOG_INFO("Initialized SPI bus " + std::to_string(m_busNumber) +
                 " (Windows simulation)");
#elif defined(__linux__)
    // Open SPI device
    char devicePath[64];
    snprintf(devicePath, sizeof(devicePath), "/dev/spidev%d.0", m_busNumber);

    impl->fd = open(devicePath, O_RDWR);
    if (impl->fd < 0) {
        return core::Error(core::ErrorCode::CommInitFailed,
                         "Failed to open SPI device: " + std::string(devicePath));
    }

    // Apply configuration
    auto result = applyConfig();
    if (result.isError()) {
        close(impl->fd);
        impl->fd = -1;
        return result;
    }

    m_initialized = true;

    FMUS_LOG_INFO("Initialized SPI bus " + std::to_string(m_busNumber) +
                 " at " + std::to_string(m_config.clockFrequency) + " Hz");
#endif

    return core::makeOk();
}

bool SPI::isInitialized() const {
    return m_initialized;
}

SPI& SPI::setMode(SPIMode mode) {
    m_config.mode = mode;

    if (m_initialized) {
        applyConfig();
    }

    return *this;
}

SPI& SPI::setClockFreq(uint32_t frequency) {
    m_config.clockFrequency = frequency;

    if (m_initialized) {
        applyConfig();
    }

    return *this;
}

SPI& SPI::setBitOrder(SPIBitOrder bitOrder) {
    m_config.bitOrder = bitOrder;

    if (m_initialized) {
        applyConfig();
    }

    return *this;
}

SPI& SPI::setChipSelectMode(SPIChipSelectMode csMode) {
    m_config.csMode = csMode;

    if (m_initialized) {
        applyConfig();
    }

    return *this;
}

SPI& SPI::setDataBits(uint8_t dataBits) {
    m_config.dataBits = dataBits;

    if (m_initialized) {
        applyConfig();
    }

    return *this;
}

SPI& SPI::setUseDMA(bool enable) {
    m_config.useDMA = enable;

    if (m_initialized) {
        applyConfig();
    }

    return *this;
}

SPI& SPI::select(uint8_t csPin) {
    if (!m_initialized) {
        FMUS_LOG_ERROR("Cannot select CS pin: SPI not initialized");
        return *this;
    }

    // If a pin is already selected, deselect it first
    if (m_currentCSPin != 0xFF) {
        deselect();
    }

    m_currentCSPin = csPin;

    // In a real implementation, we would set the CS pin low (or high for ActiveHigh mode)
    FMUS_LOG_DEBUG("Selected SPI CS pin " + std::to_string(csPin));

    return *this;
}

SPI& SPI::deselect() {
    if (!m_initialized) {
        FMUS_LOG_ERROR("Cannot deselect CS pin: SPI not initialized");
        return *this;
    }

    if (m_currentCSPin != 0xFF) {
        // In a real implementation, we would set the CS pin high (or low for ActiveHigh mode)
        FMUS_LOG_DEBUG("Deselected SPI CS pin " + std::to_string(m_currentCSPin));
        m_currentCSPin = 0xFF;
    }

    return *this;
}

core::Result<void> SPI::write(const uint8_t* data, size_t size) {
    if (!m_initialized) {
        return core::Error(core::ErrorCode::CommInitFailed,
                         "SPI not initialized");
    }

    if (!data || size == 0) {
        return core::Error(core::ErrorCode::InvalidArgument,
                         "Invalid data or size");
    }

    SPIImpl* impl = static_cast<SPIImpl*>(m_impl);

#if defined(_WIN32) || defined(_WIN64)
    // Simulate SPI write for Windows
    impl->txBuffer.assign(data, data + size);
    impl->rxBuffer.resize(size, 0);  // Fill with zeros

    FMUS_LOG_DEBUG("SPI write: " + std::to_string(size) + " bytes");
#elif defined(__linux__)
    // Prepare transfer structure
    struct spi_ioc_transfer tr;
    memset(&tr, 0, sizeof(tr));
    tr.tx_buf = (unsigned long)data;
    tr.rx_buf = 0;  // No receive buffer
    tr.len = size;
    tr.speed_hz = m_config.clockFrequency;
    tr.bits_per_word = m_config.dataBits;
    tr.delay_usecs = 0;

    // Perform transfer
    int ret = ioctl(impl->fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 0) {
        return core::Error(core::ErrorCode::CommTransmitError,
                         "SPI write failed");
    }

    FMUS_LOG_DEBUG("SPI write: " + std::to_string(size) + " bytes");
#endif

    return core::makeOk();
}

core::Result<void> SPI::write(uint8_t data) {
    return write(&data, 1);
}

core::Result<void> SPI::read(uint8_t* data, size_t size) {
    if (!m_initialized) {
        return core::Error(core::ErrorCode::CommInitFailed,
                         "SPI not initialized");
    }

    if (!data || size == 0) {
        return core::Error(core::ErrorCode::InvalidArgument,
                         "Invalid data or size");
    }

    SPIImpl* impl = static_cast<SPIImpl*>(m_impl);

#if defined(_WIN32) || defined(_WIN64)
    // Simulate SPI read for Windows
    impl->txBuffer.resize(size, 0);  // Fill with zeros
    impl->rxBuffer.resize(size, 0);  // Fill with zeros

    // In a real implementation, this would be actual data from the device
    // For simulation, just fill with incrementing values
    for (size_t i = 0; i < size; ++i) {
        data[i] = static_cast<uint8_t>(i & 0xFF);
    }

    FMUS_LOG_DEBUG("SPI read: " + std::to_string(size) + " bytes");
#elif defined(__linux__)
    // Prepare transfer structure
    struct spi_ioc_transfer tr;
    memset(&tr, 0, sizeof(tr));
    tr.tx_buf = 0;  // No transmit buffer
    tr.rx_buf = (unsigned long)data;
    tr.len = size;
    tr.speed_hz = m_config.clockFrequency;
    tr.bits_per_word = m_config.dataBits;
    tr.delay_usecs = 0;

    // Perform transfer
    int ret = ioctl(impl->fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 0) {
        return core::Error(core::ErrorCode::CommReceiveError,
                         "SPI read failed");
    }

    FMUS_LOG_DEBUG("SPI read: " + std::to_string(size) + " bytes");
#endif

    return core::makeOk();
}

core::Result<uint8_t> SPI::read() {
    if (!m_initialized) {
        return core::Error(core::ErrorCode::CommInitFailed,
                         "SPI not initialized");
    }

    uint8_t txData = 0;
    uint8_t rxData = 0;
    auto result = transfer(&txData, &rxData, 1);
    if (result.isError()) {
        return core::Error(result.error().code(), result.error().message());
    }
    return core::makeOk(std::move(rxData));
}

core::Result<void> SPI::transfer(const uint8_t* txData, uint8_t* rxData, size_t size) {
    if (!m_initialized) {
        return core::Error(core::ErrorCode::CommInitFailed,
                         "SPI not initialized");
    }

    if (!txData || !rxData || size == 0) {
        return core::Error(core::ErrorCode::InvalidArgument,
                         "Invalid data or size");
    }

    SPIImpl* impl = static_cast<SPIImpl*>(m_impl);

#if defined(_WIN32) || defined(_WIN64)
    // Simulate SPI transfer for Windows
    impl->txBuffer.assign(txData, txData + size);
    impl->rxBuffer.resize(size, 0);  // Fill with zeros

    // In a real implementation, this would be actual data from the device
    // For simulation, just fill with incrementing values
    for (size_t i = 0; i < size; ++i) {
        rxData[i] = static_cast<uint8_t>(i & 0xFF);
    }

    FMUS_LOG_DEBUG("SPI transfer: " + std::to_string(size) + " bytes");
#elif defined(__linux__)
    // Prepare transfer structure
    struct spi_ioc_transfer tr;
    memset(&tr, 0, sizeof(tr));
    tr.tx_buf = (unsigned long)txData;
    tr.rx_buf = (unsigned long)rxData;
    tr.len = size;
    tr.speed_hz = m_config.clockFrequency;
    tr.bits_per_word = m_config.dataBits;
    tr.delay_usecs = 0;

    // Perform transfer
    int ret = ioctl(impl->fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 0) {
        return core::Error(core::ErrorCode::CommTransmitError,
                         "SPI transfer failed");
    }

    FMUS_LOG_DEBUG("SPI transfer: " + std::to_string(size) + " bytes");
#endif

    return core::makeOk();
}

core::Result<uint8_t> SPI::transfer(uint8_t data) {
    if (!m_initialized) {
        return core::Error(core::ErrorCode::CommInitFailed,
                         "SPI not initialized");
    }

    uint8_t rxData;
    auto result = transfer(&data, &rxData, 1);
    if (result.isError()) {
        return core::Error(result.error().code(), result.error().message());
    }
    return core::makeOk(std::move(rxData));
}

uint8_t SPI::getBusNumber() const {
    return m_busNumber;
}

const SPIConfig& SPI::getConfig() const {
    return m_config;
}

core::Result<void> SPI::applyConfig() {
    if (!m_initialized || !m_impl) {
        return core::Error(core::ErrorCode::CommInitFailed,
                         "SPI not initialized");
    }

    SPIImpl* impl = static_cast<SPIImpl*>(m_impl);

#if defined(_WIN32) || defined(_WIN64)
    // No hardware configuration for Windows simulation
    FMUS_LOG_DEBUG("Applied SPI configuration (Windows simulation)");
#elif defined(__linux__)
    // Set SPI mode
    uint8_t mode = static_cast<uint8_t>(m_config.mode);
    if (ioctl(impl->fd, SPI_IOC_WR_MODE, &mode) < 0) {
        return core::Error(core::ErrorCode::CommInitFailed,
                         "Failed to set SPI mode");
    }

    // Set bits per word
    uint8_t bits = m_config.dataBits;
    if (ioctl(impl->fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
        return core::Error(core::ErrorCode::CommInitFailed,
                         "Failed to set SPI bits per word");
    }

    // Set max speed
    uint32_t speed = m_config.clockFrequency;
    if (ioctl(impl->fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        return core::Error(core::ErrorCode::CommInitFailed,
                         "Failed to set SPI clock frequency");
    }

    // LSB first is not directly supported by all SPI controllers
    // It may require additional configuration or bit manipulation

    FMUS_LOG_DEBUG("Applied SPI configuration: " +
                  spiModeToString(m_config.mode) + ", " +
                  std::to_string(m_config.clockFrequency) + " Hz, " +
                  std::to_string(m_config.dataBits) + " bits");
#endif

    return core::makeOk();
}

std::string spiModeToString(SPIMode mode) {
    auto it = modeStrings.find(mode);
    if (it != modeStrings.end()) {
        return it->second;
    }
    return "Unknown";
}

std::string spiBitOrderToString(SPIBitOrder bitOrder) {
    auto it = bitOrderStrings.find(bitOrder);
    if (it != bitOrderStrings.end()) {
        return it->second;
    }
    return "Unknown";
}

std::string spiChipSelectModeToString(SPIChipSelectMode csMode) {
    auto it = csModeStrings.find(csMode);
    if (it != csModeStrings.end()) {
        return it->second;
    }
    return "Unknown";
}

} // namespace comms
} // namespace fmus
