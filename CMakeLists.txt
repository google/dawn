# Copyright 2020 The Tint Authors.
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

project(tint)
enable_testing()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_DEBUG_POSTFIX "")

if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
  message(STATUS "No build type selected, default to Debug")
  set(CMAKE_BUILD_TYPE "Debug")
endif()

# TINT_IS_SUBPROJECT is 1 if added via add_subdirectory() from another project.
get_directory_property(TINT_IS_SUBPROJECT PARENT_DIRECTORY)
if(TINT_IS_SUBPROJECT)
  set(TINT_IS_SUBPROJECT 1)

  # If tint is used as a subproject, default to disabling the building of
  # documentation and tests. These are unlikely to be desirable, but can be
  # enabled.
  set(TINT_BUILD_DOCS_DEFAULT OFF)
  set(TINT_BUILD_TESTS_DEFAULT OFF)
else()
  set(TINT_BUILD_DOCS_DEFAULT ON)
  set(TINT_BUILD_TESTS_DEFAULT ON)
endif()

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

set_if_not_defined(TINT_THIRD_PARTY_DIR "${tint_SOURCE_DIR}/third_party" "Directory in which to find third-party dependencies.")

option_if_not_defined(TINT_BUILD_SAMPLES "Build samples" ON)
option_if_not_defined(TINT_BUILD_DOCS "Build documentation" ${TINT_BUILD_DOCS_DEFAULT})
option_if_not_defined(TINT_DOCS_WARN_AS_ERROR "When building documentation, treat warnings as errors" OFF)
option_if_not_defined(TINT_BUILD_SPV_READER "Build the SPIR-V input reader" ON)
option_if_not_defined(TINT_BUILD_WGSL_READER "Build the WGSL input reader" ON)
option_if_not_defined(TINT_BUILD_GLSL_WRITER "Build the GLSL output writer" ON)
option_if_not_defined(TINT_BUILD_HLSL_WRITER "Build the HLSL output writer" ON)
option_if_not_defined(TINT_BUILD_MSL_WRITER "Build the MSL output writer" ON)
option_if_not_defined(TINT_BUILD_SPV_WRITER "Build the SPIR-V output writer" ON)
option_if_not_defined(TINT_BUILD_WGSL_WRITER "Build the WGSL output writer" ON)
option_if_not_defined(TINT_BUILD_FUZZERS "Build fuzzers" OFF)
option_if_not_defined(TINT_BUILD_SPIRV_TOOLS_FUZZER "Build SPIRV-Tools fuzzer" OFF)
option_if_not_defined(TINT_BUILD_AST_FUZZER "Build AST fuzzer" OFF)
option_if_not_defined(TINT_BUILD_REGEX_FUZZER "Build regex fuzzer" OFF)
option_if_not_defined(TINT_BUILD_TESTS "Build tests" ${TINT_BUILD_TESTS_DEFAULT})
option_if_not_defined(TINT_BUILD_AS_OTHER_OS "Override OS detection to force building of *_other.cc files" OFF)
option_if_not_defined(TINT_BUILD_REMOTE_COMPILE "Build the remote-compile tool for validating shaders on a remote machine" OFF)

set(TINT_LIB_FUZZING_ENGINE_LINK_OPTIONS "" CACHE STRING "Used by OSS-Fuzz to control, via link options, which fuzzing engine should be used")

option_if_not_defined(TINT_ENABLE_MSAN "Enable memory sanitizer" OFF)
option_if_not_defined(TINT_ENABLE_ASAN "Enable address sanitizer" OFF)
option_if_not_defined(TINT_ENABLE_UBSAN "Enable undefined behaviour sanitizer" OFF)

option_if_not_defined(TINT_EMIT_COVERAGE "Emit code coverage information" OFF)

option_if_not_defined(TINT_CHECK_CHROMIUM_STYLE "Check for [chromium-style] issues during build" OFF)

option_if_not_defined(TINT_SYMBOL_STORE_DEBUG_NAME "Enable storing of name in tint::ast::Symbol to help debugging the AST" OFF)

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
message(STATUS "Tint build fuzzers: ${TINT_BUILD_FUZZERS}")
message(STATUS "Tint build SPIRV-Tools fuzzer: ${TINT_BUILD_SPIRV_TOOLS_FUZZER}")
message(STATUS "Tint build AST fuzzer: ${TINT_BUILD_AST_FUZZER}")
message(STATUS "Tint build regex fuzzer: ${TINT_BUILD_REGEX_FUZZER}")
message(STATUS "Tint build tests: ${TINT_BUILD_TESTS}")
message(STATUS "Tint build with ASAN: ${TINT_ENABLE_ASAN}")
message(STATUS "Tint build with MSAN: ${TINT_ENABLE_MSAN}")
message(STATUS "Tint build with UBSAN: ${TINT_ENABLE_UBSAN}")
message(STATUS "Tint build checking [chromium-style]: ${TINT_CHECK_CHROMIUM_STYLE}")
message(STATUS "Tint build remote-compile tool: ${TINT_BUILD_REMOTE_COMPILE}")

