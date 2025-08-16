#include "fmus/actuators/relay.h"
#include "fmus/core/logging.h"
#include "fmus/gpio/gpio.h"
#include <sstream>
#include <thread>
#include <chrono>
#include <map>
#include <memory>

namespace fmus {
namespace actuators {

// Implementation structure for relay
struct RelayImpl {
    std::chrono::steady_clock::time_point lastSwitchTime;
    std::chrono::steady_clock::time_point stateStartTime;
    std::thread timerThread;
    bool timerActive;
    std::function<void()> timerCallback;
};

// Implementation structure for relay controller
struct RelayControllerImpl {
    std::map<std::string, std::shared_ptr<Relay>> relays;
    uint32_t nextId;
};

//=============================================================================
// Relay Implementation
//=============================================================================

Relay::Relay(uint8_t controlPin, const RelayConfig& config)
    : m_controlPin(controlPin),
      m_config(config),
      m_initialized(false),
      m_currentState(RelayState::Off),
      m_stateCallback(nullptr),
      m_impl(nullptr) {
    
    // Initialize statistics
    m_statistics.totalSwitches = 0;
    m_statistics.onTime = 0;
    m_statistics.offTime = 0;
    m_statistics.switchingErrors = 0;
    m_statistics.lastSwitchTime = std::chrono::steady_clock::now();

    m_impl = new RelayImpl();
    RelayImpl* impl = static_cast<RelayImpl*>(m_impl);
    impl->lastSwitchTime = std::chrono::steady_clock::now();
    impl->stateStartTime = impl->lastSwitchTime;
    impl->timerActive = false;
}

Relay::~Relay() {
    if (m_initialized) {
        turnOff();
    }
    
    if (m_impl) {
        RelayImpl* impl = static_cast<RelayImpl*>(m_impl);
        impl->timerActive = false;
        if (impl->timerThread.joinable()) {
            impl->timerThread.join();
        }
        delete impl;
    }
}

core::Result<void> Relay::init() {
    FMUS_LOG_INFO("Initializing relay on pin " + std::to_string(m_controlPin));

    gpio::GPIO controlGpio(m_controlPin);
    auto result = controlGpio.init(gpio::GPIODirection::Output);
    if (result.isError()) {
        return core::makeError<void>(core::ErrorCode::ActuatorInitFailed,
                                   "Failed to initialize control pin: " + result.error().message());
    }

    // Set initial state (off)
    bool initialState = m_config.invertLogic ? true : false;
    controlGpio.write(initialState);

    m_initialized = true;
    FMUS_LOG_INFO("Relay initialized successfully");
    return core::makeOk();
}

bool Relay::isInitialized() const {
    return m_initialized;
}

core::Result<void> Relay::setState(RelayState state) {
    if (!m_initialized) {
        return core::makeError<void>(core::ErrorCode::NotInitialized,
                                   "Relay not initialized");
    }

    // Check switching constraints
    auto constraintResult = checkSwitchingConstraints();
    if (constraintResult.isError()) {
        return constraintResult;
    }

    if (state == m_currentState) {
        return core::makeOk(); // No change needed
    }

    RelayState oldState = m_currentState;
    m_currentState = state;

    // Set GPIO pin state
    gpio::GPIO controlGpio(m_controlPin);
    bool pinState;

    if (state == RelayState::On) {
        pinState = m_config.invertLogic ? false : true;
    } else {
        pinState = m_config.invertLogic ? true : false;
    }

    auto writeResult = controlGpio.write(pinState);
    if (writeResult.isError()) {
        m_statistics.switchingErrors++;
        return core::makeError<void>(core::ErrorCode::ActuatorSetValueError,
                                   "Failed to set relay state: " + writeResult.error().message());
    }

    // Update statistics and handle state change
    updateStatistics(state);
    handleStateChange(state);

    // Add switching delay
    if (m_config.switchingDelayMs > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(m_config.switchingDelayMs));
    }

