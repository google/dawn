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

option(TINT_BUILD_DOCS "Build documentation" ON)
option(TINT_BUILD_SPV_READER "Build the SPIR-V input reader" ON)
option(TINT_BUILD_WGSL_READER "Builde the WGSL input reader" ON)
option(TINT_BUILD_HLSL_WRITER "Build the HLSL output writer" ON)
option(TINT_BUILD_MSL_WRITER "Build the MSL output writer" ON)
option(TINT_BUILD_SPV_WRITER "Build the SPIR-V output writer" ON)
option(TINT_BUILD_WGSL_WRITER "Build the WGSL output writer" ON)
option(TINT_BUILD_FUZZERS "Build fuzzers" OFF)

option(TINT_ENABLE_MSAN "Enable memory sanitizer" OFF)
option(TINT_ENABLE_ASAN "Enable address sanitizer" OFF)
option(TINT_ENABLE_UBSAN "Enable undefined behaviour sanitizer" OFF)

option(TINT_CHECK_CHROMIUM_STYLE "Check for [chromium-style] issues during build" OFF)

if(WIN32)
  # On Windows, CMake by default compiles with the shared CRT.
  # Default it to the static CRT.
  option(TINT_ENABLE_SHARED_CRT
         "Tint: Use the shared CRT with MSVC instead of the static CRT"
         ${TINT_ENABLE_SHARED_CRT})
endif(WIN32)

message(STATUS "Tint build docs: ${TINT_BUILD_DOCS}")
message(STATUS "Tint build SPIR-V reader: ${TINT_BUILD_SPV_READER}")
message(STATUS "Tint build WGSL reader: ${TINT_BUILD_WGSL_READER}")
message(STATUS "Tint build HLSL writer: ${TINT_BUILD_HLSL_WRITER}")
message(STATUS "Tint build MSL writer: ${TINT_BUILD_MSL_WRITER}")
message(STATUS "Tint build SPIR-V writer: ${TINT_BUILD_SPV_WRITER}")
message(STATUS "Tint build WGSL writer: ${TINT_BUILD_WGSL_WRITER}")
message(STATUS "Tint build fuzzers: ${TINT_BUILD_FUZZERS}")
message(STATUS "Tint build with ASAN: ${TINT_ENABLE_ASAN}")
message(STATUS "Tint build with MSAN: ${TINT_ENABLE_MSAN}")
message(STATUS "Tint build with UBSAN: ${TINT_ENABLE_UBSAN}")
message(STATUS "Tint build checking [chromium-style]: ${TINT_CHECK_CHROMIUM_STYLE}")

message(STATUS "Using python3")
find_package(PythonInterp 3 REQUIRED)

if (${TINT_CHECK_CHROMIUM_STYLE})
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xclang -add-plugin -Xclang find-bad-constructs")
endif()

if (${TINT_BUILD_SPV_READER})
  include_directories("${PROJECT_SOURCE_DIR}/third_party/spirv-tools/include")
endif()

include_directories("${PROJECT_SOURCE_DIR}/third_party/spirv-headers/include")
include_directories("${PROJECT_SOURCE_DIR}/include")

if(("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") OR
    ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang") OR
    (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") AND
     (NOT CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")))
  set(COMPILER_IS_LIKE_GNU TRUE)
endif()

if(${TINT_BUILD_DOCS})
  find_package(Doxygen)
  if(DOXYGEN_FOUND)
    add_custom_target(tint-docs ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation"
        VERBATIM)
  else()
    message("Doxygen not found. Skipping documentation")
  endif(DOXYGEN_FOUND)
endif()

if(MSVC)
  # We don't want to have to copy the C Runtime DLL everywhere the executable
  # goes.  So by default compile code to assume the CRT is statically linked,
  # i.e. use /MT* options.  For debug builds use /MTd, and for release builds
  # use /MT.  If TINT_ENABLE_SHARED_CRT is ON, then use the shared C runtime.
  # Modify the project-wide options variables. This is ugly, but seems to be
  # the state of the art.
  if(NOT ${TINT_ENABLE_SHARED_CRT})
    message(STATUS "Tint: Static C runtime selected: replacing /MD* with /MT*")
    foreach (flag_var
       CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
       CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
       CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
       CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
      string(REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endforeach()
  endif()
endif()

function(tint_default_compile_options TARGET)
  include_directories("${PROJECT_SOURCE_DIR}")

  target_compile_definitions(${TARGET} PRIVATE
      -DTINT_BUILD_SPV_READER=$<BOOL:${TINT_BUILD_SPV_READER}>)
  target_compile_definitions(${TARGET} PRIVATE
      -DTINT_BUILD_WGSL_READER=$<BOOL:${TINT_BUILD_WGSL_READER}>)
  target_compile_definitions(${TARGET} PRIVATE
      -DTINT_BUILD_HLSL_WRITER=$<BOOL:${TINT_BUILD_HLSL_WRITER}>)
  target_compile_definitions(${TARGET} PRIVATE
    -DTINT_BUILD_MSL_WRITER=$<BOOL:${TINT_BUILD_MSL_WRITER}>)
  target_compile_definitions(${TARGET} PRIVATE
      -DTINT_BUILD_SPV_WRITER=$<BOOL:${TINT_BUILD_SPV_WRITER}>)
  target_compile_definitions(${TARGET} PRIVATE
      -DTINT_BUILD_WGSL_WRITER=$<BOOL:${TINT_BUILD_WGSL_WRITER}>)

  if (${COMPILER_IS_LIKE_GNU})
    target_compile_options(${TARGET} PRIVATE
      -std=c++14
      -fno-exceptions
      -fno-rtti
      -fvisibility=hidden
      -Wall
      -Werror
      -Wextra
      -Wno-documentation-unknown-command
      -Wno-padded
      -Wno-switch-enum
      -Wno-unknown-pragmas
      -pedantic-errors
    )

    if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR
        ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang"))
      target_compile_options(${TARGET} PRIVATE
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
        -Wno-format-pedantic
        -Wno-return-std-move-in-c++11
        -Wno-unknown-warning-option
        -Weverything
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

  if (MSVC)
    # Specify /EHs for exception handling.
    target_compile_options(${TARGET} PRIVATE
      /bigobj
      /EHsc
      /W3
      /WX
      /wd4068
      /wd4244
      /wd4267
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
  endif()

  if (NOT ${TINT_ENABLE_SHARED_CRT})
    # For MinGW cross compile, statically link to the C++ runtime.
    # But it still depends on MSVCRT.dll.
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
      if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        set_target_properties(${TARGET} PROPERTIES LINK_FLAGS
          -static
          -static-libgcc
          -static-libstdc++)
      endif()
    endif()
  endif()
endfunction()

add_subdirectory(third_party)
add_subdirectory(src)
add_subdirectory(samples)

if (${TINT_BUILD_FUZZERS})
  add_subdirectory(fuzzers)
endif()

add_custom_target(tint-lint
  COMMAND ./tools/lint
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  COMMENT "Running linter"
  VERBATIM)

add_custom_target(tint-format
  COMMAND ./tools/format
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  COMMENT "Running formatter"
  VERBATIM)
