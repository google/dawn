# Copyright 2021 The Tint Authors.
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

function(add_tint_regex_fuzzer NAME)
  add_executable(${NAME} ${NAME}.cc ${REGEX_FUZZER_SOURCES})
  target_link_libraries(${NAME} libtint-fuzz libtint_regex_fuzzer)
  tint_default_compile_options(${NAME})
  target_compile_definitions(${NAME} PRIVATE CUSTOM_MUTATOR)
  target_include_directories(${NAME} PRIVATE ${CMAKE_BINARY_DIR})
endfunction()

set(LIBTINT_REGEX_FUZZER_SOURCES
        ../mersenne_twister_engine.cc
        ../mersenne_twister_engine.h
        ../random_generator.cc
        ../random_generator.h
        ../random_generator_engine.cc
        ../random_generator_engine.h
        wgsl_mutator.cc
        wgsl_mutator.h)

# Add static library target.
add_library(libtint_regex_fuzzer STATIC ${LIBTINT_REGEX_FUZZER_SOURCES})
tint_default_compile_options(libtint_regex_fuzzer)

set(REGEX_FUZZER_SOURCES
        cli.cc
        cli.h
        fuzzer.cc
        override_cli_params.h
        ../tint_common_fuzzer.cc
        ../tint_common_fuzzer.h)

set_source_files_properties(fuzzer.cc PROPERTIES COMPILE_FLAGS -Wno-missing-prototypes)

# Add libfuzzer targets.
# Targets back-ends according to command line arguments.
add_tint_regex_fuzzer(tint_regex_fuzzer)
# Targets back-ends individually.
add_tint_regex_fuzzer(tint_regex_hlsl_writer_fuzzer)
add_tint_regex_fuzzer(tint_regex_msl_writer_fuzzer)
add_tint_regex_fuzzer(tint_regex_spv_writer_fuzzer)
add_tint_regex_fuzzer(tint_regex_wgsl_writer_fuzzer)

# Add tests.
if (${TINT_BUILD_TESTS})
    set(TEST_SOURCES
            regex_fuzzer_tests.cc)

    add_executable(tint_regex_fuzzer_unittests ${TEST_SOURCES})

    target_include_directories(
            tint_regex_fuzzer_unittests PRIVATE ${gmock_SOURCE_DIR}/include)
    target_link_libraries(tint_regex_fuzzer_unittests gmock_main libtint_regex_fuzzer)
    tint_default_compile_options(tint_regex_fuzzer_unittests)
    target_compile_options(tint_regex_fuzzer_unittests PRIVATE
            -Wno-global-constructors
            -Wno-weak-vtables
            -Wno-covered-switch-default)

    target_include_directories(tint_regex_fuzzer_unittests PRIVATE ${CMAKE_BINARY_DIR})

    add_test(NAME tint_regex_fuzzer_unittests COMMAND tint_regex_fuzzer_unittests)
endif ()
