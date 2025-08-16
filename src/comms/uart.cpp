#include "fmus/comms/uart.h"
#include "fmus/core/logging.h"
#include <cstring>
#include <thread>
#include <chrono>
#include <sstream>
#include <algorithm>

#ifdef __linux__
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <errno.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

namespace fmus {
namespace comms {

// Platform-specific implementation structure
struct UARTImpl {
#ifdef __linux__
    int fd;                         ///< File descriptor for UART device
    struct termios originalTermios; ///< Original terminal settings
    std::vector<uint8_t> rxBuffer;  ///< Receive buffer
    std::vector<uint8_t> txBuffer;  ///< Transmit buffer
    std::thread rxThread;           ///< Receive thread
    bool rxThreadRunning;           ///< Receive thread running flag
#elif defined(_WIN32)
    HANDLE hSerial;                 ///< Windows serial handle
    DCB dcbSerialParams;            ///< Serial parameters
    COMMTIMEOUTS timeouts;          ///< Timeout settings
    std::vector<uint8_t> rxBuffer;  ///< Receive buffer
    std::vector<uint8_t> txBuffer;  ///< Transmit buffer
    std::thread rxThread;           ///< Receive thread
    bool rxThreadRunning;           ///< Receive thread running flag
#endif
    uint64_t bytesTransmitted;      ///< Statistics: bytes transmitted
    uint64_t bytesReceived;         ///< Statistics: bytes received
    uint64_t transmissionErrors;    ///< Statistics: transmission errors
    uint64_t receptionErrors;       ///< Statistics: reception errors
};

UART::UART(uint8_t portNumber)
    : m_portNumber(portNumber),
      m_initialized(false),
      m_impl(nullptr),
      m_dataCallback(nullptr) {
}

UART::~UART() {
    if (m_initialized) {
        close();
    }
}

core::Result<void> UART::init(const UARTConfig& config) {
    FMUS_LOG_INFO("Initializing UART port " + std::to_string(m_portNumber));

    if (m_initialized) {
        return core::makeError<void>(core::ErrorCode::CommInitFailed,
                                   "UART already initialized");
    }

    m_config = config;
    m_impl = new UARTImpl();
    UARTImpl* impl = static_cast<UARTImpl*>(m_impl);

    // Initialize statistics
    impl->bytesTransmitted = 0;
    impl->bytesReceived = 0;
    impl->transmissionErrors = 0;
    impl->receptionErrors = 0;

#ifdef __linux__
    // Open UART device
    char devicePath[32];
    snprintf(devicePath, sizeof(devicePath), "/dev/ttyUSB%d", m_portNumber);
    
    impl->fd = open(devicePath, O_RDWR | O_NOCTTY | O_NDELAY);
    if (impl->fd < 0) {
        // Try alternative device names
        snprintf(devicePath, sizeof(devicePath), "/dev/ttyACM%d", m_portNumber);
        impl->fd = open(devicePath, O_RDWR | O_NOCTTY | O_NDELAY);
        
        if (impl->fd < 0) {
            snprintf(devicePath, sizeof(devicePath), "/dev/ttyS%d", m_portNumber);
            impl->fd = open(devicePath, O_RDWR | O_NOCTTY | O_NDELAY);
        }
    }

    if (impl->fd < 0) {
        delete impl;
        m_impl = nullptr;
        return core::makeError<void>(core::ErrorCode::CommInitFailed,
                                   "Failed to open UART device: " + std::string(devicePath));
    }

    // Save original terminal settings
    if (tcgetattr(impl->fd, &impl->originalTermios) != 0) {
        ::close(impl->fd);
        delete impl;
        m_impl = nullptr;
        return core::makeError<void>(core::ErrorCode::CommInitFailed,
                                   "Failed to get terminal attributes");
    }

    // Apply configuration
    auto result = applyConfig();
    if (result.isError()) {
        ::close(impl->fd);
        delete impl;
        m_impl = nullptr;
        return result;
    }

#elif defined(_WIN32)
    // Windows implementation
    char portName[16];
    snprintf(portName, sizeof(portName), "\\\\.\\COM%d", m_portNumber + 1);
    
    impl->hSerial = CreateFileA(portName,
                               GENERIC_READ | GENERIC_WRITE,
                               0,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);

    if (impl->hSerial == INVALID_HANDLE_VALUE) {
        delete impl;
        m_impl = nullptr;
        return core::makeError<void>(core::ErrorCode::CommInitFailed,
                                   "Failed to open COM port: " + std::string(portName));
    }

    // Apply configuration
    auto result = applyConfig();
    if (result.isError()) {
        CloseHandle(impl->hSerial);
        delete impl;
        m_impl = nullptr;
        return result;
    }
#endif

    // Initialize buffers
    impl->rxBuffer.reserve(m_config.rxBufferSize);
    impl->txBuffer.reserve(m_config.txBufferSize);

    // Start receive thread if using interrupts
    if (m_config.useInterrupts) {
        impl->rxThreadRunning = true;
        impl->rxThread = std::thread([this]() { handleDataReception(); });
    }

    m_initialized = true;
    FMUS_LOG_INFO("UART port " + std::to_string(m_portNumber) + " initialized successfully");
    return core::makeOk();
}

bool UART::isInitialized() const {
    return m_initialized;
}

core::Result<void> UART::close() {
    if (!m_initialized) {
        return core::makeOk();
    }

    FMUS_LOG_INFO("Closing UART port " + std::to_string(m_portNumber));

    UARTImpl* impl = static_cast<UARTImpl*>(m_impl);

    // Stop receive thread
    if (impl->rxThreadRunning) {
        impl->rxThreadRunning = false;
        if (impl->rxThread.joinable()) {
            impl->rxThread.join();
        }
    }

#ifdef __linux__
    // Restore original terminal settings
    tcsetattr(impl->fd, TCSANOW, &impl->originalTermios);
    ::close(impl->fd);
#elif defined(_WIN32)
    CloseHandle(impl->hSerial);
#endif

    delete impl;
    m_impl = nullptr;
    m_initialized = false;

    FMUS_LOG_INFO("UART port " + std::to_string(m_portNumber) + " closed");
    return core::makeOk();
}

core::Result<void> UART::write(const std::vector<uint8_t>& data) {
    if (!m_initialized) {
        return core::makeError<void>(core::ErrorCode::CommInitFailed,
                                   "UART not initialized");
    }

    if (data.empty()) {
        return core::makeOk();
    }

    UARTImpl* impl = static_cast<UARTImpl*>(m_impl);

#ifdef __linux__
    ssize_t bytesWritten = ::write(impl->fd, data.data(), data.size());
    if (bytesWritten < 0) {
        impl->transmissionErrors++;
        return core::makeError<void>(core::ErrorCode::CommTransmitError,
                                   "Failed to write to UART: " + std::string(strerror(errno)));
    }

    if (static_cast<size_t>(bytesWritten) != data.size()) {
        impl->transmissionErrors++;
        return core::makeError<void>(core::ErrorCode::CommTransmitError,
                                   "Incomplete write to UART");
    }

#elif defined(_WIN32)
    DWORD bytesWritten;
    if (!WriteFile(impl->hSerial, data.data(), static_cast<DWORD>(data.size()), &bytesWritten, NULL)) {
        impl->transmissionErrors++;
        return core::makeError<void>(core::ErrorCode::CommTransmitError,
                                   "Failed to write to UART");
    }

    if (bytesWritten != data.size()) {
        impl->transmissionErrors++;
        return core::makeError<void>(core::ErrorCode::CommTransmitError,
                                   "Incomplete write to UART");
    }
#endif

    impl->bytesTransmitted += data.size();
    FMUS_LOG_DEBUG("UART write: " + std::to_string(data.size()) + " bytes");
    return core::makeOk();
}

core::Result<void> UART::write(const std::string& str) {
    std::vector<uint8_t> data(str.begin(), str.end());
    return write(data);
}

core::Result<void> UART::write(uint8_t byte) {
    std::vector<uint8_t> data = {byte};
    return write(data);
}

core::Result<std::vector<uint8_t>> UART::read(size_t maxBytes) {
    if (!m_initialized) {
        return core::makeError<std::vector<uint8_t>>(core::ErrorCode::CommInitFailed,
                                                    "UART not initialized");
    }

    if (maxBytes == 0) {
        return core::makeOk<std::vector<uint8_t>>(std::vector<uint8_t>());
    }

    UARTImpl* impl = static_cast<UARTImpl*>(m_impl);
    std::vector<uint8_t> buffer(maxBytes);

#ifdef __linux__
    ssize_t bytesRead = ::read(impl->fd, buffer.data(), maxBytes);
    if (bytesRead < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No data available
            return core::makeOk<std::vector<uint8_t>>(std::vector<uint8_t>());
        }
        impl->receptionErrors++;
        return core::makeError<std::vector<uint8_t>>(core::ErrorCode::CommReceiveError,
                                                    "Failed to read from UART: " + std::string(strerror(errno)));
    }

