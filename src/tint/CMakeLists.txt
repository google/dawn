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

################################################################################
# Target flags
################################################################################
if(NOT TINT_BUILD_AS_OTHER_OS)
  if(UNIX)
    set(IS_LINUX TRUE)
    set(IS_LINUX TRUE)
  elseif(APPLE)
    set(IS_MAC TRUE)
    set(IS_MAC TRUE)
  elseif(WIN32)
    set(IS_WIN TRUE)
    set(IS_WIN TRUE)
  endif()
endif()

################################################################################
# Helper functions
################################################################################
function(tint_core_compile_options TARGET)
  target_include_directories(${TARGET} PUBLIC "${TINT_ROOT_SOURCE_DIR}")
  target_include_directories(${TARGET} PUBLIC "${TINT_ROOT_SOURCE_DIR}/include")
  target_compile_definitions(${TARGET} PUBLIC -DTINT_BUILD_SPV_READER=$<BOOL:${TINT_BUILD_SPV_READER}>)
  target_compile_definitions(${TARGET} PUBLIC -DTINT_BUILD_WGSL_READER=$<BOOL:${TINT_BUILD_WGSL_READER}>)
  target_compile_definitions(${TARGET} PUBLIC -DTINT_BUILD_GLSL_WRITER=$<BOOL:${TINT_BUILD_GLSL_WRITER}>)
  target_compile_definitions(${TARGET} PUBLIC -DTINT_BUILD_HLSL_WRITER=$<BOOL:${TINT_BUILD_HLSL_WRITER}>)
  target_compile_definitions(${TARGET} PUBLIC -DTINT_BUILD_MSL_WRITER=$<BOOL:${TINT_BUILD_MSL_WRITER}>)
  target_compile_definitions(${TARGET} PUBLIC -DTINT_BUILD_SPV_WRITER=$<BOOL:${TINT_BUILD_SPV_WRITER}>)
  target_compile_definitions(${TARGET} PUBLIC -DTINT_BUILD_WGSL_WRITER=$<BOOL:${TINT_BUILD_WGSL_WRITER}>)
  target_compile_definitions(${TARGET} PUBLIC -DTINT_BUILD_SYNTAX_TREE_WRITER=$<BOOL:${TINT_BUILD_SYNTAX_TREE_WRITER}>)
  target_compile_definitions(${TARGET} PUBLIC -DTINT_BUILD_IR=$<BOOL:${TINT_BUILD_IR}>)

  common_compile_options(${TARGET})
endfunction()

function(tint_default_compile_options TARGET)
  tint_core_compile_options(${TARGET})

  set_target_properties(${TARGET} PROPERTIES LINKER_LANGUAGE CXX)

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

  if(COMPILER_IS_LIKE_GNU)
    target_compile_options(${TARGET} PRIVATE
      -pedantic-errors
      ${COMMON_GNU_OPTIONS}
    )

    if(COMPILER_IS_CLANG)
      target_compile_options(${TARGET} PRIVATE
        ${COMMON_CLANG_OPTIONS}
      )
    endif()
  endif(COMPILER_IS_LIKE_GNU)

  if(MSVC)
    # Specify /EHs for exception handling.
    target_compile_options(${TARGET} PRIVATE
      /bigobj
      /EHsc
      /W4
      /WX
      /wd4068 # unknown pragma
      /wd4127 # conditional expression is constant
      /wd4244 # 'conversion' conversion from 'type1' to 'type2', possible loss of data
      /wd4267 # 'var' : conversion from 'size_t' to 'type', possible loss of data
      /wd4324 # 'struct_name' : structure was padded due to __declspec(align())
      /wd4459 # declaration of 'identifier' hides global declaration
      /wd4458 # declaration of 'identifier' hides class member
      /wd4514 # 'function' : unreferenced inline function has been removed
      /wd4571 # catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
      /wd4625 # 'derived class' : copy constructor was implicitly defined as deleted because a base class copy constructor is inaccessible or deleted
      /wd4626 # 'derived class' : assignment operator was implicitly defined as deleted because a base class assignment operator is inaccessible or deleted
      /wd4710 # 'function' : function not inlined
      /wd4774 # 'function' : format string 'string' requires an argument of type 'type', but variadic argument number has type 'type'
      /wd4820 # 'bytes' bytes padding added after construct 'member_name'
      /wd5026 # 'type': move constructor was implicitly defined as deleted
      /wd5027 # 'type': move assignment operator was implicitly defined as deleted
    )

    # When building with clang-cl on Windows, try to match our clang build
    # options as much as possible.
    if(COMPILER_IS_CLANG_CL)
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

  if(TINT_RANDOMIZE_HASHES)
    if(NOT DEFINED TINT_HASH_SEED)
        string(RANDOM LENGTH 16 ALPHABET "0123456789abcdef" seed)
        set(TINT_HASH_SEED "0x${seed}" CACHE STRING "Tint hash seed value")
        message("Using TINT_HASH_SEED: ${TINT_HASH_SEED}")
    endif()
    target_compile_definitions(${TARGET} PUBLIC "-DTINT_HASH_SEED=${TINT_HASH_SEED}")
  endif()
