# This file caches variables which are platform specific.

# Define IS_MOBILE variable for mobile platforms and universal builds
if (ANDROID OR 
    (CMAKE_SYSTEM_NAME STREQUAL "iOS") OR 
    (DEFINED PLATFORM AND PLATFORM MATCHES "OS64|SIMULATOR.*") OR
    (DEFINED PLATFORM AND PLATFORM STREQUAL "MAC_UNIVERSAL"))
    set(IS_MOBILE TRUE)
else()
    set(IS_MOBILE FALSE)
endif()

# Windows-specific configuration
if (WIN32)
    set(DAWN_USE_BUILT_DXC ON CACHE BOOL "")
    set(CMAKE_SYSTEM_VERSION "$ENV{WIN10_SDK_VERSION}" CACHE STRING "")
    set(CMAKE_WINDOWS_KITS_10_DIR "$ENV{WIN10_SDK_PATH}" CACHE STRING "")
    set(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION "$ENV{WIN10_SDK_VERSION}" CACHE STRING "")
endif()

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
endif()

# Mobile and universal build configuration
if (IS_MOBILE)
    # Build type
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
    
    # Disable samples and tests
    set(BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
    set(TINT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(TINT_BUILD_CMD_TOOLS OFF CACHE BOOL "" FORCE)
    set(TINT_BUILD_IR_BINARY OFF CACHE BOOL "" FORCE)
    set(DAWN_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
    
    # Disable GLFW
    set(DAWN_USE_GLFW OFF CACHE BOOL "" FORCE)
    
    # Use static monolithic library
    set(DAWN_BUILD_MONOLITHIC_LIBRARY ON CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    
    # Disable OpenGL variants
    set(DAWN_ENABLE_OPENGLES OFF CACHE BOOL "" FORCE)
    set(DAWN_ENABLE_DESKTOP_GL OFF CACHE BOOL "" FORCE)
endif()
