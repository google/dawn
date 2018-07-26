# Copyright 2017 The Dawn Authors
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
project(dawn C CXX)

# List TARGET_OBJECTS in SOURCES target property.
cmake_policy(SET CMP0051 NEW)

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

option(DAWN_USE_WERROR "Treat warnings as error (useful for CI)" 0)

# Default values for the backend-enabling options
set(ENABLE_D3D12 OFF)
set(ENABLE_METAL OFF)
if (WIN32)
    set(ENABLE_D3D12 ON)
elseif(APPLE)
    set(ENABLE_METAL ON)
endif()

option(DAWN_ENABLE_D3D12 "Enable compilation of the D3D12 backend" ${ENABLE_D3D12})
option(DAWN_ENABLE_METAL "Enable compilation of the Metal backend" ${ENABLE_METAL})
option(DAWN_ENABLE_NULL "Enable compilation of the Null backend" ON)
option(DAWN_ENABLE_OPENGL "Enable compilation of the OpenGL backend" ON)
option(DAWN_ENABLE_VULKAN "Enable compilation of the Vulkan backend" OFF)
option(DAWN_ALWAYS_ASSERT "Enable assertions on all build types" OFF)
option(DAWN_USE_CPP17 "Use some optional C++17 features for compile-time checks" OFF)

################################################################################
# Precompute compile flags and defines, functions to set them
################################################################################

set(DAWN_FLAGS "")
set(DAWN_DEFS "")
set(DAWN_INTERNAL_FLAGS "")
set(DAWN_INTERNAL_DEFS "")
set(DAWN_GENERATED_FLAGS "")

set(DAWN_ENABLE_ASSERTS $<OR:$<CONFIG:Debug>,$<BOOL:${DAWN_ALWAYS_ASSERT}>>)

list(APPEND DAWN_DEFS $<${DAWN_ENABLE_ASSERTS}:DAWN_ENABLE_ASSERTS>)

if (DAWN_USE_CPP17)
    list(APPEND DAWN_INTERNAL_DEFS "DAWN_CPP_VERSION=17")
else()
    list(APPEND DAWN_INTERNAL_DEFS "DAWN_CPP_VERSION=14")
endif()

if (DAWN_ENABLE_D3D12)
    list(APPEND DAWN_INTERNAL_DEFS "DAWN_ENABLE_BACKEND_D3D12")
endif()
if (DAWN_ENABLE_METAL)
    list(APPEND DAWN_INTERNAL_DEFS "DAWN_ENABLE_BACKEND_METAL")
endif()
if (DAWN_ENABLE_NULL)
    list(APPEND DAWN_INTERNAL_DEFS "DAWN_ENABLE_BACKEND_NULL")
endif()
if (DAWN_ENABLE_OPENGL)
    list(APPEND DAWN_INTERNAL_DEFS "DAWN_ENABLE_BACKEND_OPENGL")
endif()
if (DAWN_ENABLE_VULKAN)
    list(APPEND DAWN_INTERNAL_DEFS "DAWN_ENABLE_BACKEND_VULKAN")
endif()

if (WIN32)
    # Define NOMINMAX to prevent conflicts between std::min/max and the min/max macros in WinDef.h
    list(APPEND DAWN_DEFS "NOMINMAX")
    # Avoid Windows.h including a lot of headers
    list(APPEND DAWN_DEFS "WIN32_LEAN_AND_MEAN")
    # Remove compile error where the mock Dawn creates too many sections for the old obj format.
    list(APPEND DAWN_FLAGS "/bigobj")
endif()

if (MSVC)
    list(APPEND DAWN_FLAGS "/std:c++14")
    list(APPEND DAWN_FLAGS "/EHsc")
    list(APPEND DAWN_FLAGS "/MP")
    list(APPEND DAWN_INTERNAL_FLAGS "/W4")
    # Allow declarations hiding members as it is used all over Dawn
    list(APPEND DAWN_INTERNAL_FLAGS "/wd4458")
    list(APPEND DAWN_INTERNAL_FLAGS "/wd4996") # Allow deprecated functions like strncpy

    list(APPEND DAWN_GENERATED_FLAGS "/wd4702") # Allow unreachable code
    list(APPEND DAWN_GENERATED_FLAGS "/wd4189") # Allow unused variable

    if(DAWN_USE_WERROR)
        list(APPEND DAWN_INTERNAL_FLAGS "/WX")
    endif()