    FMUS_LOG_DEBUG("Relay state changed from " + relayStateToString(oldState) + 
                   " to " + relayStateToString(state));
    return core::makeOk();
}

core::Result<void> Relay::setState(bool on) {
    return setState(on ? RelayState::On : RelayState::Off);
}

RelayState Relay::getState() const {
    return m_currentState;
}

bool Relay::isOn() const {
    return m_currentState == RelayState::On;
}

core::Result<void> Relay::toggle() {
    RelayState newState = (m_currentState == RelayState::On) ? RelayState::Off : RelayState::On;
    return setState(newState);
}

core::Result<void> Relay::turnOn() {
    return setState(RelayState::On);
}

core::Result<void> Relay::turnOff() {
    return setState(RelayState::Off);
}

core::Result<void> Relay::setOnForDuration(uint32_t durationMs, std::function<void()> callback) {
    if (!m_initialized) {
        return core::makeError<void>(core::ErrorCode::NotInitialized,
                                   "Relay not initialized");
    }

    // Turn on the relay
    auto onResult = turnOn();
    if (onResult.isError()) {
        return onResult;
    }

    // Start timer thread
    RelayImpl* impl = static_cast<RelayImpl*>(m_impl);
    impl->timerActive = true;
    impl->timerCallback = callback;

    if (impl->timerThread.joinable()) {
        impl->timerThread.join();
    }

    impl->timerThread = std::thread([this, durationMs]() {
        RelayImpl* impl = static_cast<RelayImpl*>(m_impl);
        std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
        
        if (impl->timerActive) {
            turnOff();
            if (impl->timerCallback) {
                impl->timerCallback();
            }
        }
        impl->timerActive = false;
    });

    FMUS_LOG_DEBUG("Relay set on for " + std::to_string(durationMs) + "ms");
    return core::makeOk();
}

core::Result<void> Relay::pulse(uint32_t pulseDurationMs) {
    return setOnForDuration(pulseDurationMs);
}

core::Result<void> Relay::setStateChangeCallback(RelayCallback callback) {
    m_stateCallback = callback;
    return core::makeOk();
}

const RelayConfig& Relay::getConfig() const {
    return m_config;
}

uint8_t Relay::getControlPin() const {
    return m_controlPin;
}

RelayStatistics Relay::getStatistics() const {
    return m_statistics;
}

core::Result<void> Relay::resetStatistics() {
    m_statistics.totalSwitches = 0;
    m_statistics.onTime = 0;
    m_statistics.offTime = 0;
    m_statistics.switchingErrors = 0;
    m_statistics.lastSwitchTime = std::chrono::steady_clock::now();
    
    FMUS_LOG_DEBUG("Relay statistics reset");
    return core::makeOk();
}

bool Relay::canSwitch() const {
    if (!m_config.maxSwitchingFrequency) {
        return true; // No frequency limit
    }

    RelayImpl* impl = static_cast<RelayImpl*>(m_impl);
    auto now = std::chrono::steady_clock::now();
    auto timeSinceLastSwitch = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - impl->lastSwitchTime).count();

    uint32_t minInterval = 1000 / m_config.maxSwitchingFrequency; // ms
    return timeSinceLastSwitch >= minInterval;
}

uint32_t Relay::getTimeSinceLastSwitch() const {
    RelayImpl* impl = static_cast<RelayImpl*>(m_impl);
    auto now = std::chrono::steady_clock::now();
    return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        now - impl->lastSwitchTime).count());
}

core::Result<void> Relay::setSafetyTimeout(bool enabled) {
    m_config.enableSafetyTimeout = enabled;
    FMUS_LOG_DEBUG("Relay safety timeout " + std::string(enabled ? "enabled" : "disabled"));
    return core::makeOk();
}

bool Relay::isSafetyTimeoutActive() const {
    if (!m_config.enableSafetyTimeout || m_currentState == RelayState::Off) {
        return false;
    }

    RelayImpl* impl = static_cast<RelayImpl*>(m_impl);
    auto now = std::chrono::steady_clock::now();
    auto timeInCurrentState = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - impl->stateStartTime).count();

    return timeInCurrentState >= m_config.safetyTimeoutMs;
}

std::string Relay::getStatus() const {
    std::ostringstream oss;
    oss << "Relay Status:\n";
    oss << "  Control Pin: " << static_cast<int>(m_controlPin) << "\n";
    oss << "  Type: " << relayTypeToString(m_config.type) << "\n";
    oss << "  State: " << relayStateToString(m_currentState) << "\n";
    oss << "  Initialized: " << (m_initialized ? "Yes" : "No") << "\n";
    oss << "  Invert Logic: " << (m_config.invertLogic ? "Yes" : "No") << "\n";
    oss << "  Total Switches: " << m_statistics.totalSwitches << "\n";
    oss << "  Switching Errors: " << m_statistics.switchingErrors << "\n";
    oss << "  Time Since Last Switch: " << getTimeSinceLastSwitch() << "ms\n";
    oss << "  Safety Timeout Active: " << (isSafetyTimeoutActive() ? "Yes" : "No");
    return oss.str();
}

void Relay::handleStateChange(RelayState newState) {
    if (m_stateCallback) {
        RelayState oldState = (newState == RelayState::On) ? RelayState::Off : RelayState::On;
        m_stateCallback(newState, oldState);
    }
}

core::Result<void> Relay::checkSwitchingConstraints() {
    if (!canSwitch()) {
        return core::makeError<void>(core::ErrorCode::ActuatorSetValueError,
                                   "Switching too fast - frequency limit exceeded");
    }

    if (isSafetyTimeoutActive()) {
        // Auto turn off due to safety timeout
        FMUS_LOG_WARNING("Safety timeout triggered - turning relay off");
        m_currentState = RelayState::Off;
        gpio::GPIO controlGpio(m_controlPin);
        bool safeState = m_config.invertLogic ? true : false;
        controlGpio.write(safeState);
    }

    return core::makeOk();
}

