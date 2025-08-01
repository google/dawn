# This file caches variables which are platform specific.

# Windows-specific configuration
if (WIN32)
    set(DAWN_USE_BUILT_DXC ON CACHE BOOL "")
    set(CMAKE_SYSTEM_VERSION "$ENV{WIN10_SDK_VERSION}" CACHE STRING "")
    set(CMAKE_WINDOWS_KITS_10_DIR "$ENV{WIN10_SDK_PATH}" CACHE STRING "")
    set(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION "$ENV{WIN10_SDK_VERSION}" CACHE STRING "")
endif ()

# Common configuration for all platforms
set(DAWN_FETCH_DEPENDENCIES ON CACHE BOOL "")
set(DAWN_ENABLE_INSTALL ON CACHE BOOL "")

# Linux-specific configuration
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # `sccache` seems effective only on linux.
    # for windows, we could look into `buildcache`
    # for macos, `sccache` causes an argument parse error for clang
    # similar to https://github.com/fastbuild/fastbuild/issues/1041
    # maybe we could use `ccache` in macos.
    set(CMAKE_C_COMPILER_LAUNCHER "sccache" CACHE STRING "")
    set(CMAKE_CXX_COMPILER_LAUNCHER "sccache" CACHE STRING "")
endif ()

# Mobile platform configuration (iOS and Android only)
if (ANDROID OR (CMAKE_SYSTEM_NAME STREQUAL "iOS") OR (DEFINED PLATFORM AND PLATFORM MATCHES "OS64|SIMULATOR.*"))
    # Disable samples and tests for mobile builds
    set(BUILD_SAMPLES OFF CACHE BOOL "")
    set(TINT_BUILD_TESTS OFF CACHE BOOL "")
    set(TINT_BUILD_CMD_TOOLS OFF CACHE BOOL "")
    set(TINT_BUILD_IR_BINARY OFF CACHE BOOL "")
    set(DAWN_BUILD_SAMPLES OFF CACHE BOOL "")
    
    # Disable GLFW for mobile platforms
    set(DAWN_USE_GLFW OFF CACHE BOOL "")
    
    # Use static monolithic library for mobile
    set(DAWN_BUILD_MONOLITHIC_LIBRARY ON CACHE BOOL "")
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
    
    # Disable OpenGL variants for mobile
    set(DAWN_ENABLE_OPENGLES OFF CACHE BOOL "")
    set(DAWN_ENABLE_DESKTOP_GL OFF CACHE BOOL "")
endif ()

# macOS universal build configuration (uses Apple toolchain but is not mobile)
if (DEFINED PLATFORM AND PLATFORM STREQUAL "MAC_UNIVERSAL")
    # Apply the same mobile-like settings for consistency in the packaging process
    set(BUILD_SAMPLES OFF CACHE BOOL "")
    set(TINT_BUILD_TESTS OFF CACHE BOOL "")
    set(TINT_BUILD_CMD_TOOLS OFF CACHE BOOL "")
    set(TINT_BUILD_IR_BINARY OFF CACHE BOOL "")
    set(DAWN_BUILD_SAMPLES OFF CACHE BOOL "")
    set(DAWN_USE_GLFW OFF CACHE BOOL "")
    set(DAWN_BUILD_MONOLITHIC_LIBRARY ON CACHE BOOL "")
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
    set(DAWN_ENABLE_OPENGLES OFF CACHE BOOL "")
    set(DAWN_ENABLE_DESKTOP_GL OFF CACHE BOOL "")
endif ()

# Android-specific configuration
if (ANDROID)
    # Additional Android-specific settings can go here
endif ()

# iOS-specific configuration  
if (CMAKE_SYSTEM_NAME STREQUAL "iOS" OR (DEFINED PLATFORM AND PLATFORM MATCHES "OS64|SIMULATOR.*"))
    # Additional iOS-specific settings can go here
endif ()
