# Copyright 2022 The Dawn & Tint Authors
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

cmake_minimum_required(VERSION 3.10.2)

# When upgrading to CMake 3.11 we can remove DAWN_PLACEHOLDER_FILE because source-less add_library
# becomes available.
# When upgrading to CMake 3.12 we should add CONFIGURE_DEPENDS to DawnGenerator to rerun CMake in
# case any of the generator files changes. We should also remove the CACHE "" FORCE stuff to
# override options in third_party dependencies. We can also add the HOMEPAGE_URL
# entry to the project `HOMEPAGE_URL "https://dawn.googlesource.com/dawn"`

project(
    Dawn
    DESCRIPTION "Dawn, a WebGPU implementation"
    LANGUAGES C CXX
)
enable_testing()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_DEBUG_POSTFIX "")

if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
  message(STATUS "No build type selected, default to Debug")
  set(CMAKE_BUILD_TYPE "Debug")
endif()

set(DAWN_BUILD_GEN_DIR "${Dawn_BINARY_DIR}/gen")
set(DAWN_GENERATOR_DIR "${Dawn_SOURCE_DIR}/generator")
set(DAWN_SRC_DIR "${Dawn_SOURCE_DIR}/src")
set(DAWN_INCLUDE_DIR "${Dawn_SOURCE_DIR}/include")
set(DAWN_TEMPLATE_DIR "${DAWN_GENERATOR_DIR}/templates")

set(DAWN_PLACEHOLDER_FILE "${DAWN_SRC_DIR}/Placeholder.cpp")

################################################################################
# Configuration options
################################################################################

# option_if_not_defined(name description default)
# Behaves like:
#   option(name description default)
# If a variable is not already defined with the given name, otherwise the
# function does nothing.
# Simplifies customization by projects that use Dawn as a dependency.
function (option_if_not_defined name description default)
    if(NOT DEFINED ${name})
        option(${name} ${description} ${default})
    endif()
endfunction()

# set_if_not_defined(name value description)
# Behaves like:
#   set(${name} ${value} CACHE STRING ${description})
# If a variable is not already defined with the given name, otherwise the
# function does nothing.
# Simplifies customization by projects that use Dawn as a dependency.
function (set_if_not_defined name value description)
    if(NOT DEFINED ${name})
        set(${name} ${value} CACHE STRING ${description})
    endif()
endfunction()

# Default values for the backend-enabling options
set(ENABLE_D3D12 OFF)
set(ENABLE_METAL OFF)
set(ENABLE_OPENGLES OFF)
set(ENABLE_DESKTOP_GL OFF)
set(ENABLE_VULKAN OFF)
set(USE_WAYLAND OFF)
set(USE_X11 OFF)
set(BUILD_SAMPLES OFF)
if (WIN32)
    set(ENABLE_D3D12 ON)
    if (NOT WINDOWS_STORE)
        # Enable Vulkan in win32 compilation only
        # since UWP only supports d3d
        set(ENABLE_VULKAN ON)
    endif()
elseif(APPLE)
    set(ENABLE_METAL ON)
elseif(ANDROID)
    set(ENABLE_VULKAN ON)
    set(ENABLE_OPENGLES ON)
elseif(UNIX)
    set(ENABLE_OPENGLES ON)
    set(ENABLE_DESKTOP_GL ON)
    set(ENABLE_VULKAN ON)
    set(USE_X11 ON)
endif()

# GLFW is not supported in UWP
if ((WIN32 AND NOT WINDOWS_STORE) OR UNIX AND NOT ANDROID)
    set(DAWN_SUPPORTS_GLFW_FOR_WINDOWING ON)
endif()

# Current examples are depend on GLFW
if (DAWN_SUPPORTS_GLFW_FOR_WINDOWING)
    set(BUILD_SAMPLES ON)
endif()

option_if_not_defined(DAWN_ENABLE_ASAN "Enable address sanitizer" OFF)
option_if_not_defined(DAWN_ENABLE_TSAN "Enable thread sanitizer" OFF)
option_if_not_defined(DAWN_ENABLE_MSAN "Enable memory sanitizer" OFF)
option_if_not_defined(DAWN_ENABLE_UBSAN "Enable undefined behaviour sanitizer" OFF)

