target_sources(fmus-embed
    PRIVATE
        accelerometer.cpp
        temperature.cpp
        pressure.cpp
        gyroscope.cpp
        light.cpp
        # Add other sensor implementation files here
)

# Include sensor-specific directories if needed
# target_include_directories(fmus-embed
#     PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}/include
# )
