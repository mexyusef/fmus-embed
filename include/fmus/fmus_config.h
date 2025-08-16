#pragma once

// Configuration options
#define FMUS_EMBED_USE_EXCEPTIONS 1
#define FMUS_EMBED_HEADER_ONLY 0

// Library version
#define FMUS_EMBED_VERSION_MAJOR 0
#define FMUS_EMBED_VERSION_MINOR 0
#define FMUS_EMBED_VERSION_PATCH 1
#define FMUS_EMBED_VERSION_STRING "0.0.1"

// Platform detection
#if defined(_WIN32) || defined(_WIN64)
    #define FMUS_EMBED_PLATFORM_WINDOWS 1
#elif defined(__linux__)
    #define FMUS_EMBED_PLATFORM_LINUX 1
#elif defined(__APPLE__)
    #define FMUS_EMBED_PLATFORM_MACOS 1
#elif defined(__FreeBSD__)
    #define FMUS_EMBED_PLATFORM_FREEBSD 1
#elif defined(__unix__)
    #define FMUS_EMBED_PLATFORM_UNIX 1
#endif

// Compiler detection
#if defined(_MSC_VER)
    #define FMUS_EMBED_COMPILER_MSVC 1
#elif defined(__clang__)
    #define FMUS_EMBED_COMPILER_CLANG 1
#elif defined(__GNUC__)
    #define FMUS_EMBED_COMPILER_GCC 1
#endif

// Architecture detection
#if defined(__x86_64__) || defined(_M_X64)
    #define FMUS_EMBED_ARCH_X64 1
#elif defined(__i386) || defined(_M_IX86)
    #define FMUS_EMBED_ARCH_X86 1
#elif defined(__arm__) || defined(_M_ARM)
    #define FMUS_EMBED_ARCH_ARM 1
#elif defined(__aarch64__)
    #define FMUS_EMBED_ARCH_ARM64 1
#endif

// Export macros
#if FMUS_EMBED_HEADER_ONLY
    #define FMUS_EMBED_API inline
#else
    #if defined(_MSC_VER)
        #if defined(FMUS_EMBED_BUILDING_LIBRARY)
            #define FMUS_EMBED_API __declspec(dllexport)
        #else
            #define FMUS_EMBED_API __declspec(dllimport)
        #endif
    #else
        #if defined(FMUS_EMBED_BUILDING_LIBRARY)
            #define FMUS_EMBED_API __attribute__((visibility("default")))
        #else
            #define FMUS_EMBED_API
        #endif
    #endif
#endif
