#include "fmus/gpio/gpio.h"
#include "fmus/core/error.h"
#include "fmus/core/result.h"
#include "fmus/core/logging.h"
#include <cstring>
#include <iostream>

// Implementasi platform-specific
#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
// Simulasi GPIO untuk Windows, karena Windows tidak memiliki GPIO sebenarnya
#elif defined(__linux__)
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

namespace fmus {
namespace gpio {

// Platform-specific implementation structs
struct GPIOImpl {
#if defined(_WIN32) || defined(_WIN64)
    // Simulasi status GPIO untuk Windows
    bool state;
#elif defined(__linux__)
    int value_fd;
    int direction_fd;
    int edge_fd;
#endif
};

GPIO::GPIO(unsigned int pin)
    : m_pin(pin)
    , m_initialized(false)
    , m_direction(GPIODirection::Output)
    , m_edge(GPIOEdge::None)
    , m_pull(GPIOPull::None)
    , m_impl(nullptr) {
}

GPIO::~GPIO() {
    if (m_initialized && m_impl) {
        GPIOImpl* impl = static_cast<GPIOImpl*>(m_impl);

#if defined(_WIN32) || defined(_WIN64)
        // Nothing to clean up for Windows simulation
        delete impl;
#elif defined(__linux__)
        // Close file descriptors
        if (impl->value_fd >= 0) {
            close(impl->value_fd);
        }
        if (impl->direction_fd >= 0) {
            close(impl->direction_fd);
        }
        if (impl->edge_fd >= 0) {
            close(impl->edge_fd);
        }

        // Unexport the GPIO pin
        int unexport_fd = open("/sys/class/gpio/unexport", O_WRONLY);
        if (unexport_fd >= 0) {
            char pin_str[8];
            int len = snprintf(pin_str, sizeof(pin_str), "%u", m_pin);
            write(unexport_fd, pin_str, len);
            close(unexport_fd);
        }
        delete impl;
#endif

        m_impl = nullptr;
        m_initialized = false;
    }
}

core::Result<void> GPIO::init(GPIODirection direction) {
    if (m_initialized) {
        // Reinitialization, clean up first
        GPIOImpl* impl = static_cast<GPIOImpl*>(m_impl);

#if defined(_WIN32) || defined(_WIN64)
        // Nothing to clean up for Windows simulation
#elif defined(__linux__)
        // Close file descriptors
        if (impl->value_fd >= 0) {
            close(impl->value_fd);
            impl->value_fd = -1;
        }
        if (impl->direction_fd >= 0) {
            close(impl->direction_fd);
            impl->direction_fd = -1;
        }
        if (impl->edge_fd >= 0) {
            close(impl->edge_fd);
            impl->edge_fd = -1;
        }
#endif
    } else {
        // First initialization, create impl
        m_impl = new GPIOImpl();
    }

    GPIOImpl* impl = static_cast<GPIOImpl*>(m_impl);

#if defined(_WIN32) || defined(_WIN64)
    // Simulasi untuk Windows
    impl->state = false;
    m_direction = direction;
    m_initialized = true;
#elif defined(__linux__)
    // Setup for Linux
    char path[64];

    // Export the GPIO pin if not already exported
    int export_fd = open("/sys/class/gpio/export", O_WRONLY);
    if (export_fd < 0) {
        return core::Error(core::ErrorCode::GPIOError, "Failed to open export file");
    }

    char pin_str[8];
    int len = snprintf(pin_str, sizeof(pin_str), "%u", m_pin);
    write(export_fd, pin_str, len);
    close(export_fd);

    // Give the system time to create the GPIO files
    usleep(100000); // 100ms

    // Open direction file
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%u/direction", m_pin);
    impl->direction_fd = open(path, O_RDWR);
    if (impl->direction_fd < 0) {
        return core::Error(core::ErrorCode::GPIOError, "Failed to open direction file");
    }

    // Set direction
    const char* dir_str = (direction == GPIODirection::Output) ? "out" : "in";
    write(impl->direction_fd, dir_str, strlen(dir_str));

    // Open value file
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%u/value", m_pin);
    impl->value_fd = open(path, O_RDWR);
    if (impl->value_fd < 0) {
        close(impl->direction_fd);
        impl->direction_fd = -1;
        return core::Error(core::ErrorCode::GPIOError, "Failed to open value file");
    }

    // Open edge file
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%u/edge", m_pin);
    impl->edge_fd = open(path, O_RDWR);
    if (impl->edge_fd < 0) {
        // Edge file might not be available for all GPIOs, that's OK
        impl->edge_fd = -1;
    }

    m_direction = direction;
    m_initialized = true;
#endif

    return core::Result<void>();
}

bool GPIO::isInitialized() const {
    return m_initialized;
}

unsigned int GPIO::getPin() const {
    return m_pin;
}

core::Result<void> GPIO::setDirection(GPIODirection direction) {
    if (!m_initialized) {
        return core::Error(core::ErrorCode::GPIOError, "GPIO pin not initialized");
    }

    GPIOImpl* impl = static_cast<GPIOImpl*>(m_impl);

#if defined(_WIN32) || defined(_WIN64)
    // Simulasi untuk Windows
    m_direction = direction;
#elif defined(__linux__)
    if (impl->direction_fd < 0) {
        return core::Error(core::ErrorCode::GPIOError, "Direction file not opened");
    }

    const char* dir_str = (direction == GPIODirection::Output) ? "out" : "in";
    lseek(impl->direction_fd, 0, SEEK_SET);
    write(impl->direction_fd, dir_str, strlen(dir_str));
    m_direction = direction;
#endif

    return core::Result<void>();
}

GPIODirection GPIO::getDirection() const {
    return m_direction;
}

core::Result<void> GPIO::setEdge(GPIOEdge edge) {
    if (!m_initialized) {
        return core::Error(core::ErrorCode::GPIOError, "GPIO pin not initialized");
    }

    GPIOImpl* impl = static_cast<GPIOImpl*>(m_impl);

#if defined(_WIN32) || defined(_WIN64)
    // Simulasi untuk Windows
    m_edge = edge;
#elif defined(__linux__)
    if (impl->edge_fd < 0) {
        return core::Error(core::ErrorCode::GPIOError, "Edge file not available");
    }

    const char* edge_str;
    switch (edge) {
        case GPIOEdge::None:
            edge_str = "none";
            break;
        case GPIOEdge::Rising:
            edge_str = "rising";
            break;
        case GPIOEdge::Falling:
            edge_str = "falling";
            break;
        case GPIOEdge::Both:
            edge_str = "both";
            break;
        default:
            return core::Error(core::ErrorCode::GPIOError, "Invalid edge value");
    }

    lseek(impl->edge_fd, 0, SEEK_SET);
    write(impl->edge_fd, edge_str, strlen(edge_str));
    m_edge = edge;
#endif

    return core::Result<void>();
}

GPIOEdge GPIO::getEdge() const {
    return m_edge;
}

core::Result<void> GPIO::setPull(GPIOPull pull) {
    if (!m_initialized) {
        return core::Error(core::ErrorCode::GPIOError, "GPIO pin not initialized");
    }

    // Pull-up/pull-down configuration is platform-specific
    // and may require additional hardware-specific setup
#if defined(_WIN32) || defined(_WIN64)
    // Simulasi untuk Windows
    m_pull = pull;
#elif defined(__linux__)
    // On Linux, pull-up/down usually requires additional configuration
    // beyond the sysfs interface. This depends on the platform.
    m_pull = pull;
#endif

    return core::Result<void>();
}

GPIOPull GPIO::getPull() const {
    return m_pull;
}

core::Result<void> GPIO::write(bool value) {
    if (!m_initialized) {
        return core::Error(core::ErrorCode::GPIOError, "GPIO pin not initialized");
    }

    if (m_direction != GPIODirection::Output) {
        return core::Error(core::ErrorCode::GPIOError, "GPIO pin not configured as output");
    }

    GPIOImpl* impl = static_cast<GPIOImpl*>(m_impl);

#if defined(_WIN32) || defined(_WIN64)
    // Simulasi untuk Windows
    impl->state = value;
#elif defined(__linux__)
    if (impl->value_fd < 0) {
        return core::Error(core::ErrorCode::GPIOError, "Value file not opened");
    }

    const char val_str = value ? '1' : '0';
    lseek(impl->value_fd, 0, SEEK_SET);
    write(impl->value_fd, &val_str, 1);
#endif

    return core::Result<void>();
}

core::Result<bool> GPIO::read() const {
    if (!m_initialized) {
        return core::Error(core::ErrorCode::GPIOError, "GPIO pin not initialized");
    }

    GPIOImpl* impl = static_cast<GPIOImpl*>(m_impl);

#if defined(_WIN32) || defined(_WIN64)
    // Simulasi untuk Windows
    return core::Result<bool>(impl->state);
#elif defined(__linux__)
    if (impl->value_fd < 0) {
        return core::Error(core::ErrorCode::GPIOError, "Value file not opened");
    }

    char val_str;
    lseek(impl->value_fd, 0, SEEK_SET);
    int ret = ::read(impl->value_fd, &val_str, 1);

    if (ret != 1) {
        return core::Error(core::ErrorCode::GPIOError, "Failed to read value");
    }

    return core::Result<bool>(val_str == '1');
#else
    // Generic implementation for other platforms
    return core::Result<bool>(false);
#endif
}

template<typename CallbackType>
core::Result<void> GPIO::attachInterrupt(GPIOEdge edge, CallbackType callback) {
    if (!m_initialized) {
        return core::Error(core::ErrorCode::GPIOError, "GPIO pin not initialized");
    }

    if (m_direction != GPIODirection::Input) {
        return core::Error(core::ErrorCode::GPIOError, "GPIO pin not configured as input");
    }

    // Set the edge detection
    auto result = setEdge(edge);
    if (result.isError()) {
        return result;
    }

    // Platform-specific interrupt setup would go here
    // This is a simplified implementation

    return core::Result<void>();
}

core::Result<void> GPIO::detachInterrupt() {
    if (!m_initialized) {
        return core::Error(core::ErrorCode::GPIOError, "GPIO pin not initialized");
    }

    // Disable edge detection
    auto result = setEdge(GPIOEdge::None);
    if (result.isError()) {
        return result;
    }

    // Platform-specific interrupt cleanup would go here

    return core::Result<void>();
}

} // namespace gpio
} // namespace fmus

