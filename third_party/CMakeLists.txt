# Copyright 2022 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Don't build testing in third_party dependencies
set(BUILD_TESTING OFF)

# fetch_dawn_dependencies.py is an alternative to using depot_tools
# It is particularly interesting when building dawn as a subdirectory in
# a parent project that does not want to use depot_tools.
if (${DAWN_FETCH_DEPENDENCIES})
    find_package(PythonInterp 3 REQUIRED)

    set(EXTRA_FETCH_ARGS)
    if (NOT TARGET gmock AND ${TINT_BUILD_TESTS})
        list(APPEND EXTRA_FETCH_ARGS --use-test-deps)
    endif()

    message(STATUS "Running fetch_dawn_dependencies:")
    execute_process(
        COMMAND
            ${PYTHON_EXECUTABLE}
            "${PROJECT_SOURCE_DIR}/tools/fetch_dawn_dependencies.py"
            --directory ${PROJECT_SOURCE_DIR}
            ${EXTRA_FETCH_ARGS}
    )
endif ()

if (NOT TARGET SPIRV-Headers)
    set(SPIRV_HEADERS_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
    set(SPIRV_HEADERS_SKIP_INSTALL ON CACHE BOOL "" FORCE)

    message(STATUS "Dawn: using SPIRV-Headers at ${DAWN_SPIRV_HEADERS_DIR}")
    add_subdirectory(${DAWN_SPIRV_HEADERS_DIR} "${CMAKE_CURRENT_BINARY_DIR}/spirv-headers")
endif()

# Needs to come before SPIR-V Tools
if ((${TINT_BUILD_SPIRV_TOOLS_FUZZER} OR ${TINT_BUILD_AST_FUZZER}) AND
        (NOT TARGET protobuf::libprotobuf OR NOT TARGET protobuf::protoc))
    set(protobuf_BUILD_TESTS OFF CACHE BOOL "Controls whether protobuf tests are built" FORCE)
    set(protobuf_MSVC_STATIC_RUNTIME OFF CACHE BOOL "Controls whether a protobuf static runtime is built" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/protobuf)
endif()

if (NOT TARGET SPIRV-Tools)
    set(SPIRV_SKIP_TESTS ON CACHE BOOL "" FORCE)
    set(SPIRV_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)
    set(SKIP_SPIRV_TOOLS_INSTALL ON CACHE BOOL "" FORCE)

    if(${TINT_BUILD_SPV_READER} OR ${TINT_BUILD_SPV_WRITER})
        set(SPIRV_SKIP_TESTS ON CACHE BOOL "Controls whether SPIR-V tests are run" FORCE)
        set(SPIRV_WERROR OFF CACHE BOOL OFF FORCE)
        if (${TINT_BUILD_SPIRV_TOOLS_FUZZER})
            set(SPIRV_BUILD_FUZZER ON CACHE BOOL "Controls whether spirv-fuzz is built" FORCE)
        endif()
    endif()

    message(STATUS "Dawn: using SPIRV-Tools at ${DAWN_SPIRV_TOOLS_DIR}")
    add_subdirectory(${DAWN_SPIRV_TOOLS_DIR} "${CMAKE_CURRENT_BINARY_DIR}/spirv-tools" EXCLUDE_FROM_ALL)
endif()

if(NOT TARGET glslang AND (${TINT_BUILD_GLSL_WRITER} OR ${TINT_BUILD_GLSL_VALIDATOR}) AND ${TINT_BUILD_CMD_TOOLS})
    set(SKIP_GLSLANG_INSTALL ON CACHE BOOL "" FORCE)
    add_subdirectory("${DAWN_THIRD_PARTY_DIR}/vulkan-deps/glslang/src" "${CMAKE_CURRENT_BINARY_DIR}/glslang" EXCLUDE_FROM_ALL)
endif()

if (NOT TARGET glfw AND DAWN_USE_GLFW)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_X11 ${DAWN_USE_X11} CACHE BOOL "" FORCE)
    set(GLFW_BUILD_WAYLAND ${DAWN_USE_WAYLAND} CACHE BOOL "" FORCE)

    message(STATUS "Dawn: using GLFW at ${DAWN_GLFW_DIR}")
    add_subdirectory(${DAWN_GLFW_DIR} "${CMAKE_CURRENT_BINARY_DIR}/glfw")
endif()

if (NOT TARGET libabsl)
    message(STATUS "Dawn: using Abseil at ${DAWN_ABSEIL_DIR}")
    if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR
        ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang"))
        add_compile_options(
            -Wno-array-parameter
            -Wno-deprecated-builtins
            -Wno-unknown-warning-option
        )
    endif()

    add_subdirectory(${DAWN_ABSEIL_DIR} "${CMAKE_CURRENT_BINARY_DIR}/abseil")
endif()

if (NOT TARGET Vulkan-Headers)
    message(STATUS "Dawn: using Vulkan-Headers at ${DAWN_VULKAN_HEADERS_DIR}")
    add_subdirectory(${DAWN_VULKAN_HEADERS_DIR} "${CMAKE_CURRENT_BINARY_DIR}/vulkan-headers")
endif()

# Header-only library for khrplatform.h
add_library(dawn_khronos_platform INTERFACE)
target_sources(dawn_khronos_platform INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/khronos/EGL-Registry/api/KHR/khrplatform.h")
target_include_directories(dawn_khronos_platform INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/khronos/EGL-Registry/api")

# Header-only library for Vulkan headers
add_library(dawn_vulkan_headers INTERFACE)
target_sources(dawn_vulkan_headers INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/khronos/vulkan/vk_icd.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/khronos/vulkan/vk_layer.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/khronos/vulkan/vk_platform.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/khronos/vulkan/vk_sdk_platform.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/khronos/vulkan/vulkan.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/khronos/vulkan/vulkan_core.h"
)
target_include_directories(dawn_vulkan_headers INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/khronos")

if (NOT TARGET vk_swiftshader AND ${DAWN_ENABLE_SWIFTSHADER})
    set(SWIFTSHADER_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(SWIFTSHADER_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)

    message(STATUS "Dawn: using Swiftshader at ${DAWN_SWIFTSHADER_DIR}")
    add_subdirectory(${DAWN_SWIFTSHADER_DIR} "${CMAKE_CURRENT_BINARY_DIR}/swiftshader")
endif()

if (${TINT_BUILD_BENCHMARKS} OR ${DAWN_BUILD_BENCHMARKS})
    set(BENCHMARK_ENABLE_TESTING FALSE CACHE BOOL FALSE FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/google_benchmark/src EXCLUDE_FROM_ALL)
endif()

if (NOT TARGET gmock AND ${TINT_BUILD_TESTS})
    set(gtest_force_shared_crt ON CACHE BOOL "Controls whether a shared run-time library should be used even when Google Test is built as static library" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/googletest EXCLUDE_FROM_ALL)
endif()
