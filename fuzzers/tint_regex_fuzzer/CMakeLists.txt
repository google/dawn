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

set(LIBTINT_REGEX_FUZZER_SOURCES
        util.h
        wgsl_mutator.cc
        wgsl_mutator.h)

# Add static library target.
add_library(libtint_regex_fuzzer STATIC ${LIBTINT_REGEX_FUZZER_SOURCES})
tint_default_compile_options(libtint_regex_fuzzer)

set(REGEX_FUZZER_SOURCES
        cli.cc
        cli.h
        fuzzer.cc
        ../tint_common_fuzzer.cc
        ../tint_common_fuzzer.h)

# Add libfuzzer target.
add_executable(tint_regex_fuzzer ${REGEX_FUZZER_SOURCES})
target_compile_options(tint_regex_fuzzer PRIVATE
        -Wno-missing-prototypes)
target_link_libraries(tint_regex_fuzzer libtint-fuzz libtint_regex_fuzzer)
tint_default_compile_options(tint_regex_fuzzer)
target_compile_definitions(tint_regex_fuzzer PUBLIC CUSTOM_MUTATOR)

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