endfunction()

function(tint_spvheaders_compile_options TARGET)
  target_link_libraries(${TARGET} PRIVATE SPIRV-Headers)
  target_include_directories(${TARGET} PRIVATE "${TINT_SPIRV_HEADERS_DIR}/include")
endfunction()

function(tint_spvtools_compile_options TARGET)
  target_link_libraries(${TARGET} PRIVATE SPIRV-Tools)
  target_include_directories(${TARGET} PRIVATE "${TINT_SPIRV_TOOLS_DIR}/include")
endfunction()

function(tint_bench_compile_options TARGET)
  tint_default_compile_options(${TARGET})
  set_target_properties(${TARGET} PROPERTIES FOLDER "Benchmarks")
  target_link_libraries(${TARGET} PRIVATE benchmark::benchmark)
endfunction()

function(tint_fuzzer_compile_options TARGET)
  tint_default_compile_options(${TARGET})
  target_link_libraries(${TARGET} PRIVATE "tint_api${TINT_FUZZ_SUFFIX}")

  if(NOT "${TINT_LIB_FUZZING_ENGINE_LINK_OPTIONS}" STREQUAL "")
    # This is set when the fuzzers are being built by OSS${TINT_FUZZ_SUFFIX}. In this case the
    # variable provides the necessary linker flags, and OSS${TINT_FUZZ_SUFFIX} will take care
    # of passing suitable compiler flags.
    target_link_options(${TARGET} PUBLIC ${TINT_LIB_FUZZING_ENGINE_LINK_OPTIONS})
  else()
    # When the fuzzers are being built outside of OSS${TINT_FUZZ_SUFFIX}, specific libFuzzer
    # arguments to enable fuzzing are used.
    target_link_options(${TARGET} PUBLIC -fsanitize=fuzzer -fsanitize-coverage=trace-cmp)
  endif()
endfunction()

if(TINT_ENABLE_BREAK_IN_DEBUGGER)
  set_source_files_properties(utils/debug/debugger.cc
    PROPERTIES COMPILE_DEFINITIONS "TINT_ENABLE_BREAK_IN_DEBUGGER=1" )
endif()

# If we're building on mac / ios and we have CoreGraphics, then we can use the
# metal API to validate our shaders. This is roughly 4x faster than invoking
# the metal shader compiler executable.
if(APPLE)
  find_library(LIB_CORE_GRAPHICS CoreGraphics)
  if(LIB_CORE_GRAPHICS)
    target_sources(tint_val PRIVATE "lang/msl/validate/msl_metal.mm")
    target_compile_definitions(tint_val PUBLIC "-DTINT_ENABLE_MSL_VALIDATION_USING_METAL_API=1")
    target_compile_options(tint_val PRIVATE "-fmodules" "-fcxx-modules")
    target_link_options(tint_val PUBLIC "-framework" "CoreGraphics")
  endif()
endif()

