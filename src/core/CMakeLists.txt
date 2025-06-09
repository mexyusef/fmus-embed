# Core module CMakeLists.txt
set(CORE_SOURCES
    error.cpp
    logging.cpp
    memory.cpp
    # Add other core source files here
)

# Add the sources to the main library
target_sources(fmus-embed PRIVATE ${CORE_SOURCES})
