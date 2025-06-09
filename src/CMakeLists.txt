# Define sources for each module
set(FMUS_CORE_SOURCES
    core/error.cpp
    core/logging.cpp
    core/memory.cpp
    core/version.cpp
)

set(FMUS_MCU_SOURCES
    mcu/gpio.cpp
    mcu/timer.cpp
    mcu/adc.cpp
    mcu/platform.cpp
)

set(FMUS_GPIO_SOURCES
    gpio/gpio.cpp
)

set(FMUS_SENSORS_SOURCES
    # sensors/sensor_base.cpp
    sensors/accelerometer.cpp
    sensors/temperature.cpp
    sensors/gyroscope.cpp
    sensors/pressure.cpp
    sensors/light.cpp
)

set(FMUS_ACTUATORS_SOURCES
    # actuators/motor.cpp
    # actuators/relay.cpp
    # actuators/servo.cpp
)

set(FMUS_COMMS_SOURCES
    comms/spi.cpp
    comms/i2c.cpp
    # comms/uart.cpp
)

set(FMUS_DSP_SOURCES
    # dsp/filter.cpp
    # dsp/fft.cpp
)

set(FMUS_AI_SOURCES
    # ai/neural_net.cpp
)

set(FMUS_NET_SOURCES
    # net/mqtt.cpp
)

# Combine all sources
set(FMUS_EMBED_SOURCES
    ${FMUS_CORE_SOURCES}
    ${FMUS_MCU_SOURCES}
    ${FMUS_GPIO_SOURCES}
    ${FMUS_SENSORS_SOURCES}
    ${FMUS_ACTUATORS_SOURCES}
    ${FMUS_COMMS_SOURCES}
    ${FMUS_DSP_SOURCES}
    ${FMUS_AI_SOURCES}
    ${FMUS_NET_SOURCES}
    fmus.cpp
)

# Define the main library target
if(FMUS_EMBED_HEADER_ONLY)
    # If header-only, create an interface library
    add_library(fmus-embed INTERFACE)
    target_include_directories(fmus-embed INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<INSTALL_INTERFACE:include>
    )
else()
    # Otherwise, create a shared or static library
    add_library(fmus-embed SHARED ${FMUS_EMBED_SOURCES})
    target_include_directories(fmus-embed PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<INSTALL_INTERFACE:include>
    )
    target_compile_definitions(fmus-embed PRIVATE FMUS_EMBED_BUILDING_LIBRARY)

    # Link with dependencies
    # target_link_libraries(fmus-embed PRIVATE Threads::Threads)

    # Add dependency on copy_dlls
    add_dependencies(fmus-embed copy_dlls)
endif()

# Set the target properties
set_target_properties(fmus-embed PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Install the library
install(TARGETS fmus-embed
    EXPORT fmus-embedTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# Add subdirectories for each module
add_subdirectory(core)
add_subdirectory(gpio)
add_subdirectory(sensors)
add_subdirectory(comms)
add_subdirectory(mcu)
# add_subdirectory(actuators)
# add_subdirectory(dsp)
# add_subdirectory(ai)
# add_subdirectory(net)