################################################################################
# Tests
################################################################################
if(TINT_BUILD_TESTS)
  add_executable(tint_unittests)

  tint_default_compile_options(tint_unittests)

  if(${TINT_BUILD_FUZZERS})
    target_sources(tint_unittests PRIVATE
      fuzzers/mersenne_twister_engine.cc
      fuzzers/mersenne_twister_engine.h
      fuzzers/random_generator.cc
      fuzzers/random_generator.h
      fuzzers/random_generator_engine.cc
      fuzzers/random_generator_engine.h
      fuzzers/random_generator_test.cc
    )
  endif()

  set_target_properties(tint_unittests PROPERTIES FOLDER "Tests")

  target_include_directories(tint_unittests PRIVATE
    ${gmock_SOURCE_DIR}/include
  )

  if(MSVC)
    # TODO(crbug.com/tint/1749): MSVC debug builds can suffer from stack
    # overflows when resolving deeply nested expression chains or statements.
    # Production builds neither use MSVC nor debug, so just bump the stack size
    # for this build combination.
    if(IS_DEBUG_BUILD)
      target_link_options(tint_unittests PRIVATE "/STACK:4194304") # 4MB, default is 1MB
    endif()
  else()
    target_compile_options(tint_unittests PRIVATE
      -Wno-global-constructors
      -Wno-weak-vtables
    )
  endif()

  target_link_libraries(tint_unittests PRIVATE gmock)

  add_test(NAME tint_unittests COMMAND tint_unittests)
endif(TINT_BUILD_TESTS)

