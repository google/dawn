# Copyright 2020 The Dawn & Tint Authors
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

################################################################################
# Target flags
################################################################################
if(NOT TINT_BUILD_AS_OTHER_OS)
  if(APPLE)  # Must come before UNIX
    set(IS_MAC TRUE)
    set(IS_MAC TRUE)
  elseif(UNIX)
    set(IS_LINUX TRUE)
    set(IS_LINUX TRUE)
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
  target_compile_definitions(${TARGET} PUBLIC -DTINT_BUILD_GLSL_VALIDATOR=$<BOOL:${TINT_BUILD_GLSL_VALIDATOR}>)
  target_compile_definitions(${TARGET} PUBLIC -DTINT_BUILD_HLSL_WRITER=$<BOOL:${TINT_BUILD_HLSL_WRITER}>)
  target_compile_definitions(${TARGET} PUBLIC -DTINT_BUILD_MSL_WRITER=$<BOOL:${TINT_BUILD_MSL_WRITER}>)
  target_compile_definitions(${TARGET} PUBLIC -DTINT_BUILD_SPV_WRITER=$<BOOL:${TINT_BUILD_SPV_WRITER}>)
  target_compile_definitions(${TARGET} PUBLIC -DTINT_BUILD_WGSL_WRITER=$<BOOL:${TINT_BUILD_WGSL_WRITER}>)
  target_compile_definitions(${TARGET} PUBLIC -DTINT_BUILD_SYNTAX_TREE_WRITER=$<BOOL:${TINT_BUILD_SYNTAX_TREE_WRITER}>)

  if(TINT_BUILD_FUZZERS)
    target_compile_options(${TARGET} PRIVATE "-fsanitize=fuzzer")
  endif()

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

  if(COMPILER_IS_GNU)
    # https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80635
    # Despite the bug being closed, false-positives still seen in GCC-13.
    target_compile_options(${TARGET} PRIVATE -Wno-maybe-uninitialized)
  endif(COMPILER_IS_GNU)

  if(COMPILER_IS_LIKE_GNU)
    target_compile_options(${TARGET} PRIVATE
      -pedantic-errors
      ${COMMON_GNU_OPTIONS}
    )

    if(COMPILER_IS_CLANG)
      if(IS_DEBUG_BUILD)
        target_compile_options(${TARGET} PRIVATE -fstandalone-debug)
      endif()
      target_compile_options(${TARGET} PRIVATE ${COMMON_CLANG_OPTIONS})
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

function(tint_test_compile_options TARGET)
  tint_default_compile_options(${TARGET})
  set_target_properties(${TARGET} PROPERTIES FOLDER "Tests")

  if(NOT MSVC)
    target_compile_options(${TARGET} PRIVATE
      -Wno-global-constructors
      -Wno-weak-vtables
    )
  endif()
endfunction()

function(tint_bench_compile_options TARGET)
  tint_core_compile_options(${TARGET})
endfunction()

function(tint_fuzz_compile_options TARGET)
  tint_core_compile_options(${TARGET})
  set_target_properties(${TARGET} PROPERTIES FOLDER "Fuzzers")
endfunction()

function(tint_test_cmd_compile_options TARGET)
  tint_test_compile_options(${TARGET})

  if(MSVC)
    # TODO(crbug.com/tint/1749): MSVC debug builds can suffer from stack
    # overflows when resolving deeply nested expression chains or statements.
    # Production builds neither use MSVC nor debug, so just bump the stack size
    # for this build combination.
    if(IS_DEBUG_BUILD)
      target_link_options(${TARGET} PRIVATE "/STACK:4194304") # 4MB, default is 1MB
    endif()
  endif()

  target_link_libraries(${TARGET} PRIVATE gmock)
endfunction()

function(tint_bench_cmd_compile_options TARGET)
  tint_bench_compile_options(${TARGET})
  target_link_libraries(${TARGET} PRIVATE benchmark::benchmark)
endfunction()

function(tint_fuzz_cmd_compile_options TARGET)
  tint_fuzz_compile_options(${TARGET})

  if(NOT "${TINT_LIB_FUZZING_ENGINE_LINK_OPTIONS}" STREQUAL "")
    # This is set when the fuzzers are being built by OSS-Fuzz. In this case the
    # variable provides the necessary linker flags, and OSS-Fuzz will take care
    # of passing suitable compiler flags.
    target_link_options(${TARGET} PUBLIC ${TINT_LIB_FUZZING_ENGINE_LINK_OPTIONS})
  else()
    # When the fuzzers are being built outside of OSS-Fuzz, specific libFuzzer
    # arguments to enable fuzzing are used.
    target_link_options(${TARGET} PUBLIC -fsanitize=fuzzer -fsanitize-coverage=trace-cmp)
  endif()

  # Link the version of tint_api with -sanitize=fuzzer enabled
  target_link_libraries(${TARGET} PRIVATE "tint_api_sanitize_fuzzer")
