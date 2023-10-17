# Copyright 2021 The Dawn & Tint Authors
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

function(add_tint_regex_fuzzer NAME)
  add_executable(${NAME} ${NAME}.cc ${REGEX_FUZZER_SOURCES})
  target_link_libraries(${NAME} PRIVATE libtint_regex_fuzzer tint_lang_spirv_writer_helpers)
  tint_fuzzer_compile_options(${NAME})
  tint_spvtools_compile_options(${NAME})
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
