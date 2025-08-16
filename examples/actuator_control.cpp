#include <fmus/fmus.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <memory>

using namespace fmus;
using namespace fmus::actuators;

// Helper function to check results
template<typename T>
void checkResult(const core::Result<T>& result, const std::string& operation) {
    if (result.isError()) {
        std::cerr << "Error in " << operation << ": " << result.error().toString() << std::endl;
        exit(1);
    }
}

void demonstrateDCMotor() {
    std::cout << "\n=== DC Motor Control Demo ===" << std::endl;
    
    // Create DC motor on PWM pin 9, direction pin 10
    DCMotor motor(9, 10);
    
    std::cout << "Initializing DC motor..." << std::endl;
    auto initResult = motor.init();
    if (initResult.isError()) {
        std::cout << "DC Motor initialization failed (expected without hardware): " 
                  << initResult.error().toString() << std::endl;
        std::cout << "Demonstrating API usage..." << std::endl;
    } else {
        std::cout << "DC Motor initialized successfully!" << std::endl;
    }
    
    std::cout << motor.getStatus() << std::endl;
    
    // Demonstrate motor control
    std::cout << "\nTesting motor operations:" << std::endl;
    
    // Set speed to 50%
    std::cout << "Setting speed to 50%..." << std::endl;
    motor.setSpeed(0.5f);
    
    // Set direction to forward
    std::cout << "Setting direction to forward..." << std::endl;
    motor.setDirection(MotorDirection::Forward);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // Change direction to reverse
    std::cout << "Setting direction to reverse..." << std::endl;
    motor.setDirection(MotorDirection::Reverse);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // Use combined speed and direction
    std::cout << "Setting speed and direction (-0.75 = 75% reverse)..." << std::endl;
    motor.setSpeedAndDirection(-0.75f);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // Stop motor
    std::cout << "Stopping motor..." << std::endl;
    motor.stop();
    
    std::cout << "Current speed: " << (motor.getSpeed() * 100.0f) << "%" << std::endl;
    std::cout << "Current direction: " << motorDirectionToString(motor.getDirection()) << std::endl;
}

void demonstrateServoMotor() {
    std::cout << "\n=== Servo Motor Control Demo ===" << std::endl;
    
    // Create servo motor on PWM pin 11
    ServoMotor servo(11);
    
    std::cout << "Initializing servo motor..." << std::endl;
    auto initResult = servo.init();
    if (initResult.isError()) {
        std::cout << "Servo Motor initialization failed (expected without hardware): " 
                  << initResult.error().toString() << std::endl;
        std::cout << "Demonstrating API usage..." << std::endl;
    } else {
        std::cout << "Servo Motor initialized successfully!" << std::endl;
    }
    
    std::cout << servo.getStatus() << std::endl;
    
    // Demonstrate servo control
    std::cout << "\nTesting servo operations:" << std::endl;
    
    // Move to 0 degrees
    std::cout << "Moving to 0 degrees..." << std::endl;
    servo.setAngle(0.0f);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Move to 90 degrees
    std::cout << "Moving to 90 degrees..." << std::endl;
    servo.setAngle(90.0f);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Move to 180 degrees
    std::cout << "Moving to 180 degrees..." << std::endl;
    servo.setAngle(180.0f);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Sweep demonstration
    std::cout << "Performing sweep from 0 to 180 degrees over 2 seconds..." << std::endl;
    servo.sweep(0.0f, 180.0f, 2000);
    std::this_thread::sleep_for(std::chrono::milliseconds(2500));
    
    std::cout << "Current angle: " << servo.getAngle() << "°" << std::endl;
}

void demonstrateStepperMotor() {
    std::cout << "\n=== Stepper Motor Control Demo ===" << std::endl;
    
    // Create stepper motor with pins 2, 3, 4, 5
    StepperMotor stepper(2, 3, 4, 5, 200); // 200 steps per revolution
    
    std::cout << "Initializing stepper motor..." << std::endl;
    auto initResult = stepper.init();
    if (initResult.isError()) {
        std::cout << "Stepper Motor initialization failed (expected without hardware): " 
                  << initResult.error().toString() << std::endl;
        std::cout << "Demonstrating API usage..." << std::endl;
    } else {
        std::cout << "Stepper Motor initialized successfully!" << std::endl;
    }
    
    std::cout << stepper.getStatus() << std::endl;
    
    // Demonstrate stepper control
    std::cout << "\nTesting stepper operations:" << std::endl;
    
    // Step forward 100 steps
    std::cout << "Stepping forward 100 steps..." << std::endl;
    stepper.step(100);
    std::cout << "Current position: " << stepper.getPosition() << " steps" << std::endl;
    
    // Step backward 50 steps
    std::cout << "Stepping backward 50 steps..." << std::endl;
    stepper.step(-50);
    std::cout << "Current position: " << stepper.getPosition() << " steps" << std::endl;
    
    // Rotate 90 degrees
    std::cout << "Rotating 90 degrees..." << std::endl;
    stepper.rotate(90.0f);
    std::cout << "Current position: " << stepper.getPosition() << " steps" << std::endl;
    
    // Change step mode
    std::cout << "Changing to half-step mode..." << std::endl;
    stepper.setStepMode(StepMode::Half);
    
    // Set step delay
    std::cout << "Setting step delay to 2000 microseconds..." << std::endl;
    stepper.setStepDelay(2000);
    
    // Reset position
    std::cout << "Resetting position..." << std::endl;
    stepper.resetPosition();
    std::cout << "Current position: " << stepper.getPosition() << " steps" << std::endl;
}