option_if_not_defined(DAWN_ENABLE_D3D12 "Enable compilation of the D3D12 backend" ${ENABLE_D3D12})
option_if_not_defined(DAWN_ENABLE_METAL "Enable compilation of the Metal backend" ${ENABLE_METAL})
option_if_not_defined(DAWN_ENABLE_NULL "Enable compilation of the Null backend" ON)
option_if_not_defined(DAWN_ENABLE_DESKTOP_GL "Enable compilation of the OpenGL backend" ${ENABLE_DESKTOP_GL})
option_if_not_defined(DAWN_ENABLE_OPENGLES "Enable compilation of the OpenGL ES backend" ${ENABLE_OPENGLES})
option_if_not_defined(DAWN_ENABLE_VULKAN "Enable compilation of the Vulkan backend" ${ENABLE_VULKAN})

option_if_not_defined(DAWN_ALWAYS_ASSERT "Enable assertions on all build types" OFF)
option_if_not_defined(DAWN_USE_WAYLAND "Enable support for Wayland surface" ${USE_WAYLAND})
option_if_not_defined(DAWN_USE_X11 "Enable support for X11 surface" ${USE_X11})

option_if_not_defined(DAWN_BUILD_SAMPLES "Enables building Dawn's samples" ${BUILD_SAMPLES})
option_if_not_defined(DAWN_BUILD_NODE_BINDINGS "Enables building Dawn's NodeJS bindings" OFF)
option_if_not_defined(DAWN_ENABLE_SWIFTSHADER "Enables building Swiftshader as part of the build and Vulkan adapter discovery" OFF)

option_if_not_defined(DAWN_ENABLE_PIC "Build with Position-Independent-Code enabled" OFF)

option_if_not_defined(DAWN_EMIT_COVERAGE "Emit code coverage information" OFF)
set_if_not_defined(LLVM_SOURCE_DIR "${Dawn_LLVM_SOURCE_DIR}" "Directory to an LLVM source checkout. Required to build turbo-cov")

if (DAWN_ENABLE_OPENGLES OR DAWN_ENABLE_DESKTOP_GL)
  set(TINT_DEFAULT_GLSL ON)
else()
  set(TINT_DEFAULT_GLSL OFF)
endif()

option_if_not_defined(TINT_BUILD_SAMPLES "Build samples" ${DAWN_BUILD_SAMPLES})
option_if_not_defined(TINT_BUILD_DOCS "Build documentation" ON)
option_if_not_defined(TINT_DOCS_WARN_AS_ERROR "When building documentation, treat warnings as errors" OFF)

option_if_not_defined(TINT_BUILD_SPV_READER "Build the SPIR-V input reader" ${DAWN_ENABLE_VULKAN})
option_if_not_defined(TINT_BUILD_WGSL_READER "Build the WGSL input reader" ON)
option_if_not_defined(TINT_BUILD_GLSL_WRITER "Build the GLSL output writer" ${TINT_DEFAULT_GLSL})
option_if_not_defined(TINT_BUILD_HLSL_WRITER "Build the HLSL output writer" ${DAWN_ENABLE_D3D12})
option_if_not_defined(TINT_BUILD_MSL_WRITER "Build the MSL output writer" ${DAWN_ENABLE_METAL})
option_if_not_defined(TINT_BUILD_SPV_WRITER "Build the SPIR-V output writer" ${DAWN_ENABLE_VULKAN})
option_if_not_defined(TINT_BUILD_WGSL_WRITER "Build the WGSL output writer" ON)

option_if_not_defined(TINT_BUILD_IR "Build the IR" ON)

option_if_not_defined(TINT_BUILD_FUZZERS "Build fuzzers" OFF)
option_if_not_defined(TINT_BUILD_SPIRV_TOOLS_FUZZER "Build SPIRV-Tools fuzzer" OFF)
option_if_not_defined(TINT_BUILD_AST_FUZZER "Build AST fuzzer" OFF)
option_if_not_defined(TINT_BUILD_REGEX_FUZZER "Build regex fuzzer" OFF)
option_if_not_defined(TINT_BUILD_BENCHMARKS "Build benchmarks" OFF)
option_if_not_defined(TINT_BUILD_TESTS "Build tests" ON)
option_if_not_defined(TINT_BUILD_AS_OTHER_OS "Override OS detection to force building of *_other.cc files" OFF)
option_if_not_defined(TINT_BUILD_REMOTE_COMPILE "Build the remote-compile tool for validating shaders on a remote machine" OFF)

option_if_not_defined(TINT_ENABLE_BREAK_IN_DEBUGGER "Enable tint::debugger::Break()" OFF)
option_if_not_defined(TINT_CHECK_CHROMIUM_STYLE "Check for [chromium-style] issues during build" OFF)
option_if_not_defined(TINT_SYMBOL_STORE_DEBUG_NAME "Enable storing of name in tint::ast::Symbol to help debugging the AST" OFF)
option_if_not_defined(TINT_RANDOMIZE_HASHES "Randomize the hash seed value to detect non-deterministic output" OFF)

