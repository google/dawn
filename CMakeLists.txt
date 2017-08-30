# Copyright 2017 The NXT Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 2.8)
project(nxt C CXX)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR
        "In-source builds are unsupported. Use another directory, like build/, "
        "as your CMake build directory.\n"
        "Note: CMakeFiles/ and CMakeCache.txt may have been generated in the "
        "source directory. These may be removed.")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
        "Build type (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
endif()

################################################################################
# Configuration options
################################################################################

option(NXT_USE_WERROR "Treat warnings as error (useful for CI)" 0)

# Default values for the backend-enabling options
set(ENABLE_D3D12 OFF)
set(ENABLE_METAL OFF)
if (WIN32)
    set(ENABLE_D3D12 ON)
elseif(APPLE)
    set(ENABLE_METAL ON)
endif()

option(NXT_ENABLE_D3D12 "Enable compilation of the D3D12 backend" ${ENABLE_D3D12})
option(NXT_ENABLE_METAL "Enable compilation of the Metal backend" ${ENABLE_METAL})
option(NXT_ENABLE_NULL "Enable compilation of the Null backend" ON)
option(NXT_ENABLE_OPENGL "Enable compilation of the OpenGL backend" ON)
option(NXT_ENABLE_VULKAN "Enable compilation of the Vulkan backend" OFF)
option(NXT_ALWAYS_ASSERT "Enable assertions on all build types" OFF)

################################################################################
# Precompute compile flags and defines, functions to set them
################################################################################

set(NXT_FLAGS "")
set(NXT_DEFS "")
set(NXT_INTERNAL_FLAGS "")
set(NXT_INTERNAL_DEFS "")
set(NXT_GENERATED_FLAGS "")

set(NXT_ENABLE_ASSERTS $<OR:$<CONFIG:Debug>,$<BOOL:${NXT_ALWAYS_ASSERT}>>)

list(APPEND NXT_DEFS $<${NXT_ENABLE_ASSERTS}:NXT_ENABLE_ASSERTS>)

if (NXT_ENABLE_D3D12)
    list(APPEND NXT_INTERNAL_DEFS "NXT_ENABLE_BACKEND_D3D12")
endif()
if (NXT_ENABLE_METAL)
    list(APPEND NXT_INTERNAL_DEFS "NXT_ENABLE_BACKEND_METAL")
endif()
if (NXT_ENABLE_NULL)
    list(APPEND NXT_INTERNAL_DEFS "NXT_ENABLE_BACKEND_NULL")
endif()
if (NXT_ENABLE_OPENGL)
    list(APPEND NXT_INTERNAL_DEFS "NXT_ENABLE_BACKEND_OPENGL")
endif()
if (NXT_ENABLE_VULKAN)
    list(APPEND NXT_INTERNAL_DEFS "NXT_ENABLE_BACKEND_VULKAN")
endif()

if (WIN32)
    # Define NOMINMAX to prevent conflicts between std::min/max and the min/max macros in WinDef.h
    list(APPEND NXT_DEFS "NOMINMAX")
    # Avoid Windows.h including a lot of headers
    list(APPEND NXT_DEFS "WIN32_LEAN_AND_MEAN")
    # Remove compile error where the mock NXT creates too many sections for the old obj format.
    list(APPEND NXT_FLAGS "/bigobj")
endif()

if (MSVC)
    list(APPEND NXT_FLAGS "/std:c++14")
    list(APPEND NXT_FLAGS "/EHsc")
    list(APPEND NXT_INTERNAL_FLAGS "/W4")
    # Allow declarations hiding members as it is used all over NXT
    list(APPEND NXT_INTERNAL_FLAGS "/wd4458")
    list(APPEND NXT_INTERNAL_FLAGS "/wd4996") # Allow deprecated functions like strncpy

    list(APPEND NXT_GENERATED_FLAGS "/wd4702") # Allow unreachable code
    list(APPEND NXT_GENERATED_FLAGS "/wd4189") # Allow unused variable

    if(NXT_USE_WERROR)
        list(APPEND NXT_INTERNAL_FLAGS "/WX")
    endif()