endfunction()

# TODO(bclayton): Remove this when fuzzers fully migrated to gen build
function(tint_fuzzer_compile_options TARGET)
  tint_fuzz_cmd_compile_options(${TARGET})
  target_link_libraries(${TARGET} PRIVATE "tint_api_sanitize_fuzzer")
endfunction()

if(TINT_ENABLE_BREAK_IN_DEBUGGER)
  set_source_files_properties(utils/debug/debugger.cc
    PROPERTIES COMPILE_DEFINITIONS "TINT_ENABLE_BREAK_IN_DEBUGGER=1")
endif()

################################################################################
# Benchmarks
################################################################################
if(TINT_BUILD_BENCHMARKS AND TINT_EXTERNAL_BENCHMARK_CORPUS_DIR)
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
    "${TINT_EXTERNAL_BENCHMARK_CORPUS_DIR}/**.wgsl"
  )
  list(TRANSFORM TINT_EXTERNAL_WGSL_BENCHMARK_FILES REPLACE
    "(.+)"
    "    BENCHMARK_CAPTURE\(FUNC, \"\\1\", \"${TINT_EXTERNAL_BENCHMARK_CORPUS_DIR}/\\1\")"
  )
  list(JOIN TINT_EXTERNAL_WGSL_BENCHMARK_FILES "; \\\n" TINT_EXTERNAL_WGSL_BENCHMARK_FILES)

  file(GLOB_RECURSE
    TINT_EXTERNAL_SPV_BENCHMARK_FILES
    RELATIVE "${TINT_EXTERNAL_BENCHMARK_CORPUS_DIR}"
    "${TINT_EXTERNAL_BENCHMARK_CORPUS_DIR}/**.spv")

  list(TRANSFORM TINT_EXTERNAL_SPV_BENCHMARK_FILES REPLACE
    "(.+)"
    "    BENCHMARK_CAPTURE\(FUNC, \"\\1\", \"${TINT_EXTERNAL_BENCHMARK_CORPUS_DIR}/\\1\")"
  )
  list(JOIN TINT_EXTERNAL_SPV_BENCHMARK_FILES "; \\\n" TINT_EXTERNAL_SPV_BENCHMARK_FILES)

  file(CONFIGURE
    OUTPUT "${TINT_BENCHMARK_EXTERNAL_SHADERS_HEADER}"
    CONTENT "
#define TINT_BENCHMARK_EXTERNAL_WGSL_PROGRAMS(FUNC) \\
${TINT_EXTERNAL_WGSL_BENCHMARK_FILES};

#define TINT_BENCHMARK_EXTERNAL_SPV_PROGRAMS(FUNC) \\
${TINT_EXTERNAL_SPV_BENCHMARK_FILES};"
  )
endif(TINT_BUILD_BENCHMARKS AND TINT_EXTERNAL_BENCHMARK_CORPUS_DIR)

################################################################################
# Functions used by BUILD.cmake files
# The CMake build handles the target kinds in different ways:
# 'cmd'       - Translates to a CMake executable target.
# 'lib'       - Translates to a CMake static library.
# 'test'      - Translates to a CMake object library, configured for compiling and
#               linking against google-test.
# 'bench'     - Translates to a CMake object library, configured for compiling and
#               linking against google-benchmark.
# 'fuzz'      - Translates to a CMake object library, configured for compiling and
#               linking against libfuzzer.
# 'test_cmd'  - Translates to a CMake executable target linked against google-test.
# 'bench_cmd' - Translates to a CMake executable target linked against google-benchmark.
# 'fuzz_cmd'  - Translates to a CMake executable target linked against libfuzz.
# See also: docs/tint/gen.md
################################################################################