# Recommended setting for compability with future abseil releases.
set(ABSL_PROPAGATE_CXX_STD ON)

set_if_not_defined(DAWN_THIRD_PARTY_DIR "${Dawn_SOURCE_DIR}/third_party" "Directory in which to find third-party dependencies.")
set_if_not_defined(DAWN_ABSEIL_DIR "${DAWN_THIRD_PARTY_DIR}/abseil-cpp" "Directory in which to find Abseil")
set_if_not_defined(DAWN_GLFW_DIR "${DAWN_THIRD_PARTY_DIR}/glfw" "Directory in which to find GLFW")
set_if_not_defined(DAWN_JINJA2_DIR "${DAWN_THIRD_PARTY_DIR}/jinja2" "Directory in which to find Jinja2")
set_if_not_defined(DAWN_KHRONOS_DIR "${DAWN_THIRD_PARTY_DIR}/khronos" "Directory in which to find Khronos GL headers")
set_if_not_defined(DAWN_SPIRV_HEADERS_DIR "${DAWN_THIRD_PARTY_DIR}/vulkan-deps/spirv-headers/src" "Directory in which to find SPIRV-Headers")
set_if_not_defined(DAWN_SPIRV_TOOLS_DIR "${DAWN_THIRD_PARTY_DIR}/vulkan-deps/spirv-tools/src" "Directory in which to find SPIRV-Tools")
set_if_not_defined(DAWN_SWIFTSHADER_DIR "${DAWN_THIRD_PARTY_DIR}/swiftshader" "Directory in which to find swiftshader")
set_if_not_defined(DAWN_TINT_DIR "${Dawn_SOURCE_DIR}" "Directory in which to find Tint")
set_if_not_defined(DAWN_VULKAN_DEPS_DIR "${DAWN_THIRD_PARTY_DIR}/vulkan-deps" "Directory in which to find vulkan-deps")
set_if_not_defined(DAWN_VULKAN_HEADERS_DIR "${DAWN_VULKAN_DEPS_DIR}/vulkan-headers/src" "Directory in which to find Vulkan-Headers")
set_if_not_defined(DAWN_VULKAN_TOOLS_DIR "${DAWN_VULKAN_DEPS_DIR}/vulkan-tools/src" "Directory in which to find Vulkan-Tools")

# Dependencies for DAWN_BUILD_NODE_BINDINGS
set_if_not_defined(NODE_ADDON_API_DIR "${DAWN_THIRD_PARTY_DIR}/node-addon-api" "Directory in which to find node-addon-api")
set_if_not_defined(NODE_API_HEADERS_DIR "${DAWN_THIRD_PARTY_DIR}/node-api-headers" "Directory in which to find node-api-headers")
set_if_not_defined(WEBGPU_IDL_PATH "${DAWN_THIRD_PARTY_DIR}/gpuweb/webgpu.idl" "Path to the webgpu.idl definition file")
set_if_not_defined(GO_EXECUTABLE "go" "Golang executable for running the IDL generator")

# Much of the backend code is shared among desktop OpenGL and OpenGL ES
if (${DAWN_ENABLE_DESKTOP_GL} OR ${DAWN_ENABLE_OPENGLES})
    set(DAWN_ENABLE_OPENGL ON)
endif()

if(DAWN_ENABLE_PIC)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

