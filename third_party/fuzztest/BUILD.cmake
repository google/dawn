# Copyright 2026 The Dawn & Tint Authors
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

# A minimal library target for FuzzTest in libFuzzer compatibility mode.
add_library(fuzztest STATIC EXCLUDE_FROM_ALL
    "${DAWN_FUZZTEST_DIR}/common/bazel.cc"
    "${DAWN_FUZZTEST_DIR}/common/blob_file.cc"
    "${DAWN_FUZZTEST_DIR}/common/crashing_input_filename.cc"
    "${DAWN_FUZZTEST_DIR}/common/hash.cc"
    "${DAWN_FUZZTEST_DIR}/common/remote_file.cc"
    "${DAWN_FUZZTEST_DIR}/common/remote_file_oss.cc"
    "${DAWN_FUZZTEST_DIR}/common/sha1.cc"
    "${DAWN_FUZZTEST_DIR}/common/temp_dir.cc"
    "${DAWN_FUZZTEST_DIR}/fuzztest/fuzzing_bit_gen.cc"
    "${DAWN_FUZZTEST_DIR}/fuzztest/fuzztest_macros.cc"
    "${DAWN_FUZZTEST_DIR}/fuzztest/init_fuzztest.cc"
    "${DAWN_FUZZTEST_DIR}/fuzztest/internal/compatibility_mode.cc"
    "${DAWN_FUZZTEST_DIR}/fuzztest/internal/configuration.cc"
    "${DAWN_FUZZTEST_DIR}/fuzztest/internal/corpus_database.cc"
    "${DAWN_FUZZTEST_DIR}/fuzztest/internal/coverage.cc"
    "${DAWN_FUZZTEST_DIR}/fuzztest/internal/domains/in_grammar_impl.cc"
    "${DAWN_FUZZTEST_DIR}/fuzztest/internal/domains/in_regexp_impl.cc"
    "${DAWN_FUZZTEST_DIR}/fuzztest/internal/domains/regexp_dfa.cc"
    "${DAWN_FUZZTEST_DIR}/fuzztest/internal/domains/rune.cc"
    "${DAWN_FUZZTEST_DIR}/fuzztest/internal/domains/utf.cc"
    "${DAWN_FUZZTEST_DIR}/fuzztest/internal/escaping.cc"
    "${DAWN_FUZZTEST_DIR}/fuzztest/internal/fixture_driver.cc"
    "${DAWN_FUZZTEST_DIR}/fuzztest/internal/googletest_adaptor.cc"
    "${DAWN_FUZZTEST_DIR}/fuzztest/internal/io.cc"
    "${DAWN_FUZZTEST_DIR}/fuzztest/internal/logging.cc"
    "${DAWN_FUZZTEST_DIR}/fuzztest/internal/register_absl_fuzzing_mocks.cc"
    "${DAWN_FUZZTEST_DIR}/fuzztest/internal/registry.cc"
    "${DAWN_FUZZTEST_DIR}/fuzztest/internal/runtime.cc"
    "${DAWN_FUZZTEST_DIR}/fuzztest/internal/sanitizer_interface.cc"
    "${DAWN_FUZZTEST_DIR}/fuzztest/internal/seed_seq.cc"
    "${DAWN_FUZZTEST_DIR}/fuzztest/internal/serialization.cc"
    "${DAWN_FUZZTEST_DIR}/fuzztest/internal/status.cc"
    "${DAWN_FUZZTEST_DIR}/fuzztest/internal/subprocess.cc"
    "${DAWN_FUZZTEST_DIR}/fuzztest/internal/type_support.cc"
)

target_include_directories(fuzztest SYSTEM PUBLIC "${DAWN_FUZZTEST_DIR}")

target_compile_definitions(fuzztest PUBLIC
    "CENTIPEDE_DISABLE_RIEGELI"
    "FUZZTEST_NO_LEGACY_COVERAGE"
    "FUZZTEST_COMPATIBILITY_MODE"
)

if ((DAWN_ENABLE_ASAN OR DAWN_ENABLE_MSAN) AND NOT CMAKE_CXX_FLAGS MATCHES "-fsanitize=[^ ]*(address|memory)")
    # Dawn's DAWN_ENABLE_ASAN/DAWN_ENABLE_MSAN only instruments Dawn/Tint targets
    # (via common_compile_options), not third_party/abseil-cpp. Keep Abseil
    # container layouts (SwissTable/B-tree generations) ABI-compatible across the
    # boundary when FuzzTest headers are included by instrumented Tint targets.
    target_compile_definitions(fuzztest INTERFACE "NDEBUG_SANITIZER")