# tint_check_target_is_enabled(IS_ENABLED KIND)
#
# Checks whether a target of the kind KIND is enabled.
# Assigns TRUE or FALSE to IS_ENABLED based on whether the target is enabled.
#
# Parameters:
#   IS_ENABLED - the output variable
#   KIND       - the target kind
function(tint_check_target_is_enabled IS_ENABLED KIND)
  set(IS_ENABLED FALSE PARENT_SCOPE) # Default to disabled
  if(${KIND} STREQUAL cmd)
    if(TINT_BUILD_CMD_TOOLS)
      set(IS_ENABLED TRUE PARENT_SCOPE)
    endif()
  elseif(${KIND} STREQUAL lib)
    set(IS_ENABLED TRUE PARENT_SCOPE)
  elseif((${KIND} STREQUAL test) OR (${KIND} STREQUAL test_cmd))
    if(TINT_BUILD_TESTS)
      set(IS_ENABLED TRUE PARENT_SCOPE)
    endif()
  elseif((${KIND} STREQUAL bench) OR (${KIND} STREQUAL bench_cmd))
    if(TINT_BUILD_BENCHMARKS)
      set(IS_ENABLED TRUE PARENT_SCOPE)
    endif()
  elseif((${KIND} STREQUAL fuzz) OR (${KIND} STREQUAL fuzz_cmd))
    if(TINT_BUILD_FUZZERS)
      set(IS_ENABLED TRUE PARENT_SCOPE)
    endif()
  else()
    message(FATAL_ERROR "unhandled target kind ${KIND}")
  endif()
endfunction()

# tint_add_target(TARGET KIND [SOURCES...])
#
# Registers a Tint target with the provided sources.
# Additional sources can be appended with subsequent calls to tint_target_add_sources()
#
# Parameters:
#   TARGET   - The target name
#   KIND     - The target kind
#   SOURCES  - a list of source files, relative to this directory
function(tint_add_target TARGET KIND)
  set(TARGET "${TARGET}${TINT_TARGET_SUFFIX}") # Apply suffix
  set(SOURCES ${ARGN})

  tint_check_target_is_enabled(IS_ENABLED ${KIND})
  if(NOT IS_ENABLED)
    return() # Target is disabled via build flags
  endif()

  if(${KIND} STREQUAL lib)
    add_library(${TARGET} STATIC EXCLUDE_FROM_ALL)
    if (TINT_ENABLE_INSTALL)
      install(TARGETS ${TARGET}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      )
    endif()
    tint_default_compile_options(${TARGET})
  elseif(${KIND} STREQUAL cmd)
    add_executable(${TARGET})
    tint_default_compile_options(${TARGET})
  elseif(${KIND} STREQUAL test_cmd)
    add_executable(${TARGET})
    tint_test_cmd_compile_options(${TARGET})
  elseif(${KIND} STREQUAL bench_cmd)
    add_executable(${TARGET})
    tint_bench_cmd_compile_options(${TARGET})
  elseif(${KIND} STREQUAL fuzz_cmd)
    add_executable(${TARGET})
    tint_fuzz_cmd_compile_options(${TARGET})
  elseif(${KIND} STREQUAL test)
    add_library(${TARGET} OBJECT EXCLUDE_FROM_ALL)
    tint_test_compile_options(${TARGET})
  elseif(${KIND} STREQUAL bench)
    add_library(${TARGET} OBJECT EXCLUDE_FROM_ALL)
    tint_bench_compile_options(${TARGET})
  elseif(${KIND} STREQUAL fuzz)
    add_library(${TARGET} OBJECT EXCLUDE_FROM_ALL)
    tint_fuzz_compile_options(${TARGET})
  else()
    message(FATAL_ERROR "unhandled target kind ${KIND}")
  endif()

  # Add the sources to the target
  target_sources(${TARGET} PRIVATE ${SOURCES})
endfunction()

# tint_target_add_sources(TARGET [SOURCES...])
#
# Adds source files to a Tint target
#
# Parameters:
#   TARGET   - The target name
#   KIND     - The target kind
#   SOURCES  - a list of source files, relative to this directory
function(tint_target_add_sources TARGET KIND)
  set(TARGET "${TARGET}${TINT_TARGET_SUFFIX}") # Apply suffix
  set(SOURCES ${ARGN})

  tint_check_target_is_enabled(IS_ENABLED ${KIND})
  if(NOT IS_ENABLED)
    return() # Target is disabled via build flags
  endif()

  target_sources(${TARGET} PRIVATE ${SOURCES})
endfunction()

# tint_target_add_dependencies(TARGET DEPENDENCIES...)
#
# Adds dependencies to a Tint target.
#
# Parameters:
#   TARGET       - The target name
#   KIND         - The target kind
#   DEPENDENCIES - a list of target names
function(tint_target_add_dependencies TARGET KIND)
  set(TARGET "${TARGET}${TINT_TARGET_SUFFIX}") # Apply suffix
  set(DEPENDENCIES ${ARGN})

  tint_check_target_is_enabled(IS_ENABLED ${KIND})
  if(NOT IS_ENABLED)
    return() # Target is disabled via build flags
  endif()

  # Apply target suffix
  set(SUFFIXED_DEPENDENCIES "")
  foreach(DEPENDENCY ${DEPENDENCIES})
    list(APPEND SUFFIXED_DEPENDENCIES "${DEPENDENCY}${TINT_TARGET_SUFFIX}")
  endforeach()

  # Register the dependencies
  target_link_libraries(${TARGET} PRIVATE ${SUFFIXED_DEPENDENCIES})