if (NOT ${TINT_LIB_FUZZING_ENGINE_LINK_OPTIONS} STREQUAL "")
  message(STATUS "Using provided LIB_FUZZING_ENGINE options: ${TINT_LIB_FUZZING_ENGINE_LINK_OPTIONS}")
endif()

message(STATUS "Using python3")
find_package(PythonInterp 3 REQUIRED)

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
  include_directories("${TINT_ROOT_SOURCE_DIR}/third_party/spirv-tools/include")
endif()

if((CMAKE_CXX_COMPILER_ID STREQUAL "Clang") AND (CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC"))
  set(COMPILER_IS_CLANG_CL TRUE)
endif()

if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR
    (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang") OR
    ((CMAKE_CXX_COMPILER_ID STREQUAL "Clang") AND
     (NOT COMPILER_IS_CLANG_CL)))
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

function(tint_default_compile_options TARGET)
  target_include_directories(${TARGET} PUBLIC "${TINT_ROOT_SOURCE_DIR}")
  target_include_directories(${TARGET} PUBLIC "${TINT_ROOT_SOURCE_DIR}/include")

  if (${TINT_BUILD_SPV_READER} OR ${TINT_BUILD_SPV_WRITER})
    target_include_directories(${TARGET} PUBLIC
        "${TINT_ROOT_SOURCE_DIR}/third_party/spirv-headers/include")
  endif()

  target_compile_definitions(${TARGET} PUBLIC
      -DTINT_BUILD_SPV_READER=$<BOOL:${TINT_BUILD_SPV_READER}>)
  target_compile_definitions(${TARGET} PUBLIC
      -DTINT_BUILD_WGSL_READER=$<BOOL:${TINT_BUILD_WGSL_READER}>)
  target_compile_definitions(${TARGET} PUBLIC
      -DTINT_BUILD_GLSL_WRITER=$<BOOL:${TINT_BUILD_GLSL_WRITER}>)
  target_compile_definitions(${TARGET} PUBLIC
      -DTINT_BUILD_HLSL_WRITER=$<BOOL:${TINT_BUILD_HLSL_WRITER}>)
  target_compile_definitions(${TARGET} PUBLIC
      -DTINT_BUILD_MSL_WRITER=$<BOOL:${TINT_BUILD_MSL_WRITER}>)
  target_compile_definitions(${TARGET} PUBLIC
      -DTINT_BUILD_SPV_WRITER=$<BOOL:${TINT_BUILD_SPV_WRITER}>)
  target_compile_definitions(${TARGET} PUBLIC
      -DTINT_BUILD_WGSL_WRITER=$<BOOL:${TINT_BUILD_WGSL_WRITER}>)

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
    -Wno-return-std-move-in-c++11
    -Wno-unknown-warning-option
    -Wno-undefined-var-template
    -Wno-used-but-marked-unused
    -Weverything
  )

  if (${COMPILER_IS_LIKE_GNU})
    target_compile_options(${TARGET} PRIVATE
      -std=c++14
      -fno-exceptions
      -fno-rtti
      -pedantic-errors
      ${COMMON_GNU_OPTIONS}
    )

    if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR
        ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang"))
      target_compile_options(${TARGET} PRIVATE
        ${COMMON_CLANG_OPTIONS}
      )
    endif()

    if (${TINT_ENABLE_MSAN})
      target_compile_options(${TARGET} PRIVATE -fsanitize=memory)
      target_link_options(${TARGET} PRIVATE -fsanitize=memory)
    elseif (${TINT_ENABLE_ASAN})
      target_compile_options(${TARGET} PRIVATE -fsanitize=address)
      target_link_options(${TARGET} PRIVATE -fsanitize=address)
    elseif (${TINT_ENABLE_UBSAN})
      target_compile_options(${TARGET} PRIVATE -fsanitize=undefined)
      target_link_options(${TARGET} PRIVATE -fsanitize=undefined)
    endif()
  endif()

  if (${TINT_EMIT_COVERAGE})
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_compile_options(${TARGET} PRIVATE "--coverage")
        target_link_options(${TARGET} PRIVATE "gcov")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(${TARGET} PRIVATE "-fprofile-instr-generate" "-fcoverage-mapping")
        target_link_options(${TARGET} PRIVATE "-fprofile-instr-generate" "-fcoverage-mapping")
    else()
        message(FATAL_ERROR "Coverage generation not supported for the ${CMAKE_CXX_COMPILER_ID} toolchain")
    endif()
  endif()

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
      )
    endif()
  endif()

endfunction()

add_subdirectory(third_party)
add_subdirectory(src)
if (TINT_BUILD_SAMPLES)
  add_subdirectory(samples)
endif()

if (TINT_BUILD_FUZZERS)
  add_subdirectory(fuzzers)
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


if (TINT_EMIT_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # Generates a lcov.info file at the project root.
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
