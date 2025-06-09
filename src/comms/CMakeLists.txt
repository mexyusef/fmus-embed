target_sources(fmus-embed
    PRIVATE
        spi.cpp
        i2c.cpp
        # Add other communication implementation files here
)

# Include communication-specific directories if needed
# target_include_directories(fmus-embed
#     PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}/include
# )