void Relay::updateStatistics(RelayState newState) {
    RelayImpl* impl = static_cast<RelayImpl*>(m_impl);
    auto now = std::chrono::steady_clock::now();
    
    // Update time in previous state
    auto timeInPreviousState = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - impl->stateStartTime).count();
    
    if (m_currentState == RelayState::On) {
        m_statistics.onTime += timeInPreviousState;
    } else {
        m_statistics.offTime += timeInPreviousState;
    }

    // Update switch count and timestamps
    m_statistics.totalSwitches++;
    m_statistics.lastSwitchTime = now;
    impl->lastSwitchTime = now;
    impl->stateStartTime = now;
}

//=============================================================================
// RelayController Implementation
//=============================================================================

RelayController::RelayController() : m_impl(nullptr) {
    m_impl = new RelayControllerImpl();
    RelayControllerImpl* impl = static_cast<RelayControllerImpl*>(m_impl);
    impl->nextId = 1;
}

RelayController::~RelayController() {
    if (m_impl) {
        delete static_cast<RelayControllerImpl*>(m_impl);
    }
}

core::Result<void> RelayController::addRelay(std::shared_ptr<Relay> relay, const std::string& name) {
    if (!relay) {
        return core::makeError<void>(core::ErrorCode::InvalidArgument,
                                   "Relay pointer is null");
    }

    RelayControllerImpl* impl = static_cast<RelayControllerImpl*>(m_impl);
    std::string relayName = name.empty() ? ("relay_" + std::to_string(impl->nextId++)) : name;

    if (impl->relays.find(relayName) != impl->relays.end()) {
        return core::makeError<void>(core::ErrorCode::InvalidArgument,
                                   "Relay with name '" + relayName + "' already exists");
    }

    impl->relays[relayName] = relay;
    FMUS_LOG_INFO("Added relay '" + relayName + "' to controller");
    return core::makeOk();
}

core::Result<void> RelayController::removeRelay(const std::string& name) {
    RelayControllerImpl* impl = static_cast<RelayControllerImpl*>(m_impl);
    auto it = impl->relays.find(name);

    if (it == impl->relays.end()) {
        return core::makeError<void>(core::ErrorCode::InvalidArgument,
                                   "Relay '" + name + "' not found");
    }

    // Turn off relay before removing
    it->second->turnOff();
    impl->relays.erase(it);

    FMUS_LOG_INFO("Removed relay '" + name + "' from controller");
    return core::makeOk();
}

core::Result<void> RelayController::setRelayState(const std::string& name, RelayState state) {
    RelayControllerImpl* impl = static_cast<RelayControllerImpl*>(m_impl);
    auto it = impl->relays.find(name);

    if (it == impl->relays.end()) {
        return core::makeError<void>(core::ErrorCode::InvalidArgument,
                                   "Relay '" + name + "' not found");
    }

    return it->second->setState(state);
}

core::Result<void> RelayController::turnAllOff() {
    RelayControllerImpl* impl = static_cast<RelayControllerImpl*>(m_impl);

    for (auto& pair : impl->relays) {
        auto result = pair.second->turnOff();
        if (result.isError()) {
            FMUS_LOG_ERROR("Failed to turn off relay '" + pair.first + "': " + result.error().message());
        }
    }

    FMUS_LOG_INFO("All relays turned off");
    return core::makeOk();
}

core::Result<void> RelayController::executeSequence(const std::vector<std::tuple<std::string, RelayState, uint32_t>>& sequence) {
    RelayControllerImpl* impl = static_cast<RelayControllerImpl*>(m_impl);

    for (const auto& step : sequence) {
        const std::string& relayName = std::get<0>(step);
        RelayState state = std::get<1>(step);
        uint32_t delay = std::get<2>(step);

        auto it = impl->relays.find(relayName);
        if (it == impl->relays.end()) {
            return core::makeError<void>(core::ErrorCode::InvalidArgument,
                                       "Relay '" + relayName + "' not found in sequence");
        }

        auto result = it->second->setState(state);
        if (result.isError()) {
            return result;
        }

        if (delay > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        }
    }

    FMUS_LOG_INFO("Relay sequence executed successfully");
    return core::makeOk();
}

size_t RelayController::getRelayCount() const {
    RelayControllerImpl* impl = static_cast<RelayControllerImpl*>(m_impl);
    return impl->relays.size();
}

std::shared_ptr<Relay> RelayController::getRelay(const std::string& name) const {
    RelayControllerImpl* impl = static_cast<RelayControllerImpl*>(m_impl);
    auto it = impl->relays.find(name);
    return (it != impl->relays.end()) ? it->second : nullptr;
}

//=============================================================================
// Helper Functions
//=============================================================================

std::string relayTypeToString(RelayType type) {
    switch (type) {
        case RelayType::NormallyOpen: return "Normally Open";
        case RelayType::NormallyClosed: return "Normally Closed";
        case RelayType::SPDT: return "SPDT";
        case RelayType::DPDT: return "DPDT";
        default: return "Unknown";
    }
}

std::string relayStateToString(RelayState state) {
    switch (state) {
        case RelayState::Off: return "Off";
        case RelayState::On: return "On";
        default: return "Unknown";
    }
}

} // namespace actuators
} // namespace fmus
