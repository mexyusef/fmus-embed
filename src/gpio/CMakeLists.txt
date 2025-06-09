# GPIO module sources
target_sources(fmus-embed
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/gpio.cpp
)

# Include directories
target_include_directories(fmus-embed
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
