# Enable testing
enable_testing()

# Find GTest
find_package(GTest REQUIRED)

# Define test sources for each module
set(FMUS_CORE_TEST_SOURCES
    # core/error_test.cpp
    # core/logging_test.cpp
    # core/memory_test.cpp
)

set(FMUS_MCU_TEST_SOURCES
    # mcu/gpio_test.cpp
    # mcu/timer_test.cpp
    # mcu/adc_test.cpp
)

set(FMUS_GPIO_TEST_SOURCES
    # gpio/gpio_test.cpp
)

set(FMUS_SENSORS_TEST_SOURCES
    # sensors/sensor_base_test.cpp
    # sensors/accelerometer_test.cpp
    # sensors/temperature_test.cpp
)

set(FMUS_ACTUATORS_TEST_SOURCES
    # actuators/motor_test.cpp
    # actuators/relay_test.cpp
    # actuators/servo_test.cpp
)

set(FMUS_COMMS_TEST_SOURCES
    # comms/spi_test.cpp
    # comms/i2c_test.cpp
    # comms/uart_test.cpp
)

set(FMUS_DSP_TEST_SOURCES
    # dsp/filter_test.cpp
    # dsp/fft_test.cpp
)

set(FMUS_AI_TEST_SOURCES
    # ai/neural_net_test.cpp
)

set(FMUS_NET_TEST_SOURCES
    # net/mqtt_test.cpp
)

# Create a dummy main.cpp file
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/main.cpp
"#include <gtest/gtest.h>

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
")

# Combine all test sources
set(FMUS_EMBED_TEST_SOURCES
    ${CMAKE_CURRENT_BINARY_DIR}/main.cpp
    ${FMUS_CORE_TEST_SOURCES}
    ${FMUS_MCU_TEST_SOURCES}
    ${FMUS_GPIO_TEST_SOURCES}
    ${FMUS_SENSORS_TEST_SOURCES}
    ${FMUS_ACTUATORS_TEST_SOURCES}
    ${FMUS_COMMS_TEST_SOURCES}
    ${FMUS_DSP_TEST_SOURCES}
    ${FMUS_AI_TEST_SOURCES}
    ${FMUS_NET_TEST_SOURCES}
)

# Add the test executable
add_executable(fmus_embed_tests ${FMUS_EMBED_TEST_SOURCES})

# Link with the fmus-embed library and GTest
target_link_libraries(fmus_embed_tests
    PRIVATE
    fmus-embed
    GTest::GTest
    GTest::Main
)

# Set include directories for tests
target_include_directories(fmus_embed_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GTEST_INCLUDE_DIR}
)

# Add DLL dependency
add_dependencies(fmus_embed_tests copy_dlls)

# Register tests with CTest
add_test(NAME fmus_embed_tests COMMAND fmus_embed_tests)

# Set working directory for tests
set_tests_properties(fmus_embed_tests PROPERTIES
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}
)

# Add subdirectories
# add_subdirectory(gpio)