void demonstrateRelay() {
    std::cout << "\n=== Relay Control Demo ===" << std::endl;
    
    // Create relay on pin 12
    RelayConfig config;
    config.enableSafetyTimeout = true;
    config.safetyTimeoutMs = 5000; // 5 second safety timeout
    
    Relay relay(12, config);
    
    std::cout << "Initializing relay..." << std::endl;
    auto initResult = relay.init();
    if (initResult.isError()) {
        std::cout << "Relay initialization failed (expected without hardware): " 
                  << initResult.error().toString() << std::endl;
        std::cout << "Demonstrating API usage..." << std::endl;
    } else {
        std::cout << "Relay initialized successfully!" << std::endl;
    }
    
    std::cout << relay.getStatus() << std::endl;
    
    // Demonstrate relay control
    std::cout << "\nTesting relay operations:" << std::endl;
    
    // Turn on relay
    std::cout << "Turning relay ON..." << std::endl;
    relay.turnOn();
    std::cout << "Relay state: " << relayStateToString(relay.getState()) << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // Turn off relay
    std::cout << "Turning relay OFF..." << std::endl;
    relay.turnOff();
    std::cout << "Relay state: " << relayStateToString(relay.getState()) << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Toggle relay
    std::cout << "Toggling relay..." << std::endl;
    relay.toggle();
    std::cout << "Relay state: " << relayStateToString(relay.getState()) << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Pulse relay
    std::cout << "Pulsing relay for 2 seconds..." << std::endl;
    relay.pulse(2000);
    std::this_thread::sleep_for(std::chrono::milliseconds(2500));
    std::cout << "Relay state: " << relayStateToString(relay.getState()) << std::endl;
    
    // Show statistics
    auto stats = relay.getStatistics();
    std::cout << "Total switches: " << stats.totalSwitches << std::endl;
    std::cout << "Time since last switch: " << relay.getTimeSinceLastSwitch() << "ms" << std::endl;
}

void demonstrateAdvancedServo() {
    std::cout << "\n=== Advanced Servo Control Demo ===" << std::endl;
    
    // Create servo with custom configuration
    ServoConfig config;
    config.type = ServoType::Standard;
    config.minAngle = 0.0f;
    config.maxAngle = 270.0f; // Extended range servo
    config.enableSmoothing = true;
    config.smoothingSteps = 30;
    
    Servo servo(13, config);
    
    std::cout << "Initializing advanced servo..." << std::endl;
    auto initResult = servo.init();
    if (initResult.isError()) {
        std::cout << "Advanced Servo initialization failed (expected without hardware): " 
                  << initResult.error().toString() << std::endl;
        std::cout << "Demonstrating API usage..." << std::endl;
    }
    
    std::cout << servo.getStatus() << std::endl;
    
    // Set position callback
    servo.setPositionCallback([](float angle) {
        std::cout << "Servo moved to: " << angle << "°" << std::endl;
    });
    
    // Smooth movement
    std::cout << "Smooth movement to 135° over 3 seconds..." << std::endl;
    servo.setAngle(135.0f, 3000);
    std::this_thread::sleep_for(std::chrono::milliseconds(3500));
    
    // Create movement sequence
    std::vector<ServoMovement> sequence = {
        ServoMovement(0.0f, 1000),
        ServoMovement(90.0f, 1500),
        ServoMovement(180.0f, 1000),
        ServoMovement(270.0f, 2000),
        ServoMovement(135.0f, 1500)
    };
    
    std::cout << "Executing movement sequence..." << std::endl;
    servo.executeSequence(sequence, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(8000));
    
    std::cout << "Final angle: " << servo.getAngle() << "°" << std::endl;
}

int main() {
    // Initialize the fmus-embed library
    if (!fmus::init()) {
        std::cerr << "Failed to initialize fmus-embed library" << std::endl;
        return 1;
    }

    std::cout << "fmus-embed library version: " << core::getVersionString() << std::endl;
    std::cout << "Actuator Control Comprehensive Demo" << std::endl;
    std::cout << "===================================" << std::endl;
    
    // Initialize actuators module
    auto actuatorInitResult = initActuators();
    if (actuatorInitResult.isError()) {
        std::cout << "Actuators module initialization failed: " 
                  << actuatorInitResult.error().toString() << std::endl;
        return 1;
    }
    
    std::cout << "Actuators module initialized successfully!" << std::endl;
    std::cout << getActuatorsStatus() << std::endl;
    
    try {
        // Demonstrate all actuator types
        demonstrateDCMotor();
        demonstrateServoMotor();
        demonstrateStepperMotor();
        demonstrateRelay();
        demonstrateAdvancedServo();
        
        std::cout << "\n=== Emergency Stop Demo ===" << std::endl;
        std::cout << "Testing emergency stop functionality..." << std::endl;
        auto emergencyResult = emergencyStopAll();
        if (emergencyResult.isOk()) {
            std::cout << "Emergency stop executed successfully!" << std::endl;
        } else {
            std::cout << "Emergency stop had issues: " << emergencyResult.error().toString() << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        emergencyStopAll();
        return 1;
    }
    
    // Shutdown actuators module
    auto shutdownResult = shutdownActuators();
    if (shutdownResult.isOk()) {
        std::cout << "\nActuators module shutdown successfully!" << std::endl;
    }
    
    std::cout << "\nActuator control demo completed successfully!" << std::endl;
    std::cout << "Note: Hardware-specific operations were simulated." << std::endl;
    std::cout << "In a real embedded system, these would control actual motors, servos, and relays." << std::endl;

    // Clean up
    fmus::shutdown();
    return 0;
}
