#pragma once

/**
 * @file relay.h
 * @brief Relay control interface for the fmus-embed library
 *
 * This header provides relay control functionality with support for
 * various relay types, timing controls, and safety features.
 */

#include "../fmus_config.h"
#include "../core/result.h"
#include <cstdint>
#include <functional>
#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include <tuple>

namespace fmus {
namespace actuators {

/**
 * @brief Relay types enumeration
 */
enum class RelayType : uint8_t {
    NormallyOpen = 0,    ///< Normally open relay
    NormallyClosed = 1,  ///< Normally closed relay
    SPDT = 2,           ///< Single pole double throw
    DPDT = 3            ///< Double pole double throw
};

/**
 * @brief Relay state enumeration
 */
enum class RelayState : uint8_t {
    Off = 0,    ///< Relay off (de-energized)
    On = 1      ///< Relay on (energized)
};

/**
 * @brief Relay configuration structure
 */
struct RelayConfig {
    RelayType type;                 ///< Relay type
    bool invertLogic;               ///< Invert control logic
    uint32_t switchingDelayMs;      ///< Delay between switching operations
    uint32_t maxSwitchingFrequency; ///< Maximum switching frequency (Hz)
    bool enableSafetyTimeout;       ///< Enable safety timeout
    uint32_t safetyTimeoutMs;       ///< Safety timeout in milliseconds

    /**
     * @brief Constructor with default values
     */
    RelayConfig(RelayType relayType = RelayType::NormallyOpen,
                bool invert = false,
                uint32_t switchDelay = 10,
                uint32_t maxFreq = 100,
                bool safetyTimeout = false,
                uint32_t timeoutMs = 60000)
        : type(relayType), invertLogic(invert), switchingDelayMs(switchDelay),
          maxSwitchingFrequency(maxFreq), enableSafetyTimeout(safetyTimeout),
          safetyTimeoutMs(timeoutMs) {}
};

/**
 * @brief Relay statistics structure
 */
struct RelayStatistics {
    uint64_t totalSwitches;         ///< Total number of switches
    uint64_t onTime;                ///< Total time in ON state (ms)
    uint64_t offTime;               ///< Total time in OFF state (ms)
    uint32_t switchingErrors;       ///< Number of switching errors
    std::chrono::steady_clock::time_point lastSwitchTime; ///< Last switch time
};

/**
 * @brief Callback function type for relay state changes
 */
using RelayCallback = std::function<void(RelayState newState, RelayState oldState)>;

/**
 * @brief Relay control class
 */
class FMUS_EMBED_API Relay {
public:
    /**
     * @brief Construct a new Relay
     *
     * @param controlPin Control pin number
     * @param config Relay configuration
     */
    explicit Relay(uint8_t controlPin, const RelayConfig& config = RelayConfig());

    /**
     * @brief Destructor
     */
    ~Relay();

    /**
     * @brief Initialize the relay
     *
     * @return core::Result<void> Success or error
     */
    core::Result<void> init();

    /**
     * @brief Check if the relay is initialized
     *
     * @return bool True if initialized
     */
    bool isInitialized() const;

    /**
     * @brief Set relay state
     *
     * @param state Desired relay state
     * @return core::Result<void> Success or error
     */
    core::Result<void> setState(RelayState state);

    /**
     * @brief Set relay state (boolean convenience method)
     *
     * @param on True for ON, false for OFF
     * @return core::Result<void> Success or error
     */
    core::Result<void> setState(bool on);

    /**
     * @brief Get current relay state
     *
     * @return RelayState Current state
     */
    RelayState getState() const;

    /**
     * @brief Get current relay state as boolean
     *
     * @return bool True if ON, false if OFF
     */
    bool isOn() const;

    /**
     * @brief Toggle relay state
     *
     * @return core::Result<void> Success or error
     */
    core::Result<void> toggle();

    /**
     * @brief Turn relay on
     *
     * @return core::Result<void> Success or error
     */
    core::Result<void> turnOn();

    /**
     * @brief Turn relay off
     *
     * @return core::Result<void> Success or error
     */
    core::Result<void> turnOff();

    /**
     * @brief Set relay on for a specific duration
     *
     * @param durationMs Duration in milliseconds
     * @param callback Optional callback when timer expires
     * @return core::Result<void> Success or error
     */
    core::Result<void> setOnForDuration(uint32_t durationMs, 
                                       std::function<void()> callback = nullptr);

    /**
     * @brief Pulse relay (turn on briefly then off)
     *
     * @param pulseDurationMs Pulse duration in milliseconds
     * @return core::Result<void> Success or error
     */
    core::Result<void> pulse(uint32_t pulseDurationMs);