    buffer.resize(bytesRead);

#elif defined(_WIN32)
    DWORD bytesRead;
    if (!ReadFile(impl->hSerial, buffer.data(), static_cast<DWORD>(maxBytes), &bytesRead, NULL)) {
        impl->receptionErrors++;
        return core::makeError<std::vector<uint8_t>>(core::ErrorCode::CommReceiveError,
                                                    "Failed to read from UART");
    }

    buffer.resize(bytesRead);
#endif

    impl->bytesReceived += buffer.size();
    if (!buffer.empty()) {
        FMUS_LOG_DEBUG("UART read: " + std::to_string(buffer.size()) + " bytes");
    }

    return core::makeOk<std::vector<uint8_t>>(std::move(buffer));
}

core::Result<std::string> UART::readLine(char delimiter) {
    if (!m_initialized) {
        return core::makeError<std::string>(core::ErrorCode::CommInitFailed,
                                          "UART not initialized");
    }

    std::string line;
    auto startTime = std::chrono::steady_clock::now();

    while (true) {
        auto data = read(1);
        if (data.isError()) {
            return core::makeError<std::string>(data.error().code(), data.error().message());
        }

        if (!data.value().empty()) {
            char ch = static_cast<char>(data.value()[0]);
            if (ch == delimiter) {
                break;
            }
            line += ch;
        }

        // Check timeout
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - startTime).count();
        if (elapsed >= m_config.timeoutMs) {
            if (line.empty()) {
                return core::makeError<std::string>(core::ErrorCode::Timeout,
                                                  "Timeout reading line from UART");
            }
            break; // Return partial line
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return core::makeOk<std::string>(std::move(line));
}

core::Result<size_t> UART::available() const {
    if (!m_initialized) {
        return core::makeError<size_t>(core::ErrorCode::CommInitFailed,
                                     "UART not initialized");
    }

    UARTImpl* impl = static_cast<UARTImpl*>(m_impl);

#ifdef __linux__
    int bytesAvailable;
    if (ioctl(impl->fd, FIONREAD, &bytesAvailable) < 0) {
        return core::makeError<size_t>(core::ErrorCode::CommReceiveError,
                                     "Failed to check available bytes");
    }
    return core::makeOk<size_t>(static_cast<size_t>(bytesAvailable));

#elif defined(_WIN32)
    COMSTAT comStat;
    DWORD errors;
    if (!ClearCommError(impl->hSerial, &errors, &comStat)) {
        return core::makeError<size_t>(core::ErrorCode::CommReceiveError,
                                     "Failed to check available bytes");
    }
    return core::makeOk<size_t>(static_cast<size_t>(comStat.cbInQue));
#endif
}

core::Result<void> UART::flush() {
    if (!m_initialized) {
        return core::makeError<void>(core::ErrorCode::CommInitFailed,
                                   "UART not initialized");
    }

    UARTImpl* impl = static_cast<UARTImpl*>(m_impl);

#ifdef __linux__
    if (tcdrain(impl->fd) != 0) {
        return core::makeError<void>(core::ErrorCode::CommTransmitError,
                                   "Failed to flush UART");
    }
#elif defined(_WIN32)
    if (!FlushFileBuffers(impl->hSerial)) {
        return core::makeError<void>(core::ErrorCode::CommTransmitError,
                                   "Failed to flush UART");
    }
#endif

    return core::makeOk();
}

core::Result<void> UART::setBaudRate(uint32_t baudRate) {
    if (!m_initialized) {
        return core::makeError<void>(core::ErrorCode::CommInitFailed,
                                   "UART not initialized");
    }

    m_config.baudRate = baudRate;
    return applyConfig();
}

core::Result<void> UART::setTimeout(uint16_t timeoutMs) {
    m_config.timeoutMs = timeoutMs;
    if (m_initialized) {
        return applyConfig();
    }
    return core::makeOk();
}

uint8_t UART::getPortNumber() const {
    return m_portNumber;
}

const UARTConfig& UART::getConfig() const {
    return m_config;
}

bool UART::isTransmitting() const {
    // For simplicity, always return false in this implementation
    // In a real implementation, this would check hardware status
    return false;
}

std::string UART::getStatistics() const {
    if (!m_impl) {
        return "UART not initialized";
    }

    UARTImpl* impl = static_cast<UARTImpl*>(m_impl);
    std::ostringstream oss;
    oss << "UART Port " << static_cast<int>(m_portNumber) << " Statistics:\n";
    oss << "  Bytes Transmitted: " << impl->bytesTransmitted << "\n";
    oss << "  Bytes Received: " << impl->bytesReceived << "\n";
    oss << "  Transmission Errors: " << impl->transmissionErrors << "\n";
    oss << "  Reception Errors: " << impl->receptionErrors << "\n";
    oss << "  Baud Rate: " << m_config.baudRate << "\n";
    oss << "  Data Bits: " << static_cast<int>(m_config.dataBits) << "\n";
    oss << "  Parity: " << uartParityToString(m_config.parity) << "\n";
    oss << "  Stop Bits: " << uartStopBitsToString(m_config.stopBits);
    return oss.str();
}

core::Result<void> UART::writeAsync(const std::vector<uint8_t>& data, UARTCallback callback) {
    // For simplicity, perform synchronous write and call callback
    // In a real implementation, this would use async I/O
    auto result = write(data);
    if (callback) {
        callback(result);
    }
    return core::makeOk();
}

core::Result<void> UART::setDataCallback(UARTDataCallback callback) {
    m_dataCallback = callback;
    return core::makeOk();
}

core::Result<void> UART::clearRxBuffer() {
    if (!m_initialized) {
        return core::makeError<void>(core::ErrorCode::CommInitFailed,
                                   "UART not initialized");
    }

    UARTImpl* impl = static_cast<UARTImpl*>(m_impl);

#ifdef __linux__
    if (tcflush(impl->fd, TCIFLUSH) != 0) {
        return core::makeError<void>(core::ErrorCode::CommReceiveError,
                                   "Failed to clear receive buffer");
    }
#elif defined(_WIN32)
    if (!PurgeComm(impl->hSerial, PURGE_RXCLEAR)) {
        return core::makeError<void>(core::ErrorCode::CommReceiveError,
                                   "Failed to clear receive buffer");
    }
#endif

    impl->rxBuffer.clear();
    return core::makeOk();
}

core::Result<void> UART::applyConfig() {
    if (!m_impl) {
        return core::makeError<void>(core::ErrorCode::CommInitFailed,
                                   "UART implementation not initialized");
    }

    UARTImpl* impl = static_cast<UARTImpl*>(m_impl);

#ifdef __linux__
    struct termios tty;
    if (tcgetattr(impl->fd, &tty) != 0) {
        return core::makeError<void>(core::ErrorCode::CommInitFailed,
                                   "Failed to get terminal attributes");
    }

    // Set baud rate
    speed_t speed;
    switch (m_config.baudRate) {
        case 9600: speed = B9600; break;
        case 19200: speed = B19200; break;
        case 38400: speed = B38400; break;
        case 57600: speed = B57600; break;
        case 115200: speed = B115200; break;
        case 230400: speed = B230400; break;
        case 460800: speed = B460800; break;
        case 921600: speed = B921600; break;
        default:
            return core::makeError<void>(core::ErrorCode::InvalidArgument,
                                       "Unsupported baud rate: " + std::to_string(m_config.baudRate));
    }

    cfsetispeed(&tty, speed);
    cfsetospeed(&tty, speed);

    // Set data bits
    tty.c_cflag &= ~CSIZE;
    switch (m_config.dataBits) {
        case UARTDataBits::Five: tty.c_cflag |= CS5; break;
        case UARTDataBits::Six: tty.c_cflag |= CS6; break;
        case UARTDataBits::Seven: tty.c_cflag |= CS7; break;
        case UARTDataBits::Eight: tty.c_cflag |= CS8; break;
    }

    // Set parity
    switch (m_config.parity) {
        case UARTParity::None:
            tty.c_cflag &= ~PARENB;
            break;
        case UARTParity::Even:
            tty.c_cflag |= PARENB;
            tty.c_cflag &= ~PARODD;
            break;
        case UARTParity::Odd:
            tty.c_cflag |= PARENB;
            tty.c_cflag |= PARODD;
            break;
        default:
            return core::makeError<void>(core::ErrorCode::InvalidArgument,
                                       "Unsupported parity setting");
    }

    // Set stop bits
    switch (m_config.stopBits) {
        case UARTStopBits::One:
            tty.c_cflag &= ~CSTOPB;
            break;
        case UARTStopBits::Two:
            tty.c_cflag |= CSTOPB;
            break;
        default:
            return core::makeError<void>(core::ErrorCode::InvalidArgument,
                                       "Unsupported stop bits setting");
    }

    // Configure for raw mode
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_iflag &= ~(IXON | IXOFF | IXANY | ICRNL);
    tty.c_oflag &= ~OPOST;

    // Set timeouts
    tty.c_cc[VMIN] = 0;  // Non-blocking read
    tty.c_cc[VTIME] = m_config.timeoutMs / 100;  // Timeout in deciseconds

    // Apply settings
    if (tcsetattr(impl->fd, TCSANOW, &tty) != 0) {
        return core::makeError<void>(core::ErrorCode::CommInitFailed,
                                   "Failed to set terminal attributes");
    }

#elif defined(_WIN32)
    // Windows implementation
    if (!GetCommState(impl->hSerial, &impl->dcbSerialParams)) {
        return core::makeError<void>(core::ErrorCode::CommInitFailed,
                                   "Failed to get comm state");
    }

    impl->dcbSerialParams.BaudRate = m_config.baudRate;
    impl->dcbSerialParams.ByteSize = static_cast<BYTE>(m_config.dataBits);

    switch (m_config.parity) {
        case UARTParity::None: impl->dcbSerialParams.Parity = NOPARITY; break;
        case UARTParity::Even: impl->dcbSerialParams.Parity = EVENPARITY; break;
        case UARTParity::Odd: impl->dcbSerialParams.Parity = ODDPARITY; break;
        case UARTParity::Mark: impl->dcbSerialParams.Parity = MARKPARITY; break;
        case UARTParity::Space: impl->dcbSerialParams.Parity = SPACEPARITY; break;
    }

    switch (m_config.stopBits) {
        case UARTStopBits::One: impl->dcbSerialParams.StopBits = ONESTOPBIT; break;
        case UARTStopBits::OneAndHalf: impl->dcbSerialParams.StopBits = ONE5STOPBITS; break;
        case UARTStopBits::Two: impl->dcbSerialParams.StopBits = TWOSTOPBITS; break;
    }

    if (!SetCommState(impl->hSerial, &impl->dcbSerialParams)) {
        return core::makeError<void>(core::ErrorCode::CommInitFailed,
                                   "Failed to set comm state");
    }

    // Set timeouts
    impl->timeouts.ReadIntervalTimeout = 50;
    impl->timeouts.ReadTotalTimeoutConstant = m_config.timeoutMs;
    impl->timeouts.ReadTotalTimeoutMultiplier = 10;
    impl->timeouts.WriteTotalTimeoutConstant = 50;
    impl->timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(impl->hSerial, &impl->timeouts)) {
        return core::makeError<void>(core::ErrorCode::CommInitFailed,
                                   "Failed to set comm timeouts");
    }
#endif

    return core::makeOk();
}

void UART::handleDataReception() {
    UARTImpl* impl = static_cast<UARTImpl*>(m_impl);

    while (impl->rxThreadRunning) {
        auto data = read(256);  // Read up to 256 bytes
        if (data.isOk() && !data.value().empty() && m_dataCallback) {
            m_dataCallback(data.value());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// Helper function implementations
std::string uartParityToString(UARTParity parity) {
    switch (parity) {
        case UARTParity::None: return "None";
        case UARTParity::Even: return "Even";
        case UARTParity::Odd: return "Odd";
        case UARTParity::Mark: return "Mark";
        case UARTParity::Space: return "Space";
        default: return "Unknown";
    }
}

std::string uartStopBitsToString(UARTStopBits stopBits) {
    switch (stopBits) {
        case UARTStopBits::One: return "1";
        case UARTStopBits::OneAndHalf: return "1.5";
        case UARTStopBits::Two: return "2";
        default: return "Unknown";
    }
}

std::string uartFlowControlToString(UARTFlowControl flowControl) {
    switch (flowControl) {
        case UARTFlowControl::None: return "None";
        case UARTFlowControl::RTS_CTS: return "RTS/CTS";
        case UARTFlowControl::XON_XOFF: return "XON/XOFF";
        default: return "Unknown";
    }
}

} // namespace comms
} // namespace fmus
