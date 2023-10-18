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

function (install_if_enabled target)
    if(NOT DAWN_ENABLE_INSTALL)
        return()
    endif()

    install(TARGETS ${target}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    get_target_property(targetHeaders ${target} INTERFACE_SOURCES)
    if (NOT targetHeaders)
        return()
    endif()

    foreach(headerFile ${targetHeaders})
        # Starting from CMake 3.20 there is the cmake_path command that could simplify this code.
        # Compute the install subdirectory for the header by stripping out the path to
        # the include / gen/include directory...
        string(FIND "${headerFile}" "${DAWN_INCLUDE_DIR}" foundIndex)
        if (foundIndex EQUAL 0)
            string(LENGTH "${DAWN_INCLUDE_DIR}/" lengthToRemove)
        endif()
        string(FIND "${headerFile}" "${DAWN_BUILD_GEN_DIR}/include/" foundIndex)
        if (foundIndex EQUAL 0)
            string(LENGTH "${DAWN_BUILD_GEN_DIR}/include/" lengthToRemove)
        endif()
        string(SUBSTRING "${headerFile}" "${lengthToRemove}" -1 headerRelDir)

        # ... then remove everything after the last /
        string(FIND "${headerRelDir}" "/" foundIndex REVERSE)
        string(SUBSTRING "${headerRelDir}" 0 ${foundIndex} headerRelDir)

        install(FILES "${headerFile}" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${headerRelDir})
    endforeach()
endfunction()


# Default values for the backend-enabling options
set(ENABLE_D3D11 OFF)
set(ENABLE_D3D12 OFF)
set(ENABLE_METAL OFF)
set(ENABLE_OPENGLES OFF)
set(ENABLE_DESKTOP_GL OFF)
set(ENABLE_VULKAN OFF)
set(USE_WAYLAND OFF)
set(USE_X11 OFF)
set(USE_WINDOWS_UI OFF)
set(BUILD_SAMPLES OFF)
set(TARGET_MACOS OFF)
if (WIN32)
    set(ENABLE_D3D11 ON)
    set(USE_WINDOWS_UI ON)
    set(ENABLE_D3D12 ON)
    if (NOT WINDOWS_STORE)
        # Enable Vulkan in win32 compilation only
        # since UWP only supports d3d
        set(ENABLE_VULKAN ON)
    endif()
elseif(APPLE)
    set(ENABLE_METAL ON)
    if(CMAKE_SYSTEM_NAME MATCHES "Darwin" OR CMAKE_SYSTEM_NAME MATCHES "MacOS")
      set(TARGET_MACOS ON)
    endif()
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
set(DAWN_SUPPORTS_GLFW_FOR_WINDOWING OFF)

if ((WIN32 AND NOT WINDOWS_STORE) OR (UNIX AND NOT ANDROID))
    set(DAWN_SUPPORTS_GLFW_FOR_WINDOWING ON)
endif()

# Current examples are depend on GLFW
if (DAWN_SUPPORTS_GLFW_FOR_WINDOWING)
    set(BUILD_SAMPLES ON)
endif()

option_if_not_defined(DAWN_ENABLE_ASAN "Enable address sanitizer" OFF)
option_if_not_defined(DAWN_ENABLE_INSTALL "Enable install step for Dawn libraries" OFF)
option_if_not_defined(DAWN_ENABLE_TSAN "Enable thread sanitizer" OFF)
option_if_not_defined(DAWN_ENABLE_MSAN "Enable memory sanitizer" OFF)
option_if_not_defined(DAWN_ENABLE_UBSAN "Enable undefined behaviour sanitizer" OFF)

option_if_not_defined(DAWN_ENABLE_D3D11 "Enable compilation of the D3D11 backend" ${ENABLE_D3D11})
option_if_not_defined(DAWN_ENABLE_D3D12 "Enable compilation of the D3D12 backend" ${ENABLE_D3D12})
option_if_not_defined(DAWN_ENABLE_METAL "Enable compilation of the Metal backend" ${ENABLE_METAL})
option_if_not_defined(DAWN_ENABLE_NULL "Enable compilation of the Null backend" ON)
option_if_not_defined(DAWN_ENABLE_DESKTOP_GL "Enable compilation of the OpenGL backend" ${ENABLE_DESKTOP_GL})
option_if_not_defined(DAWN_ENABLE_OPENGLES "Enable compilation of the OpenGL ES backend" ${ENABLE_OPENGLES})
option_if_not_defined(DAWN_ENABLE_VULKAN "Enable compilation of the Vulkan backend" ${ENABLE_VULKAN})

option_if_not_defined(DAWN_ALWAYS_ASSERT "Enable assertions on all build types" OFF)
option_if_not_defined(DAWN_USE_WAYLAND "Enable support for Wayland surface" ${USE_WAYLAND})
option_if_not_defined(DAWN_USE_X11 "Enable support for X11 surface" ${USE_X11})
option_if_not_defined(DAWN_USE_GLFW "Enable compilation of the GLFW windowing utils" ${DAWN_SUPPORTS_GLFW_FOR_WINDOWING})
option_if_not_defined(DAWN_USE_WINDOWS_UI "Enable support for Windows UI surface" ${USE_WINDOWS_UI})
option_if_not_defined(DAWN_TARGET_MACOS "Manually link Apple core frameworks" ${TARGET_MACOS})

option_if_not_defined(DAWN_BUILD_SAMPLES "Enables building Dawn's samples" ${BUILD_SAMPLES})
option_if_not_defined(DAWN_BUILD_NODE_BINDINGS "Enables building Dawn's NodeJS bindings" OFF)
option_if_not_defined(DAWN_ENABLE_SWIFTSHADER "Enables building Swiftshader as part of the build and Vulkan adapter discovery" OFF)
option_if_not_defined(DAWN_BUILD_BENCHMARKS "Build Dawn benchmarks" OFF)

option_if_not_defined(DAWN_ENABLE_PIC "Build with Position-Independent-Code enabled" OFF)

option_if_not_defined(DAWN_EMIT_COVERAGE "Emit code coverage information" OFF)
set_if_not_defined(LLVM_SOURCE_DIR "${Dawn_LLVM_SOURCE_DIR}" "Directory to an LLVM source checkout. Required to build turbo-cov")

if (DAWN_ENABLE_OPENGLES OR DAWN_ENABLE_DESKTOP_GL)
  set(TINT_DEFAULT_GLSL ON)
else()
  set(TINT_DEFAULT_GLSL OFF)
endif()

option_if_not_defined(TINT_ENABLE_INSTALL "Enable install step for Tint libraries" OFF)
option_if_not_defined(TINT_BUILD_CMD_TOOLS "Build the Tint command line tools" ON)
option_if_not_defined(TINT_BUILD_DOCS "Build documentation" ON)
option_if_not_defined(TINT_DOCS_WARN_AS_ERROR "When building documentation, treat warnings as errors" OFF)

if (NOT DAWN_USE_GLFW AND (TINT_BUILD_CMD_TOOLS OR DAWN_BUILD_SAMPLES))
  message(SEND_ERROR "Dawn samples require GLFW")
endif()

option_if_not_defined(TINT_BUILD_SPV_READER "Build the SPIR-V input reader" ${DAWN_ENABLE_VULKAN})
option_if_not_defined(TINT_BUILD_WGSL_READER "Build the WGSL input reader" ON)
option_if_not_defined(TINT_BUILD_GLSL_WRITER "Build the GLSL output writer" ${TINT_DEFAULT_GLSL})
option_if_not_defined(TINT_BUILD_GLSL_VALIDATOR "Build the GLSL output validator" ON)
option_if_not_defined(TINT_BUILD_HLSL_WRITER "Build the HLSL output writer" ${DAWN_ENABLE_D3D12})
option_if_not_defined(TINT_BUILD_MSL_WRITER "Build the MSL output writer" ${DAWN_ENABLE_METAL})
option_if_not_defined(TINT_BUILD_SPV_WRITER "Build the SPIR-V output writer" ${DAWN_ENABLE_VULKAN})
option_if_not_defined(TINT_BUILD_WGSL_WRITER "Build the WGSL output writer" ON)

option_if_not_defined(TINT_BUILD_SYNTAX_TREE_WRITER "Build the syntax tree writer" OFF)

option_if_not_defined(TINT_BUILD_FUZZERS "Build fuzzers" OFF)
option_if_not_defined(TINT_BUILD_AST_FUZZER "Build AST fuzzer" OFF)
option_if_not_defined(TINT_BUILD_REGEX_FUZZER "Build regex fuzzer" OFF)
option_if_not_defined(TINT_BUILD_BENCHMARKS "Build Tint benchmarks" OFF)
option_if_not_defined(TINT_BUILD_TESTS "Build tests" ON)
option_if_not_defined(TINT_BUILD_AS_OTHER_OS "Override OS detection to force building of *_other.cc files" OFF)

set_if_not_defined(TINT_EXTERNAL_BENCHMARK_CORPUS_DIR "" "Directory that holds a corpus of external shaders to benchmark.")

option_if_not_defined(TINT_ENABLE_BREAK_IN_DEBUGGER "Enable tint::debugger::Break()" OFF)
option_if_not_defined(TINT_CHECK_CHROMIUM_STYLE "Check for [chromium-style] issues during build" OFF)
option_if_not_defined(TINT_RANDOMIZE_HASHES "Randomize the hash seed value to detect non-deterministic output" OFF)

# Recommended setting for compability with future abseil releases.
set(ABSL_PROPAGATE_CXX_STD ON)

set_if_not_defined(DAWN_THIRD_PARTY_DIR "${Dawn_SOURCE_DIR}/third_party" "Directory in which to find third-party dependencies.")
set_if_not_defined(DAWN_VULKAN_DEPS_DIR "${DAWN_THIRD_PARTY_DIR}/vulkan-deps" "Directory in which to find vulkan-deps")

set_if_not_defined(DAWN_ABSEIL_DIR "${DAWN_THIRD_PARTY_DIR}/abseil-cpp" "Directory in which to find Abseil")
set_if_not_defined(DAWN_GLFW_DIR "${DAWN_THIRD_PARTY_DIR}/glfw" "Directory in which to find GLFW")
set_if_not_defined(DAWN_JINJA2_DIR "${DAWN_THIRD_PARTY_DIR}/jinja2" "Directory in which to find Jinja2")
set_if_not_defined(DAWN_MARKUPSAFE_DIR "${DAWN_THIRD_PARTY_DIR}/markupsafe" "Directory in which to find MarkupSafe")
set_if_not_defined(DAWN_KHRONOS_DIR "${DAWN_THIRD_PARTY_DIR}/khronos" "Directory in which to find Khronos GL headers")
set_if_not_defined(DAWN_SWIFTSHADER_DIR "${DAWN_THIRD_PARTY_DIR}/swiftshader" "Directory in which to find swiftshader")

set_if_not_defined(DAWN_SPIRV_TOOLS_DIR "${DAWN_VULKAN_DEPS_DIR}/spirv-tools/src" "Directory in which to find SPIRV-Tools")
set_if_not_defined(DAWN_SPIRV_HEADERS_DIR "${DAWN_VULKAN_DEPS_DIR}/spirv-headers/src" "Directory in which to find SPIRV-Headers")
set_if_not_defined(DAWN_VULKAN_HEADERS_DIR "${DAWN_VULKAN_DEPS_DIR}/vulkan-headers/src" "Directory in which to find Vulkan-Headers")
set_if_not_defined(DAWN_VULKAN_UTILITY_LIBRARIES_DIR "${DAWN_VULKAN_DEPS_DIR}/vulkan-utility-libraries/src" "Directory in which to find Vulkan-Utility-Libraries")

# Dependencies for DAWN_BUILD_NODE_BINDINGS
set_if_not_defined(NODE_ADDON_API_DIR "${DAWN_THIRD_PARTY_DIR}/node-addon-api" "Directory in which to find node-addon-api")
set_if_not_defined(NODE_API_HEADERS_DIR "${DAWN_THIRD_PARTY_DIR}/node-api-headers" "Directory in which to find node-api-headers")
set_if_not_defined(WEBGPU_IDL_PATH "${DAWN_THIRD_PARTY_DIR}/gpuweb/webgpu.idl" "Path to the webgpu.idl definition file")
set_if_not_defined(GO_EXECUTABLE "go" "Golang executable for running the IDL generator")

option_if_not_defined(DAWN_FETCH_DEPENDENCIES "Use fetch_dawn_dependencies.py as an alternative to using depot_tools" OFF)

# Much of the backend code is shared among desktop OpenGL and OpenGL ES
if (${DAWN_ENABLE_DESKTOP_GL} OR ${DAWN_ENABLE_OPENGLES})
    set(DAWN_ENABLE_OPENGL ON)
endif()

if(DAWN_ENABLE_PIC)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
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

message(STATUS "Dawn build D3D11 backend: ${DAWN_ENABLE_D3D11}")
message(STATUS "Dawn build D3D12 backend: ${DAWN_ENABLE_D3D12}")
message(STATUS "Dawn build Metal backend: ${DAWN_ENABLE_METAL}")
message(STATUS "Dawn build Vulkan backend: ${DAWN_ENABLE_VULKAN}")
message(STATUS "Dawn build OpenGL backend: ${DAWN_ENABLE_DESKTOP_GL}")
message(STATUS "Dawn build OpenGL ES backend: ${DAWN_ENABLE_OPENGLES}")
message(STATUS "Dawn build Null backend: ${DAWN_ENABLE_NULL}")

message(STATUS "Dawn build with asserts in all configurations: ${DAWN_ALWAYS_ASSERT}")
message(STATUS "Dawn build Wayland support: ${DAWN_USE_WAYLAND}")
message(STATUS "Dawn build X11 support: ${DAWN_USE_X11}")
message(STATUS "Dawn build GLFW support: ${DAWN_USE_GLFW}")
message(STATUS "Dawn build Windows UI support: ${DAWN_USE_WINDOWS_UI}")

message(STATUS "Dawn build samples: ${DAWN_BUILD_SAMPLES}")
message(STATUS "Dawn build Node bindings: ${DAWN_BUILD_NODE_BINDINGS}")
message(STATUS "Dawn build Swiftshader: ${DAWN_ENABLE_SWIFTSHADER}")
message(STATUS "Dawn build benchmarks: ${DAWN_BUILD_BENCHMARKS}")

message(STATUS "Dawn build PIC: ${DAWN_ENABLE_PIC}")

message(STATUS "Dawn build with ASAN: ${DAWN_ENABLE_ASAN}")
message(STATUS "Dawn build with TSAN: ${DAWN_ENABLE_TSAN}")
message(STATUS "Dawn build with MSAN: ${DAWN_ENABLE_MSAN}")
message(STATUS "Dawn build with UBSAN: ${DAWN_ENABLE_UBSAN}")

message(STATUS "Tint build command line executable tools: ${TINT_BUILD_CMD_TOOLS}")
message(STATUS "Tint build docs: ${TINT_BUILD_DOCS}")
message(STATUS "Tint build docs with warn as error: ${TINT_DOCS_WARN_AS_ERROR}")
message(STATUS "Tint build SPIR-V reader: ${TINT_BUILD_SPV_READER}")
message(STATUS "Tint build WGSL reader: ${TINT_BUILD_WGSL_READER}")
message(STATUS "Tint build GLSL writer: ${TINT_BUILD_GLSL_WRITER}")
message(STATUS "Tint build GLSL validator: ${TINT_BUILD_GLSL_VALIDATOR}")
message(STATUS "Tint build HLSL writer: ${TINT_BUILD_HLSL_WRITER}")
message(STATUS "Tint build MSL writer: ${TINT_BUILD_MSL_WRITER}")
message(STATUS "Tint build SPIR-V writer: ${TINT_BUILD_SPV_WRITER}")
message(STATUS "Tint build WGSL writer: ${TINT_BUILD_WGSL_WRITER}")
message(STATUS "Tint build Syntax Tree writer: ${TINT_BUILD_SYNTAX_TREE_WRITER}")
message(STATUS "Tint build fuzzers: ${TINT_BUILD_FUZZERS}")
message(STATUS "Tint build AST fuzzer: ${TINT_BUILD_AST_FUZZER}")
message(STATUS "Tint build regex fuzzer: ${TINT_BUILD_REGEX_FUZZER}")
message(STATUS "Tint build benchmarks: ${TINT_BUILD_BENCHMARKS}")
message(STATUS "Tint build tests: ${TINT_BUILD_TESTS}")
message(STATUS "Tint build checking [chromium-style]: ${TINT_CHECK_CHROMIUM_STYLE}")
message(STATUS "Tint external benchmark corpus dir: ${TINT_EXTERNAL_BENCHMARK_CORPUS_DIR}")


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
      target_compile_options(${TARGET} PUBLIC -fsanitize=undefined -fsanitize=float-divide-by-zero)
      target_link_options(${TARGET} PUBLIC -fsanitize=undefined -fsanitize=float-divide-by-zero)
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
if (DAWN_ENABLE_D3D11)
    target_compile_definitions(dawn_internal_config INTERFACE "DAWN_ENABLE_BACKEND_D3D11")
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
if (DAWN_USE_WINDOWS_UI)
    target_compile_definitions(dawn_internal_config INTERFACE "DAWN_USE_WINDOWS_UI")
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
set(TINT_SPIRV_HEADERS_DIR ${DAWN_SPIRV_HEADERS_DIR})
set(TINT_SPIRV_TOOLS_DIR   ${DAWN_SPIRV_TOOLS_DIR})

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

if((CMAKE_CXX_COMPILER_ID STREQUAL "Clang") AND (CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC"))
  set(COMPILER_IS_CLANG_CL TRUE)
endif()

if((CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang") OR
    ((CMAKE_CXX_COMPILER_ID STREQUAL "Clang") AND
     (NOT COMPILER_IS_CLANG_CL)))
  set(COMPILER_IS_CLANG TRUE)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(COMPILER_IS_GNU TRUE)
endif()

if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR COMPILER_IS_CLANG)
  set(COMPILER_IS_LIKE_GNU TRUE)
endif()

# Enable msbuild multiprocessor builds
if (MSVC AND NOT COMPILER_IS_CLANG_CL)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
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