################################################################################
# Benchmarks
################################################################################
if(TINT_BUILD_BENCHMARKS)
  add_executable(tint_benchmark
    "cmd/bench/main_bench.cc"
  )

  tint_core_compile_options(tint_benchmark)

  target_link_libraries(tint_benchmark PRIVATE benchmark::benchmark)

  if(TINT_EXTERNAL_BENCHMARK_CORPUS_DIR)
    # Glob all the files at TINT_EXTERNAL_BENCHMARK_CORPUS_DIR, and create a header
    # that lists these with the macros:
    # TINT_BENCHMARK_EXTERNAL_WGSL_PROGRAMS()
    # TINT_BENCHMARK_EXTERNAL_SPV_PROGRAMS()
    set(TINT_BENCHMARK_GEN_DIR "${DAWN_BUILD_GEN_DIR}/src/tint/benchmark/")
    set(TINT_BENCHMARK_EXTERNAL_SHADERS_HEADER "${TINT_BENCHMARK_GEN_DIR}/external_wgsl_programs.h")
    message("Globbing ${TINT_EXTERNAL_BENCHMARK_CORPUS_DIR}...")

    file(GLOB_RECURSE
      TINT_EXTERNAL_WGSL_BENCHMARK_FILES
      RELATIVE "${TINT_EXTERNAL_BENCHMARK_CORPUS_DIR}"
      "${TINT_EXTERNAL_BENCHMARK_CORPUS_DIR}/**.wgsl")
    list(TRANSFORM TINT_EXTERNAL_WGSL_BENCHMARK_FILES REPLACE
      "(.+)"
      "    BENCHMARK_CAPTURE\(FUNC, \"\\1\", \"${TINT_EXTERNAL_BENCHMARK_CORPUS_DIR}/\\1\")")
    list(JOIN TINT_EXTERNAL_WGSL_BENCHMARK_FILES "; \\\n" TINT_EXTERNAL_WGSL_BENCHMARK_FILES)

    file(GLOB_RECURSE
      TINT_EXTERNAL_SPV_BENCHMARK_FILES
      RELATIVE "${TINT_EXTERNAL_BENCHMARK_CORPUS_DIR}"
      "${TINT_EXTERNAL_BENCHMARK_CORPUS_DIR}/**.spv")

      list(TRANSFORM TINT_EXTERNAL_SPV_BENCHMARK_FILES REPLACE
      "(.+)"
      "    BENCHMARK_CAPTURE\(FUNC, \"\\1\", \"${TINT_EXTERNAL_BENCHMARK_CORPUS_DIR}/\\1\")")
    list(JOIN TINT_EXTERNAL_SPV_BENCHMARK_FILES "; \\\n" TINT_EXTERNAL_SPV_BENCHMARK_FILES)

    file(CONFIGURE
      OUTPUT "${TINT_BENCHMARK_EXTERNAL_SHADERS_HEADER}"
      CONTENT "
#define TINT_BENCHMARK_EXTERNAL_WGSL_PROGRAMS(FUNC) \\
${TINT_EXTERNAL_WGSL_BENCHMARK_FILES};

#define TINT_BENCHMARK_EXTERNAL_SPV_PROGRAMS(FUNC) \\
${TINT_EXTERNAL_SPV_BENCHMARK_FILES};")
    # Define TINT_BENCHMARK_EXTERNAL_SHADERS_HEADER to the generated header path
    target_compile_definitions(tint_benchmark PRIVATE
      "TINT_BENCHMARK_EXTERNAL_SHADERS_HEADER=\"${TINT_BENCHMARK_EXTERNAL_SHADERS_HEADER}\"")
  endif()
endif(TINT_BUILD_BENCHMARKS)

################################################################################
# Fuzzers
################################################################################
if(${TINT_BUILD_FUZZERS})
  add_subdirectory(fuzzers)
  set(TINT_FUZZ_SUFFIX "_fuzz")
endif()

################################################################################
# Functions used by BUILD.cmake files
# The CMake build handles the target kinds in different ways:
# 'lib'   - Translates to CMake static library.
#           If TINT_BUILD_FUZZERS is enabled, then a second static library with
#           the ${TINT_FUZZ_SUFFIX} suffix is also created. This is done because
#           the fuzzer build requires compilation with the '-fsanitize=fuzzer'
#           flag, which results in a separate set of compilation units.
# 'test'  - Appends files to the 'tint_unittests' CMake target.
#           Static libraries are not created for each test target, as gtest
#           relies on static constructors to register the test fixtures, and
#           these will get stripped by the linker if each target is packaged in
#           a separate library.
# 'bench' - Appends files to the 'tint_benchmark' CMake target.
#           Static libraries are not created for each test target, as google
#           benchmark relies on static constructors to register the benchmark
#           fixtures, and these will get stripped by the linker if each target
#           is packaged in a separate library.
# 'cmd'   - Translates to a CMake executable target.
# See also: docs/tint/gen.md
################################################################################

# A special name given to targets and target kinds that are not to be built due
# to the active CMake flags.
set(TINT_TARGET_DISABLED "<DISABLED>")

# tint_translate_target(TARGET_OUT KIND_OUT GN_TARGET_IN)
#
# Translates a GN-style target to a CMake target name, and target kind.
# The GN-style target is of the form `<path>` for a 'lib' target or
# `<path>:<kind>` for any other target kind.
#
# Parameters:
#   TARGET_OUT   - The name of the variable receieving the CMake target name
#   KIND_OUT     - The name of the variable receieving the target kind
#   GN_TARGET_IN - The GN-style target name
function(tint_translate_target TARGET_OUT KIND_OUT GN_TARGET_IN)
  string(REGEX MATCH "^([^:]+):(.+)$" MATCH ${GN_TARGET_IN})
  if (MATCH)
    # Explicit target kind
    set(TARGET ${CMAKE_MATCH_1})
    set(KIND ${CMAKE_MATCH_2})
  else()
    # No target kind. Most likely a 'lib', but needs to handle 'tint_unittests' or 'tint_benchmark'
    set(TARGET ${GN_TARGET_IN})
    set(KIND "<unknown>")
  endif()

  # Remove path slashes.
  string(REPLACE "/" "_" TARGET "${TARGET}")

  # Infer kind, and apply naming tweaks
  if ((${TARGET} STREQUAL "tint_unittests") OR (${KIND} STREQUAL "test"))
    if(TINT_BUILD_TESTS)
      set(TARGET "tint_unittests")
      set(KIND   "test")
    else()
      set(TARGET TINT_TARGET_DISABLED)
      set(KIND   TINT_TARGET_DISABLED)
    endif()
  elseif ((${TARGET} STREQUAL "tint_benchmark") OR (${KIND} STREQUAL "bench"))
    if(TINT_BUILD_BENCHMARKS)
      set(TARGET "tint_benchmark")
      set(KIND   "bench")
    else()
      set(TARGET TINT_TARGET_DISABLED)
      set(KIND   TINT_TARGET_DISABLED)
    endif()
  elseif (${KIND} STREQUAL "cmd")
    if(TINT_BUILD_CMD_TOOLS)
      set(TARGET "tint_${TARGET}")
      set(KIND   "cmd")
    else()
      set(TARGET TINT_TARGET_DISABLED)
      set(KIND   TINT_TARGET_DISABLED)
    endif()
  else()
    set(TARGET "tint_${TARGET}")
    set(KIND   "lib")
  endif()

  # Output values
  set(${TARGET_OUT} ${TARGET} PARENT_SCOPE)
  set(${KIND_OUT}   ${KIND}   PARENT_SCOPE)
endfunction()

# tint_add_target(GN_TARGET [SOURCES...])
#
# Registers the target with the given GN-style target name, with the provided
# sources. Additional sources can be appended with subsequent calls to
# tint_target_add_sources()
#
# Parameters:
#   GN_TARGET - The GN-style target name
#   SOURCES   - a list of source files, relative to this directory
function(tint_add_target GN_TARGET)
  set(SOURCES ${ARGN})

  # Split the GN-style target name into the CMake target name and target kind
  tint_translate_target(TARGET KIND ${GN_TARGET})

  if(${KIND} STREQUAL ${TINT_TARGET_DISABLED})
    return() # Target is disabled via build flags
  elseif(${KIND} STREQUAL lib)
    add_library(${TARGET} STATIC EXCLUDE_FROM_ALL)
    target_sources(${TARGET} PRIVATE ${SOURCES})
    tint_default_compile_options(${TARGET})

    if(TINT_BUILD_FUZZERS)
      # Create a second library target for use of the fuzzers, with the
      # ${TINT_FUZZ_SUFFIX} suffix
      set(FUZZ_TARGET "${TARGET}${TINT_FUZZ_SUFFIX}")
      add_library(${FUZZ_TARGET} STATIC EXCLUDE_FROM_ALL)
      target_sources(${FUZZ_TARGET} PRIVATE ${SOURCES})
      tint_default_compile_options(${FUZZ_TARGET})
    endif()
  elseif(${KIND} STREQUAL cmd)
    add_executable(${TARGET})
    target_sources(${TARGET} PRIVATE ${SOURCES})
    tint_default_compile_options(${TARGET})

    # Strip the 'tint_cmd_' prefix for commands under src/tint/cmd/...
    string(REGEX MATCH "^tint_cmd_(.+)$" MATCH ${CMD_NAME} ${TARGET})
    if (MATCH)
      set(OUTPUT_NAME ${CMAKE_MATCH_1})
      if(NOT OUTPUT_NAME STREQUAL "tint")
        set(OUTPUT_NAME "tint_${OUTPUT_NAME}")
      endif()
      set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME ${OUTPUT_NAME})
    endif()

  elseif(${KIND} STREQUAL test)
    target_sources(${TARGET} PRIVATE ${SOURCES})
  elseif(${KIND} STREQUAL bench)
    target_sources(${TARGET} PRIVATE ${SOURCES})
  else()
    message(FATAL_ERROR "unhandled kind ${KIND}")
  endif()