else()
    # Activate C++14 only on C++ files, not C files.
    list(APPEND NXT_FLAGS "$<$<STREQUAL:$<TARGET_PROPERTY:LINKER_LANGUAGE>,CXX>:-std=c++14>")
    # enable -Wold-style-cast on C++
    list(APPEND NXT_FLAGS "$<$<STREQUAL:$<TARGET_PROPERTY:LINKER_LANGUAGE>,CXX>:-Wold-style-cast>")
    list(APPEND NXT_FLAGS "-fPIC")

    list(APPEND NXT_INTERNAL_FLAGS "-Wall" "-Wextra")
    list(APPEND NXT_INTERNAL_FLAGS "-pedantic")
    list(APPEND NXT_GENERATED_FLAGS "-Wno-unused-variable" "-Wno-unused-function")

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        # don't break the build on older clang versions
        list(APPEND NXT_INTERNAL_FLAGS "-Wno-error=unknown-warning-option")
        # GCC's conversion warnings are less useful than clang's
        list(APPEND NXT_INTERNAL_FLAGS "-Wconversion" "-Wno-sign-conversion")
        # disable a clang-only -pedantic warning
        list(APPEND NXT_INTERNAL_FLAGS "-Wno-gnu-zero-variadic-macro-arguments")
        # additional potentially useful warnings (feel free to remove if they prove un-useful)
        list(APPEND NXT_INTERNAL_FLAGS "-Wextra-semi")
        list(APPEND NXT_INTERNAL_FLAGS "-Wstrict-aliasing")
        list(APPEND NXT_INTERNAL_FLAGS "-Wunreachable-code")
        list(APPEND NXT_GENERATED_FLAGS "-Wno-unreachable-code")
        # Probably okay to enable if we establish a field naming convention:
        #list(APPEND NXT_INTERNAL_FLAGS "-Wshadow")
    endif()
    if(NXT_USE_WERROR)
        list(APPEND NXT_INTERNAL_FLAGS "-Werror")
    endif()
endif()

function(NXTExternalTarget folder target)
    set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS ${NXT_FLAGS})
    set_property(TARGET ${target} APPEND PROPERTY COMPILE_DEFINITIONS ${NXT_DEFS})
    set_property(TARGET ${target} PROPERTY FOLDER "nxt/${folder}")
endfunction()

function(NXTInternalTarget folder target)
    NXTExternalTarget("${folder}" ${target})
    set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS ${NXT_INTERNAL_FLAGS})
    set_property(TARGET ${target} APPEND PROPERTY COMPILE_DEFINITIONS ${NXT_INTERNAL_DEFS})

    # Group the target sources by folder to have folders show in Visual Studio
    if (MSVC)
        get_target_property(targetSources ${target} SOURCES)
        foreach(sourceFile IN ITEMS ${targetSources})
            if (IS_ABSOLUTE "${sourceFile}")
                file(RELATIVE_PATH sourceFile "${CMAKE_CURRENT_SOURCE_DIR}" "${sourceFile}")
            endif()
            get_filename_component(sourceDir "${sourceFile}" PATH)
            string(REPLACE "/" "\\" sourceDir "${sourceDir}")
            source_group("${sourceDir}" FILES "${sourceFile}")
        endforeach()
    endif()
endfunction()

# Enable the creation of folders for Visual Studio projects
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################################################################################
# Generate the C and C++ NXT APIs
################################################################################

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory(generator)

Generate(
    LIB_NAME nxt
    LIB_TYPE STATIC
    PRINT_NAME libNXT
    COMMAND_LINE_ARGS
        ${GENERATOR_COMMON_ARGS}
        -T nxt
)
target_include_directories(nxt PUBLIC ${GENERATED_DIR})

Generate(
    LIB_NAME nxtcpp
    LIB_TYPE STATIC
    PRINT_NAME libNXT++
    COMMAND_LINE_ARGS
        ${GENERATOR_COMMON_ARGS}
        -T nxtcpp
)
target_include_directories(nxtcpp PUBLIC ${GENERATED_DIR} PUBLIC ${INCLUDE_DIR})
target_link_libraries(nxtcpp nxt)

################################################################################
# Call to other CMakeLists.txt
################################################################################

add_subdirectory(third_party)

add_subdirectory(src/common)
add_subdirectory(src/backend)
add_subdirectory(src/wire)
add_subdirectory(src/utils)
add_subdirectory(src/tests)

add_subdirectory(examples)