else()
    # Activate C++14 only on C++ files, not C files.
    list(APPEND DAWN_FLAGS "$<$<COMPILE_LANGUAGE:CXX>:-std=c++14>")
    # enable -Wold-style-cast on C++
    list(APPEND DAWN_FLAGS "$<$<COMPILE_LANGUAGE:CXX>:-Wold-style-cast>")

    list(APPEND DAWN_FLAGS "-fvisibility=hidden" "$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>" "-fno-exceptions")
    list(APPEND DAWN_FLAGS "-fPIC")

    list(APPEND DAWN_INTERNAL_FLAGS "-Wall" "-Wextra")
    list(APPEND DAWN_INTERNAL_FLAGS "-pedantic")
    list(APPEND DAWN_GENERATED_FLAGS "-Wno-unused-variable" "-Wno-unused-function")

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        # don't break the build on older clang versions
        list(APPEND DAWN_INTERNAL_FLAGS "-Wno-error=unknown-warning-option")
        # GCC's conversion warnings are less useful than clang's
        list(APPEND DAWN_INTERNAL_FLAGS "-Wconversion" "-Wno-sign-conversion")
        # disable a clang-only -pedantic warning
        list(APPEND DAWN_INTERNAL_FLAGS "-Wno-gnu-zero-variadic-macro-arguments")
        # additional potentially useful warnings (feel free to remove if they prove un-useful)
        list(APPEND DAWN_INTERNAL_FLAGS "-Wextra-semi")
        list(APPEND DAWN_INTERNAL_FLAGS "-Wstrict-aliasing")
        list(APPEND DAWN_INTERNAL_FLAGS "-Wunreachable-code")
        list(APPEND DAWN_GENERATED_FLAGS "-Wno-unreachable-code")
        # Probably okay to enable if we establish a field naming convention:
        #list(APPEND DAWN_INTERNAL_FLAGS "-Wshadow")
    endif()
    if(DAWN_USE_WERROR)
        list(APPEND DAWN_INTERNAL_FLAGS "-Werror")
    endif()
endif()

function(DawnExternalTarget folder target)
    set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS ${DAWN_FLAGS})
    set_property(TARGET ${target} APPEND PROPERTY COMPILE_DEFINITIONS ${DAWN_DEFS})
    set_property(TARGET ${target} PROPERTY FOLDER "dawn/${folder}")
endfunction()

function(DawnInternalTarget folder target)
    DawnExternalTarget("${folder}" ${target})
    set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS ${DAWN_INTERNAL_FLAGS})
    set_property(TARGET ${target} APPEND PROPERTY COMPILE_DEFINITIONS ${DAWN_INTERNAL_DEFS})

    # Common include directories shared by all internal targets
    target_include_directories(${target} PRIVATE ${SRC_DIR} ${GENERATED_DIR} ${INCLUDE_DIR})

    # All internal targets require the headers to have been generated
    add_dependencies(${target} dawn_headers)

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

# Output shared libs and executables directly in the build directory
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

################################################################################
# Generate the C and C++ Dawn APIs
################################################################################

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory(generator)

# Dawn header generation is in its own target so that it can be set as a build dependency for all
# internal targets
Generate(
    LIB_NAME dawn_headers
    LIB_TYPE OBJECT
    FOLDER ""
    PRINT_NAME "Dawn headers"
    COMMAND_LINE_ARGS
        ${GENERATOR_COMMON_ARGS}
        -T dawn_headers
)
# Older versions of CMake aren't able to know which linker to use without this and fail.
set_property(TARGET dawn_headers PROPERTY LINKER_LANGUAGE "CXX")

# libdawn.so/dll/dylib which contains the static proctable C interface and its C++ wrapper
Generate(
    LIB_NAME libdawn_autogen
    LIB_TYPE OBJECT
    FOLDER "libdawn"
    PRINT_NAME "libdawn"
    COMMAND_LINE_ARGS
        ${GENERATOR_COMMON_ARGS}
        -T libdawn
)
target_compile_definitions(libdawn_autogen PRIVATE DAWN_IMPLEMENTATION)

add_library(libdawn SHARED
    $<TARGET_OBJECTS:libdawn_autogen>
    ${INCLUDE_DIR}/dawn/dawn_export.h
    ${INCLUDE_DIR}/dawn/dawn_wsi.h
    ${INCLUDE_DIR}/dawn/EnumClassBitmasks.h
)
set_property(TARGET libdawn PROPERTY OUTPUT_NAME "dawn")
target_include_directories(libdawn PUBLIC ${GENERATED_DIR} ${INCLUDE_DIR})
DawnInternalTarget("libdawn" libdawn)

################################################################################
# Call to other CMakeLists.txt
################################################################################

add_subdirectory(third_party)

add_subdirectory(src/common)
add_subdirectory(src/dawn_native)
add_subdirectory(src/dawn_wire)
add_subdirectory(src/utils)
add_subdirectory(src/tests)

add_subdirectory(examples)
