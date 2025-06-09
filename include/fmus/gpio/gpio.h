#ifndef FMUS_GPIO_GPIO_H
#define FMUS_GPIO_GPIO_H

#include <string>
#include <fmus/core/result.h>

namespace fmus {
namespace gpio {

/**
 * @brief Enum for GPIO pin direction
 */
enum class GPIODirection {
    Input,
    Output
};

/**
 * @brief Convert GPIODirection to string
 * @param direction The direction to convert
 * @return String representation of the direction
 */
inline std::string gpioDirectionToString(GPIODirection direction) {
    switch (direction) {
        case GPIODirection::Input:
            return "Input";
        case GPIODirection::Output:
            return "Output";
        default:
            return "Unknown";
    }
}

/**
 * @brief Enum for GPIO edge detection
 */
enum class GPIOEdge {
    None,
    Rising,
    Falling,
    Both
};

/**
 * @brief Convert GPIOEdge to string
 * @param edge The edge to convert
 * @return String representation of the edge
 */
inline std::string gpioEdgeToString(GPIOEdge edge) {
    switch (edge) {
        case GPIOEdge::None:
            return "None";
        case GPIOEdge::Rising:
            return "Rising";
        case GPIOEdge::Falling:
            return "Falling";
        case GPIOEdge::Both:
            return "Both";
        default:
            return "Unknown";
    }
}

/**
 * @brief Enum for GPIO pull-up/down resistors
 */
enum class GPIOPull {
    None,
    Up,
    Down
};

/**
 * @brief Convert GPIOPull to string
 * @param pull The pull to convert
 * @return String representation of the pull
 */
inline std::string gpioPullToString(GPIOPull pull) {
    switch (pull) {
        case GPIOPull::None:
            return "None";
        case GPIOPull::Up:
            return "Pull-Up";
        case GPIOPull::Down:
            return "Pull-Down";
        default:
            return "Unknown";
    }
}

/**
 * @brief Class for managing GPIO pins
 */
class GPIO {
public:
    /**
     * @brief Constructor
     * @param pin The GPIO pin number
     */
    GPIO(unsigned int pin);

    /**
     * @brief Destructor
     */
    ~GPIO();

    /**
     * @brief Initialize the GPIO pin
     * @param direction The pin direction (input or output)
     * @return Result indicating success or failure
     */
    core::Result<void> init(GPIODirection direction = GPIODirection::Output);

    /**
     * @brief Check if the GPIO pin is initialized
     * @return True if initialized, false otherwise
     */
    bool isInitialized() const;

    /**
     * @brief Get the pin number
     * @return The pin number
     */
    unsigned int getPin() const;

    /**
     * @brief Set the pin direction
     * @param direction The direction to set
     * @return Result indicating success or failure
     */
    core::Result<void> setDirection(GPIODirection direction);

    /**
     * @brief Get the pin direction
     * @return The pin direction
     */
    GPIODirection getDirection() const;

    /**
     * @brief Set the edge detection
     * @param edge The edge detection to set
     * @return Result indicating success or failure
     */
    core::Result<void> setEdge(GPIOEdge edge);

    /**
     * @brief Get the edge detection
     * @return The edge detection
     */
    GPIOEdge getEdge() const;

    /**
     * @brief Set the pull-up/down resistor
     * @param pull The pull resistor to set
     * @return Result indicating success or failure
     */
    core::Result<void> setPull(GPIOPull pull);

    /**
     * @brief Get the pull-up/down resistor
     * @return The pull resistor
     */
    GPIOPull getPull() const;

    /**
     * @brief Write to the GPIO pin
     * @param value The value to write (true for high, false for low)
     * @return Result indicating success or failure
     */
    core::Result<void> write(bool value);

    /**
     * @brief Read from the GPIO pin
     * @return Result containing the pin value (true for high, false for low)
     */
    core::Result<bool> read() const;

    /**
     * @brief Attach an interrupt handler to the GPIO pin
     * @param edge The edge to trigger on
     * @param callback The function to call when the interrupt is triggered
     * @return Result indicating success or failure
     */
    template<typename CallbackType>
    core::Result<void> attachInterrupt(GPIOEdge edge, CallbackType callback);

    /**
     * @brief Detach the interrupt handler from the GPIO pin
     * @return Result indicating success or failure
     */
    core::Result<void> detachInterrupt();

private:
    unsigned int m_pin;
    bool m_initialized;
    GPIODirection m_direction;
    GPIOEdge m_edge;
    GPIOPull m_pull;
    void* m_impl; // Platform-specific implementation
};

} // namespace gpio
} // namespace fmus

#endif // FMUS_GPIO_GPIO_H
