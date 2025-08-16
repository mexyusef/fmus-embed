#include "fmus/actuators/actuators.h"
#include "fmus/core/logging.h"
#include <vector>
#include <memory>
#include <sstream>

namespace fmus {
namespace actuators {

// Global state for actuators module
static bool g_actuatorsInitialized = false;
static std::vector<std::shared_ptr<IMotor>> g_motors;
static std::vector<std::shared_ptr<Servo>> g_servos;
static std::vector<std::shared_ptr<Relay>> g_relays;

core::Result<void> initActuators() {
    if (g_actuatorsInitialized) {
        return core::makeOk(); // Already initialized
    }

    FMUS_LOG_INFO("Initializing actuators module");

    // Initialize any global actuator resources here
    // For now, just mark as initialized
    g_actuatorsInitialized = true;

    FMUS_LOG_INFO("Actuators module initialized successfully");
    return core::makeOk();
}

core::Result<void> shutdownActuators() {
    if (!g_actuatorsInitialized) {
        return core::makeOk(); // Already shutdown
    }

    FMUS_LOG_INFO("Shutting down actuators module");

    // Stop all motors
    for (auto& motor : g_motors) {
        if (motor && motor->isInitialized()) {
            auto result = motor->stop();
            if (result.isError()) {
                FMUS_LOG_ERROR("Failed to stop motor: " + result.error().message());
            }
        }
    }

    // Stop all servos
    for (auto& servo : g_servos) {
        if (servo && servo->isInitialized()) {
            auto result = servo->stop();
            if (result.isError()) {
                FMUS_LOG_ERROR("Failed to stop servo: " + result.error().message());
            }
        }
    }

    // Turn off all relays
    for (auto& relay : g_relays) {
        if (relay && relay->isInitialized()) {
            auto result = relay->turnOff();
            if (result.isError()) {
                FMUS_LOG_ERROR("Failed to turn off relay: " + result.error().message());
            }
        }
    }

    // Clear all collections
    g_motors.clear();
    g_servos.clear();
    g_relays.clear();

    g_actuatorsInitialized = false;
    FMUS_LOG_INFO("Actuators module shutdown completed");
    return core::makeOk();
}

bool isActuatorsInitialized() {
    return g_actuatorsInitialized;
}

core::Result<void> emergencyStopAll() {
    FMUS_LOG_ERROR("EMERGENCY STOP - Stopping all actuators immediately!");

    bool hasErrors = false;
    std::ostringstream errorMessages;

    // Emergency stop all motors
    for (auto& motor : g_motors) {
        if (motor && motor->isInitialized()) {
            auto result = motor->stop();
            if (result.isError()) {
                hasErrors = true;
                errorMessages << "Motor stop failed: " << result.error().message() << "; ";
            }
        }
    }

    // Emergency stop all servos
    for (auto& servo : g_servos) {
        if (servo && servo->isInitialized()) {
            auto result = servo->stop();
            if (result.isError()) {
                hasErrors = true;
                errorMessages << "Servo stop failed: " << result.error().message() << "; ";
            }
        }
    }

    // Emergency turn off all relays
    for (auto& relay : g_relays) {
        if (relay && relay->isInitialized()) {
            auto result = relay->turnOff();
            if (result.isError()) {
                hasErrors = true;
                errorMessages << "Relay off failed: " << result.error().message() << "; ";
            }
        }
    }

    if (hasErrors) {
        FMUS_LOG_ERROR("Emergency stop completed with errors: " + errorMessages.str());
        return core::makeError<void>(core::ErrorCode::ActuatorSetValueError,
                                   "Emergency stop had errors: " + errorMessages.str());
    }

    FMUS_LOG_INFO("Emergency stop completed successfully - all actuators stopped");
    return core::makeOk();
}

std::string getActuatorsStatus() {
    std::ostringstream oss;
    oss << "Actuators Module Status:\n";
    oss << "  Initialized: " << (g_actuatorsInitialized ? "Yes" : "No") << "\n";
    oss << "  Registered Motors: " << g_motors.size() << "\n";
    oss << "  Registered Servos: " << g_servos.size() << "\n";
    oss << "  Registered Relays: " << g_relays.size() << "\n";

    if (!g_motors.empty()) {
        oss << "\n  Motors:\n";
        for (size_t i = 0; i < g_motors.size(); ++i) {
            if (g_motors[i]) {
                oss << "    " << i << ": " << motorTypeToString(g_motors[i]->getType()) 
                    << " (" << (g_motors[i]->isInitialized() ? "Initialized" : "Not Initialized") << ")\n";
            }
        }
    }

    if (!g_servos.empty()) {
        oss << "\n  Servos:\n";
        for (size_t i = 0; i < g_servos.size(); ++i) {
            if (g_servos[i]) {
                oss << "    " << i << ": Pin " << static_cast<int>(g_servos[i]->getPWMPin())
                    << " (" << (g_servos[i]->isInitialized() ? "Initialized" : "Not Initialized") << ")\n";
            }
        }
    }

    if (!g_relays.empty()) {
        oss << "\n  Relays:\n";
        for (size_t i = 0; i < g_relays.size(); ++i) {
            if (g_relays[i]) {
                oss << "    " << i << ": Pin " << static_cast<int>(g_relays[i]->getControlPin())
                    << " (" << relayStateToString(g_relays[i]->getState()) << ")\n";
            }
        }
    }

    return oss.str();
}

// Internal functions for registering actuators (used by actuator constructors)
namespace internal {

void registerMotor(std::shared_ptr<IMotor> motor) {
    if (motor) {
        g_motors.push_back(motor);
        FMUS_LOG_DEBUG("Motor registered with actuators module");
    }
}

void unregisterMotor(std::shared_ptr<IMotor> motor) {
    auto it = std::find(g_motors.begin(), g_motors.end(), motor);
    if (it != g_motors.end()) {
        g_motors.erase(it);
        FMUS_LOG_DEBUG("Motor unregistered from actuators module");
    }
}

void registerServo(std::shared_ptr<Servo> servo) {
    if (servo) {
        g_servos.push_back(servo);
        FMUS_LOG_DEBUG("Servo registered with actuators module");
    }
}

void unregisterServo(std::shared_ptr<Servo> servo) {
    auto it = std::find(g_servos.begin(), g_servos.end(), servo);
    if (it != g_servos.end()) {
        g_servos.erase(it);
        FMUS_LOG_DEBUG("Servo unregistered from actuators module");
    }
}

void registerRelay(std::shared_ptr<Relay> relay) {
    if (relay) {
        g_relays.push_back(relay);
        FMUS_LOG_DEBUG("Relay registered with actuators module");
    }
}

void unregisterRelay(std::shared_ptr<Relay> relay) {
    auto it = std::find(g_relays.begin(), g_relays.end(), relay);
    if (it != g_relays.end()) {
        g_relays.erase(it);
        FMUS_LOG_DEBUG("Relay unregistered from actuators module");
    }
}

} // namespace internal

} // namespace actuators
} // namespace fmus
