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

################################################################################
# Configuration options
################################################################################

option(NXT_USE_WERROR "Treat warnings as error (useful for CI)" 0)

################################################################################
# Precompute compile flags and defines, functions to set them
################################################################################

set(NXT_FLAGS "")
set(NXT_DEFS "")
set(NXT_INTERNAL_FLAGS "")
set(NXT_INTERNAL_DEFS "")

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    list(APPEND NXT_DEFS "NXT_ENABLE_ASSERTS")
endif()

if (WIN32)
    # Define NOMINMAX to prevent conflicts between std::min/max and the min/max macros in WinDef.h
    list(APPEND NXT_DEFS "NOMINMAX")
    # Remove compile error where the mock NXT creates too many sections for the old obj format.
    list(APPEND NXT_FLAGS "/bigobj")
endif()

if (MSVC)
    list(APPEND NXT_FLAGS "/std:c++14")
else()
    # Activate C++14 only on C++ files, not C files.
    list(APPEND NXT_FLAGS "$<$<STREQUAL:$<TARGET_PROPERTY:LINKER_LANGUAGE>,CXX>:-std=c++14>")
    list(APPEND NXT_FLAGS "-fPIC")

    list(APPEND NXT_INTERNAL_FLAGS "-Wall" "-Wextra")
    list(APPEND NXT_GENERATED_FLAGS "-Wno-unused-variable")
    if(NXT_USE_WERROR)
        list(APPEND NXT_INTERNAL_FLAGS "-Werror")
    endif()
endif()

function(NXTExternalTarget folder target)
    set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS ${NXT_FLAGS})
    set_property(TARGET ${target} APPEND PROPERTY COMPILE_DEFINITIONS ${NXT_DEFS})
    set_property(TARGET ${target} PROPERTY FOLDER ${folder})
endfunction()

function(NXTInternalTarget folder target)
    NXTExternalTarget("${folder}" ${target})
    set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS ${NXT_INTERNAL_FLAGS})
    set_property(TARGET ${target} APPEND PROPERTY COMPILE_DEFINITIONS ${NXT_INTERNAL_DEFS})
endfunction()

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