endfunction()

# tint_target_add_external_dependencies(TARGET KIND DEPENDENCIES...)
#
# Adds external dependencies to a Tint target.
#
# Parameters:
#   TARGET       - The target name
#   KIND         - The target kind
#   DEPENDENCIES - a list of external target names
#
# See src/tint/externals.json for the list of external dependencies.
function(tint_target_add_external_dependencies TARGET KIND)
  set(TARGET "${TARGET}${TINT_TARGET_SUFFIX}") # Apply suffix
  set(DEPENDENCIES ${ARGN})

  tint_check_target_is_enabled(IS_ENABLED ${KIND})
  if(NOT IS_ENABLED)
    return() # Target is disabled via build flags
  endif()

  foreach(DEPENDENCY ${DEPENDENCIES})  # Each external dependency requires special handling...
    if(${DEPENDENCY} STREQUAL "abseil")
      target_link_libraries(${TARGET} PRIVATE
        absl_strings
      )
    elseif(${DEPENDENCY} STREQUAL "glslang")
      target_link_libraries(${TARGET} PRIVATE glslang)
      if(NOT MSVC)
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
    elseif(${DEPENDENCY} STREQUAL "google-benchmark")
      set_target_properties(${TARGET} PROPERTIES FOLDER "Benchmarks")
      target_link_libraries(${TARGET} PRIVATE benchmark::benchmark)
      if(TINT_BENCHMARK_EXTERNAL_SHADERS_HEADER)
        target_compile_definitions(${TARGET} PRIVATE
          "TINT_BENCHMARK_EXTERNAL_SHADERS_HEADER=\"${TINT_BENCHMARK_EXTERNAL_SHADERS_HEADER}\""
        )
      endif()
    elseif(${DEPENDENCY} STREQUAL "gtest")
      target_include_directories(${TARGET} PRIVATE ${gmock_SOURCE_DIR}/include)
      target_link_libraries(${TARGET} PRIVATE gmock)
    elseif(${DEPENDENCY} STREQUAL "metal")
      find_library(CoreGraphicsFramework CoreGraphics REQUIRED)
      find_library(FoundationFramework Foundation REQUIRED)
      find_library(MetalFramework Metal REQUIRED)
      target_link_libraries(${TARGET} PRIVATE
        ${CoreGraphicsFramework}
        ${FoundationFramework}
        ${MetalFramework}
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
    elseif(${DEPENDENCY} STREQUAL "thread")
      find_package(Threads REQUIRED)
      target_link_libraries(${TARGET} PRIVATE Threads::Threads)
    elseif(${DEPENDENCY} STREQUAL "winsock")
      target_link_libraries(${TARGET} PRIVATE ws2_32)
    else()
      message(FATAL_ERROR "unhandled external dependency ${DEPENDENCY}")
    endif()
  endforeach()
endfunction()

# tint_target_set_output_name(TARGET KIND OUTPUT_NAME)
#
# Overrides the output name for the given target
#
# Parameters:
#   TARGET      - The target name
#   KIND        - The target kind
#   OUTPUT_NAME - the new name for the target output
function(tint_target_set_output_name TARGET KIND OUTPUT_NAME)
  set(TARGET "${TARGET}${TINT_TARGET_SUFFIX}") # Apply suffix
  tint_check_target_is_enabled(IS_ENABLED ${KIND})
  if(NOT IS_ENABLED)
    return() # Target is disabled via build flags
  endif()

  set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME ${OUTPUT_NAME})

  # Create an alias target with the name OUTPUT_NAME to TARGET
  if(${KIND} STREQUAL lib)
    add_library(${OUTPUT_NAME} ALIAS ${TARGET})
  elseif(${KIND} STREQUAL test)
    add_library(${OUTPUT_NAME} ALIAS ${TARGET})
  elseif(${KIND} STREQUAL bench)
    add_library(${OUTPUT_NAME} ALIAS ${TARGET})
  elseif(${KIND} STREQUAL fuzz)
    add_library(${OUTPUT_NAME} ALIAS ${TARGET})
  elseif(${KIND} STREQUAL cmd)
    add_executable(${OUTPUT_NAME} ALIAS ${TARGET})
  elseif(${KIND} STREQUAL test_cmd)
    add_executable(${OUTPUT_NAME} ALIAS ${TARGET})
  elseif(${KIND} STREQUAL bench_cmd)
    add_executable(${OUTPUT_NAME} ALIAS ${TARGET})
  elseif(${KIND} STREQUAL fuzz_cmd)
    add_executable(${OUTPUT_NAME} ALIAS ${TARGET})
  else()
    message(FATAL_ERROR "unhandled target kind ${KIND}")
  endif()
