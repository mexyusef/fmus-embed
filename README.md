# fmus-embed

A lightweight, cross-platform embedded systems library for microcontroller units (MCUs) and sensor integrations.

## Overview

fmus-embed is a modern C++ library (C++17) designed to simplify embedded systems development by providing a consistent API across different platforms and hardware.

It offers abstractions for common MCU peripherals, sensor interfaces, and communication protocols.

This is a work in progress. Some of the functionalities have not been implemented yet and the examples do not build at this point.

## Features

- **Core utilities**: Error handling, logging, memory management
- **MCU interfaces**: GPIO, timers, ADC, platform abstractions
- **Sensor support**: Temperature, accelerometer, gyroscope, pressure, light sensors
- **Communication protocols**: I2C, SPI
- **Cross-platform**: Windows, Linux, macOS, and embedded targets
- **Modern C++**: Uses C++17 features for better type safety and usability

## Building

fmus-embed uses CMake (3.20+) as its build system:

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Configuration Options

- `FMUS_EMBED_BUILD_TESTS`: Build tests (ON by default)
- `FMUS_EMBED_BUILD_EXAMPLES`: Build examples (ON by default)
- `FMUS_EMBED_USE_EXCEPTIONS`: Use exceptions for error handling (ON by default)
- `FMUS_EMBED_HEADER_ONLY`: Build as header-only library (OFF by default)

## Examples

See the `examples/` directory for usage examples:

```cpp
#include <fmus/fmus.h>

int main() {
    // Initialize the library
    if (!fmus::init()) {
        return 1;
    }

    // Use library features
    auto pin = fmus::gpio::PinConfig{.port = 0, .pin = 13};
    auto result = fmus::gpio::configurePin(pin, fmus::gpio::PinMode::Output);

    if (result.isOk()) {
        fmus::gpio::writePin(pin, true);  // Turn on LED
    }

    // Clean up
    fmus::shutdown();
    return 0;
}
```

## Documentation

API documentation is available in the `docs/` directory.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