if (${TINT_BUILD_SPIRV_TOOLS_FUZZER})
  message(STATUS "TINT_BUILD_SPIRV_TOOLS_FUZZER is ON - setting
      TINT_BUILD_FUZZERS
      TINT_BUILD_SPV_READER
      TINT_BUILD_SPV_WRITER
      TINT_BUILD_WGSL_READER
      TINT_BUILD_WGSL_WRITER
      TINT_BUILD_GLSL_WRITER
      TINT_BUILD_HLSL_WRITER
      TINT_BUILD_MSL_WRITER to ON")
  set(TINT_BUILD_FUZZERS ON CACHE BOOL "Build tint fuzzers" FORCE)
  set(TINT_BUILD_SPV_READER ON CACHE BOOL "Build SPIR-V reader" FORCE)
  set(TINT_BUILD_SPV_WRITER ON CACHE BOOL "Build SPIR-V writer" FORCE)
  set(TINT_BUILD_WGSL_READER ON CACHE BOOL "Build WGSL reader" FORCE)
  set(TINT_BUILD_WGSL_WRITER ON CACHE BOOL "Build WGSL writer" FORCE)
  set(TINT_BUILD_GLSL_WRITER ON CACHE BOOL "Build HLSL writer" FORCE)
  set(TINT_BUILD_HLSL_WRITER ON CACHE BOOL "Build HLSL writer" FORCE)
  set(TINT_BUILD_MSL_WRITER ON CACHE BOOL "Build MSL writer" FORCE)
endif()

if (${TINT_BUILD_AST_FUZZER})
  message(STATUS "TINT_BUILD_AST_FUZZER is ON - setting
      TINT_BUILD_FUZZERS
      TINT_BUILD_WGSL_READER
      TINT_BUILD_WGSL_WRITER
      TINT_BUILD_SPV_WRITER
      TINT_BUILD_MSL_WRITER
      TINT_BUILD_GLSL_WRITER
      TINT_BUILD_HLSL_WRITER to ON")
  set(TINT_BUILD_FUZZERS ON CACHE BOOL "Build tint fuzzers" FORCE)
  set(TINT_BUILD_WGSL_READER ON CACHE BOOL "Build WGSL reader" FORCE)
  set(TINT_BUILD_WGSL_WRITER ON CACHE BOOL "Build WGSL writer" FORCE)
  set(TINT_BUILD_SPV_WRITER ON CACHE BOOL "Build SPIR-V writer" FORCE)
  set(TINT_BUILD_MSL_WRITER ON CACHE BOOL "Build MSL writer" FORCE)
  set(TINT_BUILD_GLSL_WRITER ON CACHE BOOL "Build GLSL writer" FORCE)
  set(TINT_BUILD_HLSL_WRITER ON CACHE BOOL "Build HLSL writer" FORCE)
endif()

if (${TINT_BUILD_REGEX_FUZZER})
  message(STATUS "TINT_BUILD_REGEX_FUZZER is ON - setting
      TINT_BUILD_FUZZERS
      TINT_BUILD_WGSL_READER
      TINT_BUILD_WGSL_WRITER
      TINT_BUILD_SPV_WRITER
      TINT_BUILD_MSL_WRITER
      TINT_BUILD_GLSL_WRITER
      TINT_BUILD_HLSL_WRITER to ON")
      set(TINT_BUILD_FUZZERS ON CACHE BOOL "Build tint fuzzers" FORCE)
      set(TINT_BUILD_WGSL_READER ON CACHE BOOL "Build WGSL reader" FORCE)
      set(TINT_BUILD_WGSL_WRITER ON CACHE BOOL "Build WGSL writer" FORCE)
      set(TINT_BUILD_SPV_WRITER ON CACHE BOOL "Build SPIR-V writer" FORCE)
      set(TINT_BUILD_MSL_WRITER ON CACHE BOOL "Build MSL writer" FORCE)
      set(TINT_BUILD_GLSL_WRITER ON CACHE BOOL "Build GLSL writer" FORCE)
      set(TINT_BUILD_HLSL_WRITER ON CACHE BOOL "Build HLSL writer" FORCE)
endif()

message(STATUS "Dawn build D3D12 backend: ${DAWN_ENABLE_D3D12}")
message(STATUS "Dawn build Metal backend: ${DAWN_ENABLE_METAL}")
message(STATUS "Dawn build Vulkan backend: ${DAWN_ENABLE_VULKAN}")
message(STATUS "Dawn build OpenGL backend: ${DAWN_ENABLE_DESKTOP_GL}")
message(STATUS "Dawn build OpenGL ES backend: ${DAWN_ENABLE_OPENGLES}")
message(STATUS "Dawn build Null backend: ${DAWN_ENABLE_NULL}")

message(STATUS "Dawn build with asserts in all configurations: ${DAWN_ALWAYS_ASSERT}")
message(STATUS "Dawn build Wayland support: ${DAWN_USE_WAYLAND}")
message(STATUS "Dawn build X11 support: ${DAWN_USE_X11}")

message(STATUS "Dawn build samples: ${DAWN_BUILD_SAMPLES}")
message(STATUS "Dawn build Node bindings: ${DAWN_BUILD_NODE_BINDINGS}")
message(STATUS "Dawn build Swiftshader: ${DAWN_ENABLE_SWIFTSHADER}")

message(STATUS "Dawn build PIC: ${DAWN_ENABLE_PIC}")

message(STATUS "Dawn build with ASAN: ${DAWN_ENABLE_ASAN}")
message(STATUS "Dawn build with TSAN: ${DAWN_ENABLE_TSAN}")
message(STATUS "Dawn build with MSAN: ${DAWN_ENABLE_MSAN}")
message(STATUS "Dawn build with UBSAN: ${DAWN_ENABLE_UBSAN}")

message(STATUS "Tint build samples: ${TINT_BUILD_SAMPLES}")
message(STATUS "Tint build docs: ${TINT_BUILD_DOCS}")
message(STATUS "Tint build docs with warn as error: ${TINT_DOCS_WARN_AS_ERROR}")
message(STATUS "Tint build SPIR-V reader: ${TINT_BUILD_SPV_READER}")
message(STATUS "Tint build WGSL reader: ${TINT_BUILD_WGSL_READER}")
message(STATUS "Tint build GLSL writer: ${TINT_BUILD_GLSL_WRITER}")
message(STATUS "Tint build HLSL writer: ${TINT_BUILD_HLSL_WRITER}")
message(STATUS "Tint build MSL writer: ${TINT_BUILD_MSL_WRITER}")
message(STATUS "Tint build SPIR-V writer: ${TINT_BUILD_SPV_WRITER}")
message(STATUS "Tint build WGSL writer: ${TINT_BUILD_WGSL_WRITER}")
message(STATUS "Tint build IR: ${TINT_BUILD_IR}")
message(STATUS "Tint build fuzzers: ${TINT_BUILD_FUZZERS}")
message(STATUS "Tint build SPIRV-Tools fuzzer: ${TINT_BUILD_SPIRV_TOOLS_FUZZER}")
message(STATUS "Tint build AST fuzzer: ${TINT_BUILD_AST_FUZZER}")
message(STATUS "Tint build regex fuzzer: ${TINT_BUILD_REGEX_FUZZER}")
message(STATUS "Tint build benchmarks: ${TINT_BUILD_BENCHMARKS}")
message(STATUS "Tint build tests: ${TINT_BUILD_TESTS}")
message(STATUS "Tint build checking [chromium-style]: ${TINT_CHECK_CHROMIUM_STYLE}")
message(STATUS "Tint build remote-compile tool: ${TINT_BUILD_REMOTE_COMPILE}")

if (NOT ${TINT_LIB_FUZZING_ENGINE_LINK_OPTIONS} STREQUAL "")
  message(STATUS "Using provided LIB_FUZZING_ENGINE options: ${TINT_LIB_FUZZING_ENGINE_LINK_OPTIONS}")
endif()

message(STATUS "Using python3")
find_package(PythonInterp 3 REQUIRED)

################################################################################
# common_compile_options - sets compiler and linker options common for dawn and
# tint on the given target
################################################################################
function(common_compile_options TARGET)
  if (COMPILER_IS_LIKE_GNU)
    target_compile_options(${TARGET} PRIVATE
      -fno-exceptions
      -fno-rtti

      -Wno-deprecated-builtins
      -Wno-unknown-warning-option
    )

    if (${DAWN_ENABLE_MSAN})
      target_compile_options(${TARGET} PUBLIC -fsanitize=memory)
      target_link_options(${TARGET} PUBLIC -fsanitize=memory)
    elseif (${DAWN_ENABLE_ASAN})
      target_compile_options(${TARGET} PUBLIC -fsanitize=address)
      target_link_options(${TARGET} PUBLIC -fsanitize=address)
    elseif (${DAWN_ENABLE_TSAN})
      target_compile_options(${TARGET} PUBLIC -fsanitize=thread)
      target_link_options(${TARGET} PUBLIC -fsanitize=thread)
    elseif (${DAWN_ENABLE_UBSAN})
      target_compile_options(${TARGET} PUBLIC -fsanitize=undefined)
      target_link_options(${TARGET} PUBLIC -fsanitize=undefined)
    endif()
  endif(COMPILER_IS_LIKE_GNU)

  if(MSVC)
      target_compile_options(${TARGET} PUBLIC /utf-8)
  endif()

  if (DAWN_EMIT_COVERAGE)
    target_compile_definitions(${TARGET} PRIVATE "DAWN_EMIT_COVERAGE")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_compile_options(${TARGET} PRIVATE "--coverage")
        target_link_options(${TARGET} PRIVATE "gcov")
    elseif(COMPILER_IS_CLANG OR COMPILER_IS_CLANG_CL)
        target_compile_options(${TARGET} PRIVATE "-fprofile-instr-generate" "-fcoverage-mapping")
        target_link_options(${TARGET} PRIVATE "-fprofile-instr-generate" "-fcoverage-mapping")
    else()
        message(FATAL_ERROR "Coverage generation not supported for the ${CMAKE_CXX_COMPILER_ID} toolchain")
    endif()
  endif(DAWN_EMIT_COVERAGE)
endfunction()

if (${DAWN_ENABLE_TSAN})
  add_compile_options(-stdlib=libc++)
  add_link_options(-stdlib=libc++)
endif()

################################################################################
# Dawn's public and internal "configs"
################################################################################

set(IS_DEBUG_BUILD 0)
string(TOUPPER "${CMAKE_BUILD_TYPE}" build_type)
if ((NOT ${build_type} STREQUAL "RELEASE") AND (NOT ${build_type} STREQUAL "RELWITHDEBINFO"))
  set(IS_DEBUG_BUILD 1)
endif()

# The public config contains only the include paths for the Dawn headers.
add_library(dawn_public_config INTERFACE)
target_include_directories(dawn_public_config INTERFACE
    "${DAWN_INCLUDE_DIR}"
    "${DAWN_BUILD_GEN_DIR}/include"
)

# The internal config conatins additional path but includes the dawn_public_config include paths
add_library(dawn_internal_config INTERFACE)
target_include_directories(dawn_internal_config INTERFACE
    "${DAWN_SRC_DIR}"
    "${DAWN_BUILD_GEN_DIR}/src"
)
target_link_libraries(dawn_internal_config INTERFACE dawn_public_config)

# Compile definitions for the internal config
if (DAWN_ALWAYS_ASSERT OR IS_DEBUG_BUILD)
    target_compile_definitions(dawn_internal_config INTERFACE "DAWN_ENABLE_ASSERTS")
endif()
if (DAWN_ENABLE_D3D12)
    target_compile_definitions(dawn_internal_config INTERFACE "DAWN_ENABLE_BACKEND_D3D12")
endif()
if (DAWN_ENABLE_METAL)
    target_compile_definitions(dawn_internal_config INTERFACE "DAWN_ENABLE_BACKEND_METAL")
endif()
if (DAWN_ENABLE_NULL)
    target_compile_definitions(dawn_internal_config INTERFACE "DAWN_ENABLE_BACKEND_NULL")
endif()
if (DAWN_ENABLE_DESKTOP_GL)
    target_compile_definitions(dawn_internal_config INTERFACE "DAWN_ENABLE_BACKEND_DESKTOP_GL")
endif()
if (DAWN_ENABLE_OPENGLES)
    target_compile_definitions(dawn_internal_config INTERFACE "DAWN_ENABLE_BACKEND_OPENGLES")
endif()
if (DAWN_ENABLE_OPENGL)
    target_compile_definitions(dawn_internal_config INTERFACE "DAWN_ENABLE_BACKEND_OPENGL")
endif()
if (DAWN_ENABLE_VULKAN)
    target_compile_definitions(dawn_internal_config INTERFACE "DAWN_ENABLE_BACKEND_VULKAN")
endif()
if (DAWN_USE_WAYLAND)
    target_compile_definitions(dawn_internal_config INTERFACE "DAWN_USE_WAYLAND")
endif()
if (DAWN_USE_X11)
    target_compile_definitions(dawn_internal_config INTERFACE "DAWN_USE_X11")
endif()
if (WIN32)
    target_compile_definitions(dawn_internal_config INTERFACE "NOMINMAX" "WIN32_LEAN_AND_MEAN")
endif()

set(CMAKE_CXX_STANDARD "17")

################################################################################
# Tint
################################################################################

set(TINT_LIB_FUZZING_ENGINE_LINK_OPTIONS "" CACHE STRING "Used by OSS-Fuzz to control, via link options, which fuzzing engine should be used")

set(TINT_ROOT_SOURCE_DIR ${PROJECT_SOURCE_DIR})

# CMake < 3.15 sets /W3 in CMAKE_CXX_FLAGS. Remove it if it's there.
# See https://gitlab.kitware.com/cmake/cmake/-/issues/18317
if (MSVC)
  if (CMAKE_CXX_FLAGS MATCHES "/W3")
    string(REPLACE "/W3" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  endif()
endif()

if (${TINT_CHECK_CHROMIUM_STYLE})
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xclang -add-plugin -Xclang find-bad-constructs")
endif()

if (${TINT_BUILD_SPV_READER})
  include_directories("${DAWN_THIRD_PARTY_DIR}/vulkan-deps/spirv-tools/include")
endif()

if((CMAKE_CXX_COMPILER_ID STREQUAL "Clang") AND (CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC"))
  set(COMPILER_IS_CLANG_CL TRUE)
endif()

if((CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang") OR
    ((CMAKE_CXX_COMPILER_ID STREQUAL "Clang") AND
     (NOT COMPILER_IS_CLANG_CL)))
  set(COMPILER_IS_CLANG TRUE)
endif()

if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR COMPILER_IS_CLANG)
  set(COMPILER_IS_LIKE_GNU TRUE)
endif()

# Enable msbuild multiprocessor builds
if (MSVC AND NOT COMPILER_IS_CLANG_CL)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

set(TINT_OS_CC_SUFFIX "other")
if (NOT TINT_BUILD_AS_OTHER_OS)
  if(UNIX OR APPLE)
    set(TINT_OS_CC_SUFFIX "posix")
    set(TINT_OS_CC_SUFFIX "posix")
  elseif(WIN32)
    set(TINT_OS_CC_SUFFIX "windows")
    set(TINT_OS_CC_SUFFIX "windows")
  endif()
endif()

if(${TINT_BUILD_DOCS})
  find_package(Doxygen)
  if(DOXYGEN_FOUND)
    set(DOXYGEN_WARN_AS_ERROR NO)
    if(TINT_DOCS_WARN_AS_ERROR)
      set(DOXYGEN_WARN_AS_ERROR YES)
    endif()

    set(DOXYGEN_WARN_FORMAT "$file:$line: $text")
    if (MSVC)
      set(DOXYGEN_WARN_FORMAT "$file($line): $text")
    endif()

    add_custom_target(tint-docs ALL
        COMMAND ${CMAKE_COMMAND}
          -E env
          "DOXYGEN_OUTPUT_DIRECTORY=${CMAKE_BINARY_DIR}/docs"
          "DOXYGEN_WARN_AS_ERROR=${DOXYGEN_WARN_AS_ERROR}"
          "DOXYGEN_WARN_FORMAT=${DOXYGEN_WARN_FORMAT}"
          ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation"
        VERBATIM)
  else()
    message("Doxygen not found. Skipping documentation")
  endif(DOXYGEN_FOUND)
endif()

function(tint_core_compile_options TARGET)
  target_include_directories(${TARGET} PUBLIC "${TINT_ROOT_SOURCE_DIR}")
  target_include_directories(${TARGET} PUBLIC "${TINT_ROOT_SOURCE_DIR}/include")

  if (${TINT_BUILD_SPV_READER} OR ${TINT_BUILD_SPV_WRITER})
    target_include_directories(${TARGET} PUBLIC
        "${DAWN_THIRD_PARTY_DIR}/spirv-headers/include")
  endif()

  target_compile_definitions(${TARGET} PUBLIC -DTINT_BUILD_SPV_READER=$<BOOL:${TINT_BUILD_SPV_READER}>)
  target_compile_definitions(${TARGET} PUBLIC -DTINT_BUILD_WGSL_READER=$<BOOL:${TINT_BUILD_WGSL_READER}>)
  target_compile_definitions(${TARGET} PUBLIC -DTINT_BUILD_GLSL_WRITER=$<BOOL:${TINT_BUILD_GLSL_WRITER}>)
  target_compile_definitions(${TARGET} PUBLIC -DTINT_BUILD_HLSL_WRITER=$<BOOL:${TINT_BUILD_HLSL_WRITER}>)
  target_compile_definitions(${TARGET} PUBLIC -DTINT_BUILD_MSL_WRITER=$<BOOL:${TINT_BUILD_MSL_WRITER}>)
  target_compile_definitions(${TARGET} PUBLIC -DTINT_BUILD_SPV_WRITER=$<BOOL:${TINT_BUILD_SPV_WRITER}>)
  target_compile_definitions(${TARGET} PUBLIC -DTINT_BUILD_WGSL_WRITER=$<BOOL:${TINT_BUILD_WGSL_WRITER}>)
  target_compile_definitions(${TARGET} PUBLIC -DTINT_BUILD_IR=$<BOOL:${TINT_BUILD_IR}>)

  common_compile_options(${TARGET})
endfunction()

function(tint_default_compile_options TARGET)
  tint_core_compile_options(${TARGET})

  set(COMMON_GNU_OPTIONS
    -Wall
    -Werror
    -Wextra
    -Wno-documentation-unknown-command
    -Wno-padded
    -Wno-switch-enum
    -Wno-unknown-pragmas
  )

  set(COMMON_CLANG_OPTIONS
    -Wno-c++98-compat
    -Wno-c++98-compat-pedantic
    -Wno-format-pedantic
    -Wno-poison-system-directories
    -Wno-return-std-move-in-c++11
    -Wno-unknown-warning-option
    -Wno-undefined-var-template
    -Wno-unsafe-buffer-usage
    -Wno-used-but-marked-unused
    -Weverything
  )

  if (COMPILER_IS_LIKE_GNU)
    target_compile_options(${TARGET} PRIVATE
      -pedantic-errors
      ${COMMON_GNU_OPTIONS}
    )

    if (COMPILER_IS_CLANG)
      target_compile_options(${TARGET} PRIVATE
        ${COMMON_CLANG_OPTIONS}
      )
    endif()
  endif(COMPILER_IS_LIKE_GNU)

  if (MSVC)
    # Specify /EHs for exception handling.
    target_compile_options(${TARGET} PRIVATE
      /bigobj
      /EHsc
      /W4
      /WX
      /wd4068
      /wd4127
      /wd4244
      /wd4267
      /wd4324
      /wd4458
      /wd4514
      /wd4571
      /wd4625
      /wd4626
      /wd4710
      /wd4774
      /wd4820
      /wd5026
      /wd5027
    )

    # When building with clang-cl on Windows, try to match our clang build
    # options as much as possible.
    if (COMPILER_IS_CLANG_CL)
      target_compile_options(${TARGET} PRIVATE
        ${COMMON_GNU_OPTIONS}
        ${COMMON_CLANG_OPTIONS}
        # Disable warnings that are usually disabled in downstream deps for
        # gcc/clang, but aren't for clang-cl.
        -Wno-global-constructors
        -Wno-zero-as-null-pointer-constant
        -Wno-shorten-64-to-32
        -Wno-shadow-field-in-constructor
        -Wno-reserved-id-macro
        -Wno-language-extension-token
      )
    endif()
  endif()

  if (TINT_RANDOMIZE_HASHES)
    if(NOT DEFINED TINT_HASH_SEED)
        string(RANDOM LENGTH 16 ALPHABET "0123456789abcdef" seed)
        set(TINT_HASH_SEED "0x${seed}" CACHE STRING "Tint hash seed value")
        message("Using TINT_HASH_SEED: ${TINT_HASH_SEED}")
    endif()
    target_compile_definitions(${TARGET} PUBLIC "-DTINT_HASH_SEED=${TINT_HASH_SEED}")
  endif()
endfunction()

################################################################################
# Run on all subdirectories
################################################################################

add_subdirectory(third_party)

# TODO(crbug.com/tint/455): Tint does not currently build with CMake when
# BUILD_SHARED_LIBS=1, so always build it as static for now.
set(BUILD_SHARED_LIBS_SAVED ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS 0)
add_subdirectory(src/tint)
set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_SAVED})

add_subdirectory(generator)
add_subdirectory(src/dawn)

################################################################################
# Samples
################################################################################

if (TINT_BUILD_SAMPLES)
  add_subdirectory(src/tint/cmd)
endif()

if (TINT_BUILD_FUZZERS)
  add_subdirectory(src/tint/fuzzers)
endif()

add_custom_target(tint-lint
  COMMAND ./tools/lint
  WORKING_DIRECTORY ${TINT_ROOT_SOURCE_DIR}
  COMMENT "Running linter"
  VERBATIM)

add_custom_target(tint-format
  COMMAND ./tools/format
  WORKING_DIRECTORY ${TINT_ROOT_SOURCE_DIR}
  COMMENT "Running formatter"
  VERBATIM)

if (DAWN_EMIT_COVERAGE)
  add_subdirectory(tools/src/cmd/turbo-cov)

  # The tint-generate-coverage target generates a lcov.info file at the project
  # root, holding the code coverage for all the tint_unitests.
  # This can be used by tools such as VSCode's Coverage Gutters extension to
  # visualize code coverage in the editor.
  get_filename_component(CLANG_BIN_DIR ${CMAKE_C_COMPILER} DIRECTORY)
  set(PATH_WITH_CLANG "${CLANG_BIN_DIR}:$ENV{PATH}")
  add_custom_target(tint-generate-coverage
    COMMAND ${CMAKE_COMMAND} -E env PATH=${PATH_WITH_CLANG} ./tools/tint-generate-coverage $<TARGET_FILE:tint_unittests>
    DEPENDS tint_unittests
    WORKING_DIRECTORY ${TINT_ROOT_SOURCE_DIR}
    COMMENT "Generating tint coverage data"
    VERBATIM)
endif()

if (TINT_BUILD_REMOTE_COMPILE)
  add_subdirectory(tools/src/cmd/remote-compile)
endif()
