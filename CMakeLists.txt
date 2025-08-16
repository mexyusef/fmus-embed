cmake_minimum_required(VERSION 3.20)

# Set Visual Studio runtime library options
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

project(fmus-embed VERSION 0.0.1 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Platform-specific package locations
if(WIN32)
  # Windows package locations
  set(LLVM_DIR "C:/llvm-install/lib/cmake/llvm")
  set(LIBUV_DIR "C:/libuv-install/lib/cmake/libuv")
  set(Boost_INCLUDE_DIRS "C:/boost")
  set(USOCKETS_DIR "C:/usockets-install")
  set(NLOHMANN_JSON_INCLUDE_DIR "C:/vcpkg/packages/nlohmann-json_x64-windows/include")

  # Set GTest paths for testing
  set(GTEST_ROOT "C:/vcpkg/packages/gtest_x64-windows")
  set(GTEST_INCLUDE_DIR "${GTEST_ROOT}/include")
  set(GTEST_LIBRARY "${GTEST_ROOT}/lib/gtest.lib")
  set(GTEST_MAIN_LIBRARY "${GTEST_ROOT}/lib/manual-link/gtest_main.lib")
  set(GMOCK_LIBRARY "${GTEST_ROOT}/lib/gmock.lib")
  set(GMOCK_MAIN_LIBRARY "${GTEST_ROOT}/lib/manual-link/gmock_main.lib")

  # Add GTest DLL paths
  set(GTEST_DLL "${GTEST_ROOT}/bin/gtest.dll")
  set(GTEST_MAIN_DLL "${GTEST_ROOT}/bin/gtest_main.dll")
  set(GMOCK_DLL "${GTEST_ROOT}/bin/gmock.dll")
  set(GMOCK_MAIN_DLL "${GTEST_ROOT}/bin/gmock_main.dll")
else()
  # Linux/Unix package locations - use system packages
  # These will be found by CMake's find_package commands
endif()

# Include our custom CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Include directories
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(WIN32)
  include_directories(
    ${LLVM_DIR}/../../../include
    ${LIBUV_DIR}/../../include
    ${Boost_INCLUDE_DIRS}
    ${NLOHMANN_JSON_INCLUDE_DIR}
    ${USOCKETS_DIR}/include
  )

  # Check if required DLLs exist and warn if not
  if(NOT EXISTS "C:/libuv-install/bin/uv.dll")
    message(WARNING "LibUV DLL not found at C:/libuv-install/bin/uv.dll")
  endif()

  if(NOT EXISTS "C:/usockets-install/bin/uSockets.dll")
    message(WARNING "uSockets DLL not found at C:/usockets-install/bin/uSockets.dll")
  endif()
endif()

# Windows-specific DLL copying
if(WIN32)
  # Fix: Add a custom command to create the output directory first
  add_custom_command(
    OUTPUT ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/create_dir_marker
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/create_dir_marker
    COMMENT "Creating output directory for DLLs"
  )

  # Add a custom target to copy DLLs
  add_custom_target(copy_dlls
    DEPENDS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/create_dir_marker
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "C:/libuv-install/bin/uv.dll"
      "C:/usockets-install/bin/uSockets.dll"
      "${GTEST_DLL}"
      "${GTEST_MAIN_DLL}"
      "${GMOCK_DLL}"
      "${GMOCK_MAIN_DLL}"
      "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/"
    COMMENT "Copying DLLs to output directory"
  )
endif()

# Define library options
option(FMUS_EMBED_BUILD_TESTS "Build tests" ON)
option(FMUS_EMBED_BUILD_EXAMPLES "Build examples" ON)
option(FMUS_EMBED_USE_EXCEPTIONS "Use exceptions for error handling" ON)
option(FMUS_EMBED_HEADER_ONLY "Build as header-only library" OFF)

# Add configuration header
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/fmus_config.h.in
  ${CMAKE_CURRENT_SOURCE_DIR}/include/fmus/fmus_config.h
)

# Add subdirectories
add_subdirectory(src)

# Enable testing if requested
if(FMUS_EMBED_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# Add examples if requested
if(FMUS_EMBED_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

# Install rules
install(DIRECTORY include/ DESTINATION include)

# Generate package configuration
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/fmus-embedConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

# Export targets
install(EXPORT fmus-embedTargets
  FILE fmus-embedTargets.cmake
  NAMESPACE fmus::
  DESTINATION lib/cmake/fmus-embed
)

# Install config files
install(
  FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake/fmus-embedConfig.cmake"
  DESTINATION lib/cmake/fmus-embed
)