endif()

if (NOT MSVC)
    target_compile_options(fuzztest INTERFACE
        -Wno-sign-compare
    )
endif()

target_link_libraries(fuzztest PUBLIC
    absl::absl_check
    absl::absl_log
    absl::algorithm_container
    absl::any_invocable
    absl::bind_front
    absl::bits
    absl::cord
    absl::core_headers
    absl::failure_signal_handler
    absl::fast_type_id
    absl::flags
    absl::flags_parse
    absl::flags_reflection
    absl::flat_hash_map
    absl::flat_hash_set
    absl::function_ref
    absl::hash
    absl::int128
    absl::no_destructor
    absl::nullability
    absl::random_bit_gen_ref
    absl::random_distributions
    absl::random_mocking_access
    absl::random_random
    absl::random_seed_sequences
    absl::span
    absl::status
    absl::statusor
    absl::str_format
    absl::string_view
    absl::strings
    absl::symbolize
    absl::synchronization
    absl::time
    gtest
    libprotobuf
    re2
)

if (TINT_LIB_FUZZING_ENGINE_LINK_OPTIONS STREQUAL "")
    # Resolve Clang's libFuzzer runtime without main() so FuzzTest's own main()
    # can invoke LLVMFuzzerRunDriver in compatibility mode.
    set(_fuzztest_fuzzer_no_main "")
    foreach(_candidate
        "libclang_rt.fuzzer_no_main-${CMAKE_SYSTEM_PROCESSOR}.a"
        "libclang_rt.fuzzer_no_main.a"
        "libclang_rt.fuzzer_no_main_osx.a"
        "clang_rt.fuzzer_no_main-${CMAKE_SYSTEM_PROCESSOR}.lib"
        "clang_rt.fuzzer_no_main-x86_64.lib"
    )
        execute_process(
            COMMAND "${CMAKE_CXX_COMPILER}" "-print-file-name=${_candidate}"
            OUTPUT_VARIABLE _candidate_path
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if (_candidate_path AND NOT _candidate_path STREQUAL "${_candidate}" AND EXISTS "${_candidate_path}")
            set(_fuzztest_fuzzer_no_main "${_candidate_path}")
            break()
        endif()
    endforeach()

    if (NOT _fuzztest_fuzzer_no_main)
        message(FATAL_ERROR "Failed to locate Clang's fuzzer_no_main runtime library using '${CMAKE_CXX_COMPILER}'")
    endif()

    target_link_libraries(fuzztest PUBLIC "${_fuzztest_fuzzer_no_main}")

    if (NOT DAWN_ENABLE_ASAN AND NOT DAWN_ENABLE_MSAN AND NOT DAWN_ENABLE_TSAN)
        foreach(_interceptor_candidate
            "libclang_rt.fuzzer_interceptors-${CMAKE_SYSTEM_PROCESSOR}.a"
            "libclang_rt.fuzzer_interceptors.a"
            "libclang_rt.fuzzer_interceptors_osx.a"
        )
            execute_process(
                COMMAND "${CMAKE_CXX_COMPILER}" "-print-file-name=${_interceptor_candidate}"
                OUTPUT_VARIABLE _interceptor_path
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET
            )
            if (_interceptor_path AND NOT _interceptor_path STREQUAL "${_interceptor_candidate}" AND EXISTS "${_interceptor_path}")
                target_link_libraries(fuzztest PUBLIC "${_interceptor_path}")
                break()
            endif()
        endforeach()
    endif()
endif()

if (NOT MSVC)
    target_compile_options(fuzztest PRIVATE
        -Wno-error
        -fsanitize-coverage=0
        -Wno-sign-compare
        -Wno-extra-semi
        -Wno-unused-result
        -Wno-unused-private-field
        -Wno-unreachable-code-aggressive
        -Wno-unused-but-set-variable
        -Wno-shadow
        -Wno-unused-const-variable
        -Wno-unused-function
        -Wno-inconsistent-missing-override
        -Wno-unused-lambda-capture
        -Wno-c++98-compat-extra-semi
        -Wno-deprecated-declarations
    )
endif()
