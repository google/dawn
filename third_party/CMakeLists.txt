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

function(AddSubdirectoryDXC)
    # We use a CMake function so that all these (non-cache) variables are scoped
    # only to this function.
    set(HLSL_OPTIONAL_PROJS_IN_DEFAULT OFF)
    set(HLSL_ENABLE_ANALYZE OFF)
    set(HLSL_OFFICIAL_BUILD OFF)
    set(HLSL_ENABLE_FIXED_VER OFF)
    set(HLSL_BUILD_DXILCONV OFF)
    set(HLSL_INCLUDE_TESTS OFF)

    set(ENABLE_SPIRV_CODEGEN OFF)
    set(SPIRV_BUILD_TESTS OFF)

    set(LLVM_BUILD_RUNTIME ON)
    set(LLVM_BUILD_EXAMPLES OFF)
    set(LLVM_BUILD_TESTS OFF)
    set(LLVM_INCLUDE_TESTS OFF)
    set(LLVM_INCLUDE_DOCS OFF)
    set(LLVM_INCLUDE_EXAMPLES OFF)
    set(LLVM_OPTIMIZED_TABLEGEN OFF)
    set(LLVM_APPEND_VC_REV OFF)
    # Enable exception handling (requires RTTI)
    set(LLVM_ENABLE_RTTI ON)
    set(LLVM_ENABLE_EH ON)
    set(CLANG_CL OFF)

    # Cache variables -- these are *not* scoped to this function
    set(LLVM_TARGETS_TO_BUILD "None" CACHE STRING "" FORCE)
    set(LLVM_DEFAULT_TARGET_TRIPLE "dxil-ms-dx" CACHE STRING "" FORCE)
    set(CLANG_ENABLE_STATIC_ANALYZER OFF CACHE BOOL "" FORCE)
    set(CLANG_ENABLE_ARCMT OFF CACHE BOOL "" FORCE)
    set(CLANG_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(CLANG_INCLUDE_TESTS OFF CACHE BOOL "" FORCE)

    if (NOT WIN32)
        set(DIRECTX_HEADER_INCLUDE_DIR "${DAWN_THIRD_PARTY_DIR}/dxheaders/include")
    endif()

    # Disable HCT.cmake looking for and using clang-format. This is used to compare generated files
    # against the copy that is committed to the repo, but fails because the DXC .clangformat file is
    # not visible from our build dir. We don't need this validation, so just disable it.
    set(CLANG_FORMAT_EXE "" CACHE STRING "" FORCE)

    # DXC sometimes asserts on valid HLSL, so disable all assertions.
    set(LLVM_ENABLE_ASSERTIONS OFF CACHE BOOL "" FORCE)

    # Override RPATH so that it points to current dir (exe path). This allows both executable and
    # shared library to be in the same location, which we set below. Note that DXC places places
    # executables in a bin directory, and shared libraries in a lib directory.
    if (APPLE)
        set(CMAKE_INSTALL_NAME_DIR "@rpath")
        set(CMAKE_INSTALL_RPATH "@executable_path")
    else()
        set(CMAKE_INSTALL_RPATH "\$ORIGIN")
    endif()

    message(STATUS "\nAdding DXC to build:\n")
    add_subdirectory(dxc
        # Disable all targets by default, and enable only the dxc and dxcompiler targets (below)
        EXCLUDE_FROM_ALL
    )
    set_target_properties(dxc PROPERTIES EXCLUDE_FROM_ALL FALSE)
    set_target_properties(dxcompiler PROPERTIES EXCLUDE_FROM_ALL FALSE)

    # Override output dir for both targets so that they end up in the same place
    # as the rest of our binaries. Otherwise, DXC wants its outputs in bin and lib dirs.
    get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    if (isMultiConfig)
        set_target_properties(dxc dxcompiler PROPERTIES
            "RUNTIME_OUTPUT_DIRECTORY_DEBUG" "${CMAKE_BINARY_DIR}/$<CONFIG>"
            "RUNTIME_OUTPUT_DIRECTORY_RELEASE" "${CMAKE_BINARY_DIR}/$<CONFIG>"
            "LIBRARY_OUTPUT_DIRECTORY_DEBUG" "${CMAKE_BINARY_DIR}/$<CONFIG>"
            "LIBRARY_OUTPUT_DIRECTORY_RELEASE" "${CMAKE_BINARY_DIR}/$<CONFIG>"
        )
    else()
        set_target_properties(dxc dxcompiler PROPERTIES
            "RUNTIME_OUTPUT_DIRECTORY" "${CMAKE_BINARY_DIR}"
            "LIBRARY_OUTPUT_DIRECTORY" "${CMAKE_BINARY_DIR}"
        )
    endif()

    # Create a target that copies dxil.dll from the platform SDK
    if (WIN32)
        message(STATUS "Finding Windows SDK Directory")

        message(STATUS "Display environment variables:")
        execute_process(COMMAND ${CMAKE_COMMAND} -E environment COMMAND_ECHO STDOUT)

        if(DEFINED ENV{WINDOWSSDKDIR})
            # If WINDOWSSDKDIR env var is defined, use its value. This is defined, for example, when
            # using a Visual Studio command prompt.
            set(WIN10_SDK_PATH "$ENV{WINDOWSSDKDIR}")
            message(STATUS "Found WINDOWSSDKDIR environment variable: ${WIN10_SDK_PATH}")
        else()
            # There's no easy way to get the Windows SDK path in CMake; however, conveniently, DXC
            # contains a FindD3D12.cmake file that returns WIN10_SDK_PATH and WIN10_SDK_VERSION,
            # so let's use that.
            # TODO(crbug.com/tint/2106): Get the Win10 SDK path and version ourselves until
            # dxc/cmake/modules/FindD3D12.cmake supports non-VS generators.
            get_filename_component(WIN10_SDK_PATH "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]" ABSOLUTE CACHE)
            message(STATUS "WINDOWSSDKDIR environment variable is not defined, retrieving from registry: ${WIN10_SDK_PATH}")
        endif()

        message(STATUS "Finding Windows SDK version")
        if (CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
            set (WIN10_SDK_VERSION ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION})
            message(STATUS "Found Windows SDK version from CMake variable 'CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION': ${WIN10_SDK_VERSION}")
        else()
            # CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION may not be defined if, for example,
            # the Ninja generator is used instead of Visual Studio. Attempt to retrieve the
            # most recent SDK version from the list of paths under "${WIN10_SDK_PATH}/Include/".
            file(GLOB sdk_dirs RELATIVE "${WIN10_SDK_PATH}/Include/" "${WIN10_SDK_PATH}/Include/10.*")
            if (sdk_dirs)
                list(POP_BACK sdk_dirs WIN10_SDK_VERSION)
            endif()
            unset(sdk_dirs)
            message(STATUS "Windows SDK version not found from CMake variable 'CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION', attempted to find from SDK path: ${WIN10_SDK_VERSION}")
        endif()

        set(DXIL_DLL_PATH "${WIN10_SDK_PATH}/bin/${WIN10_SDK_VERSION}/x64/dxil.dll")
        add_custom_target(copy_dxil_dll)
        add_custom_command(
            TARGET copy_dxil_dll
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DXIL_DLL_PATH} $<TARGET_FILE_DIR:dxcompiler>
            COMMENT "Copying ${DXIL_DLL_PATH} to $<TARGET_FILE_DIR:dxcompiler>")
        # Make dxc target depend on copy_dxil_dll
        add_dependencies(dxc copy_dxil_dll)
    endif()
endfunction()

if (DAWN_USE_BUILT_DXC)
    AddSubdirectoryDXC()
endif()

if (TINT_BUILD_TINTD)
    set(LANGSVR_JSON_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp")
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp" EXCLUDE_FROM_ALL)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/langsvr" EXCLUDE_FROM_ALL)
endif()