endfunction()

################################################################################
# Include the generated build files
################################################################################
if(TINT_BUILD_FUZZERS)
  if(NOT COMPILER_IS_CLANG)
    message(FATAL_ERROR "TINT_BUILD_FUZZERS can only be enabled with the Clang toolchain")
  endif()

  # Save the current build flags
  set(SAVE_TINT_BUILD_CMD_TOOLS ${TINT_BUILD_CMD_TOOLS})
  set(SAVE_TINT_BUILD_TESTS ${TINT_BUILD_TESTS})
  set(SAVE_TINT_BUILD_BENCHMARKS ${TINT_BUILD_BENCHMARKS})
  set(SAVE_TINT_BUILD_FUZZERS ${TINT_BUILD_FUZZERS})
  set(SAVE_TINT_TARGET_SUFFIX ${TINT_TARGET_SUFFIX})

  # Declare the targets with fuzzers disabled
  set(TINT_BUILD_FUZZERS FALSE)
  include("BUILD.cmake")

  # Now redeclare the fuzzers targets with a '_sanitize_fuzzer' target suffix
  # Enabling TINT_BUILD_FUZZERS will enable the -fsanitize=fuzzer compilation flag for these
  # targets.
  set(TINT_TARGET_SUFFIX "_sanitize_fuzzer")
  set(TINT_BUILD_FUZZERS TRUE)
  set(TINT_BUILD_CMD_TOOLS FALSE)
  set(TINT_BUILD_TESTS FALSE)
  set(TINT_BUILD_BENCHMARKS FALSE)
  include("BUILD.cmake")
  add_subdirectory(fuzzers)

  # Restore the build flags
  set(TINT_BUILD_CMD_TOOLS ${SAVE_TINT_BUILD_CMD_TOOLS})
  set(TINT_BUILD_TESTS ${SAVE_TINT_BUILD_TESTS})
  set(TINT_BUILD_BENCHMARKS ${SAVE_TINT_BUILD_BENCHMARKS})
  set(TINT_BUILD_FUZZERS ${SAVE_TINT_BUILD_FUZZERS})
  set(TINT_TARGET_SUFFIX ${SAVE_TINT_TARGET_SUFFIX})
else()
  # Fuzzers not enabled. Just include BUILD.cmake with the current flags.
  include("BUILD.cmake")
endif(TINT_BUILD_FUZZERS)


################################################################################
# Bespoke target settings
################################################################################
if (MSVC)
  target_sources(tint_api PRIVATE tint.natvis)
endif()

################################################################################
# Additional fuzzer tests
################################################################################
if(TINT_BUILD_TESTS)
  if(${TINT_BUILD_FUZZERS})
    target_sources(tint_cmd_test_test_cmd PRIVATE
      fuzzers/mersenne_twister_engine.cc
      fuzzers/mersenne_twister_engine.h
      fuzzers/random_generator.cc
      fuzzers/random_generator.h
      fuzzers/random_generator_engine.cc
      fuzzers/random_generator_engine.h
      fuzzers/random_generator_test.cc
    )
  endif()

  add_test(NAME tint_unittests COMMAND tint_cmd_test_test_cmd)
endif(TINT_BUILD_TESTS)

################################################################################
# Target aliases
################################################################################
add_library(libtint ALIAS tint_api)

if (TINT_ENABLE_INSTALL)
  # We need to recurse all folders and include all headers because tint.h includes most headers from src/tint
  file(GLOB TINT_HEADERS "${DAWN_INCLUDE_DIR}/tint/*.h")

  install(FILES ${TINT_HEADERS}  DESTINATION  ${CMAKE_INSTALL_INCLUDEDIR}/tint/)

  file(GLOB_RECURSE TINT_SRC_HEADERS RELATIVE ${CMAKE_SOURCE_DIR}/src/tint/ "*.h")

  foreach(TINT_HEADER_FILE ${TINT_SRC_HEADERS})
      get_filename_component(TINT_HEADER_DIR ${TINT_HEADER_FILE} DIRECTORY)
      install(FILES ${CMAKE_SOURCE_DIR}/src/tint/${TINT_HEADER_FILE}  DESTINATION  ${CMAKE_INSTALL_INCLUDEDIR}/src/tint/${TINT_HEADER_DIR})
  endforeach ()
endif()
