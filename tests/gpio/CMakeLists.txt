# List of test source files for the GPIO module
set(FMUS_GPIO_TEST_SOURCES
    gpio_test.cpp
)

# Add test sources to the test executable
target_sources(fmus_embed_tests
    PRIVATE
        ${FMUS_GPIO_TEST_SOURCES}
)

# For each test file, make a separate test executable and CTest target
foreach(test_source ${FMUS_GPIO_TEST_SOURCES})
    # Get the file name without extension
    get_filename_component(test_name ${test_source} NAME_WE)

    # Add a CTest test
    add_test(
        NAME ${test_name}
        COMMAND fmus_embed_tests --gtest_filter=${test_name}.*
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
endforeach()
