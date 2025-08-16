# Define a function to add an example
function(add_fmus_example name source_file)
    add_executable(${name} ${source_file})

    target_link_libraries(${name} PRIVATE fmus-embed)

    set_target_properties(${name} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/examples"
    )

    # Add DLL dependency (Windows only)
    if(WIN32 AND TARGET copy_dlls)
        add_dependencies(${name} copy_dlls)
    endif()

    # Add examples to a special target group for organization in IDEs
    set_target_properties(${name} PROPERTIES FOLDER "Examples")
endfunction()

# Add examples
add_fmus_example(read_temperature read_temperature.cpp)
add_fmus_example(advanced_temperature_reading advanced_temperature_reading.cpp)
add_fmus_example(blink_led blink_led.cpp)
add_fmus_example(read_pressure read_pressure.cpp)
add_fmus_example(uart_communication uart_communication.cpp)
add_fmus_example(actuator_control actuator_control.cpp)
add_fmus_example(dsp_demo dsp_demo.cpp)
# add_fmus_example(spi_communication spi_communication.cpp)
# add_fmus_example(fft_analysis fft_analysis.cpp)
add_fmus_example(traffic_light traffic_light.cpp)