endfunction()

# tint_target_add_sources(GN_TARGET [SOURCES...])
#
# Adds source files to the target with the given GN-style target name.
#
# Parameters:
#   GN_TARGET - The GN-style target name
#   SOURCES   - a list of source files, relative to this directory
function(tint_target_add_sources GN_TARGET)
  set(SOURCES ${ARGN})

  # Split the GN-style target name into the CMake target name and target kind
  tint_translate_target(TARGET KIND ${GN_TARGET})

  if(${KIND} STREQUAL ${TINT_TARGET_DISABLED})
    return() # Target is disabled via build flags
  endif()

  target_sources(${TARGET} PRIVATE ${SOURCES})

  # If there's a corresponding fuzz target for this target, also append the files to that target
  set(FUZZ_TARGET "${TARGET}${TINT_FUZZ_SUFFIX}")
  if(TARGET "${FUZZ_TARGET}")
    target_sources("${FUZZ_TARGET}" PRIVATE ${SOURCES})
  endif()
endfunction()

# tint_target_add_dependencies(GN_TARGET DEPENDENCIES...)
#
# Adds dependencies to the target with the given GN-style target name.
#
# Parameters:
#   GN_TARGET    - The GN-style target name
#   DEPENDENCIES - a list of GN-style target names
function(tint_target_add_dependencies GN_TARGET)
  set(DEPENDENCIES ${ARGN})

  # Split the GN-style target name into the CMake target name and target kind
  tint_translate_target(TARGET KIND ${GN_TARGET})

  if(${KIND} STREQUAL ${TINT_TARGET_DISABLED})
    return() # Target is disabled via build flags
  endif()

  # Translate list of GN-style target names to CMake target names
  set(DEPENDENCY_TARGETS "")
  foreach(DEPENDENCY ${DEPENDENCIES})
    tint_translate_target(DEP_TARGET DEP_KIND ${DEPENDENCY})
    if (DEP_KIND STREQUAL lib) # If the dependency is a 'lib'
      list(APPEND DEPENDENCY_TARGETS ${DEP_TARGET})
    endif()
  endforeach()

  # Register the dependencies
  target_link_libraries(${TARGET} PRIVATE ${DEPENDENCY_TARGETS})

  # If there's a corresponding fuzz target for this target, add the corresponding fuzz dependencies
  # to the fuzz target.
  set(FUZZ_TARGET "${TARGET}${TINT_FUZZ_SUFFIX}")
  if(TARGET "${FUZZ_TARGET}")
    set(FUZZ_DEPENDENCY_TARGETS "")
    foreach(TARGET ${DEPENDENCY_TARGETS})
      list(APPEND FUZZ_DEPENDENCY_TARGETS "${TARGET}${TINT_FUZZ_SUFFIX}" )
    endforeach()
    target_link_libraries("${FUZZ_TARGET}" PRIVATE ${FUZZ_DEPENDENCY_TARGETS})
  endif()