    /**
     * @brief Set state change callback
     *
     * @param callback Callback function
     * @return core::Result<void> Success or error
     */
    core::Result<void> setStateChangeCallback(RelayCallback callback);

    /**
     * @brief Get relay configuration
     *
     * @return const RelayConfig& Current configuration
     */
    const RelayConfig& getConfig() const;

    /**
     * @brief Get control pin number
     *
     * @return uint8_t Control pin number
     */
    uint8_t getControlPin() const;

    /**
     * @brief Get relay statistics
     *
     * @return RelayStatistics Current statistics
     */
    RelayStatistics getStatistics() const;

    /**
     * @brief Reset statistics
     *
     * @return core::Result<void> Success or error
     */
    core::Result<void> resetStatistics();

    /**
     * @brief Check if relay can switch (respects timing constraints)
     *
     * @return bool True if can switch
     */
    bool canSwitch() const;

    /**
     * @brief Get time since last switch
     *
     * @return uint32_t Time in milliseconds
     */
    uint32_t getTimeSinceLastSwitch() const;

    /**
     * @brief Enable/disable safety timeout
     *
     * @param enabled True to enable, false to disable
     * @return core::Result<void> Success or error
     */
    core::Result<void> setSafetyTimeout(bool enabled);

    /**
     * @brief Check if safety timeout is active
     *
     * @return bool True if timeout is active
     */
    bool isSafetyTimeoutActive() const;

    /**
     * @brief Get relay status as string
     *
     * @return std::string Status information
     */
    std::string getStatus() const;

private:
    uint8_t m_controlPin;           ///< Control pin number
    RelayConfig m_config;           ///< Relay configuration
    bool m_initialized;             ///< Initialization state
    RelayState m_currentState;      ///< Current relay state
    RelayStatistics m_statistics;   ///< Relay statistics
    RelayCallback m_stateCallback;  ///< State change callback
    void* m_impl;                   ///< Platform-specific implementation

    /**
     * @brief Internal state change handler
     *
     * @param newState New relay state
     */
    void handleStateChange(RelayState newState);

    /**
     * @brief Check and enforce switching constraints
     *
     * @return core::Result<void> Success or error if constraints violated
     */
    core::Result<void> checkSwitchingConstraints();

    /**
     * @brief Update statistics
     *
     * @param newState New state
     */
    void updateStatistics(RelayState newState);
};

/**
 * @brief Multi-relay controller for coordinated relay operations
 */
class FMUS_EMBED_API RelayController {
public:
    /**
     * @brief Constructor
     */
    RelayController();

    /**
     * @brief Destructor
     */
    ~RelayController();

    /**
     * @brief Add relay to controller
     *
     * @param relay Relay to add
     * @param name Optional name for the relay
     * @return core::Result<void> Success or error
     */
    core::Result<void> addRelay(std::shared_ptr<Relay> relay, const std::string& name = "");

    /**
     * @brief Remove relay from controller
     *
     * @param name Relay name
     * @return core::Result<void> Success or error
     */
    core::Result<void> removeRelay(const std::string& name);

    /**
     * @brief Set state of named relay
     *
     * @param name Relay name
     * @param state Desired state
     * @return core::Result<void> Success or error
     */
    core::Result<void> setRelayState(const std::string& name, RelayState state);

    /**
     * @brief Turn all relays off
     *
     * @return core::Result<void> Success or error
     */
    core::Result<void> turnAllOff();

    /**
     * @brief Execute relay sequence
     *
     * @param sequence Vector of {relay_name, state, delay_ms} tuples
     * @return core::Result<void> Success or error
     */
    core::Result<void> executeSequence(const std::vector<std::tuple<std::string, RelayState, uint32_t>>& sequence);

    /**
     * @brief Get number of relays
     *
     * @return size_t Number of relays
     */
    size_t getRelayCount() const;

    /**
     * @brief Get relay by name
     *
     * @param name Relay name
     * @return std::shared_ptr<Relay> Relay pointer or nullptr
     */
    std::shared_ptr<Relay> getRelay(const std::string& name) const;

private:
    void* m_impl; ///< Implementation details
};

/**
 * @brief Get string representation of relay type
 *
 * @param type Relay type
 * @return std::string String representation
 */
FMUS_EMBED_API std::string relayTypeToString(RelayType type);

/**
 * @brief Get string representation of relay state
 *
 * @param state Relay state
 * @return std::string String representation
 */
FMUS_EMBED_API std::string relayStateToString(RelayState state);

} // namespace actuators
} // namespace fmus
