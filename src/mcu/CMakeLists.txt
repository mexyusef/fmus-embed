target_sources(fmus-embed
    PRIVATE
        gpio.cpp
        timer.cpp
        adc.cpp
        platform.cpp
)

# Include MCU-specific directories if needed
# target_include_directories(fmus-embed
#     PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}/include
# )