endfunction()

# tint_target_add_external_dependencies(GN_TARGET DEPENDENCIES...)
#
# Adds external dependencies to the target with the given GN-style target name.
#
# Parameters:
#   GN_TARGET    - The GN-style target name
#   DEPENDENCIES - a list of external target names
#
# See tools/src/cmd/gen/build/externals.json for the list of external dependencies.
function(tint_target_add_external_dependencies GN_TARGET)
  set(DEPENDENCIES ${ARGN})

  # Split the GN-style target name into the CMake target name and target kind
  tint_translate_target(UNSUFFIXED_TARGET KIND ${GN_TARGET})

  if(${KIND} STREQUAL ${TINT_TARGET_DISABLED})
    return() # Target is disabled via build flags
  endif()

  # Build a list of targets that we're going to operate on
  set(TARGETS ${UNSUFFIXED_TARGET})
  if(TARGET "${UNSUFFIXED_TARGET}${TINT_FUZZ_SUFFIX}")
    list(APPEND TARGETS "${UNSUFFIXED_TARGET}${TINT_FUZZ_SUFFIX}")
  endif()

  foreach(TARGET ${TARGETS})
    foreach(DEPENDENCY ${DEPENDENCIES})
      # Each external dependency requires special handling...
      if(${DEPENDENCY} STREQUAL "abseil")
        target_link_libraries(${TARGET} PRIVATE
          absl_strings
        )
      elseif(${DEPENDENCY} STREQUAL "spirv-headers")
        tint_spvheaders_compile_options(${TARGET})
      elseif(${DEPENDENCY} STREQUAL "spirv-tools")
        tint_spvtools_compile_options(${TARGET})
      elseif(${DEPENDENCY} STREQUAL "spirv-opt-internal")
        target_link_libraries(${TARGET} PRIVATE
          SPIRV-Tools-opt
        )
        target_include_directories(${TARGET} PRIVATE
          "${TINT_SPIRV_TOOLS_DIR}"
          "${TINT_SPIRV_TOOLS_DIR}/include"
          "${TINT_SPIRV_TOOLS_DIR}/source"
          "${spirv-tools_BINARY_DIR}"
        )
      elseif(${DEPENDENCY} STREQUAL "glslang")
        target_link_libraries(${TARGET} PRIVATE glslang)
        if (NOT MSVC)
          target_compile_options(${TARGET} PRIVATE
            -Wno-reserved-id-macro
            -Wno-shadow-field-in-constructor
            -Wno-shadow
            -Wno-weak-vtables
          )
        endif()
      elseif(${DEPENDENCY} STREQUAL "glslang-res-limits")
        target_link_libraries(${TARGET} PRIVATE
          glslang-default-resource-limits
        )
      else()
        message(FATAL_ERROR "unhandled external dependency ${DEPENDENCY}")
      endif()
    endforeach()
  endforeach()
endfunction()

################################################################################
# Include the generated build files
################################################################################
include("BUILD.cmake")

################################################################################
# Tint aliases
################################################################################
add_library(libtint ALIAS tint_api)
