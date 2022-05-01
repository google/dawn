// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SRC_TINT_FUZZERS_TINT_AST_FUZZER_CLI_H_
#define SRC_TINT_FUZZERS_TINT_AST_FUZZER_CLI_H_

#include <cstdint>

namespace tint::fuzzers::ast_fuzzer {

/// The backend this fuzzer will test.
enum class FuzzingTarget {
    kNone = 0,
    kHlsl = 1 << 0,
    kMsl = 1 << 1,
    kSpv = 1 << 2,
    kWgsl = 1 << 3,
    kAll = kHlsl | kMsl | kSpv | kWgsl
};

inline FuzzingTarget operator|(FuzzingTarget a, FuzzingTarget b) {
    return static_cast<FuzzingTarget>(static_cast<int>(a) | static_cast<int>(b));
}

inline FuzzingTarget operator&(FuzzingTarget a, FuzzingTarget b) {
    return static_cast<FuzzingTarget>(static_cast<int>(a) & static_cast<int>(b));
}

/// CLI parameters accepted by the fuzzer. Type -tint_help in the CLI to see the
/// help message
struct CliParams {
    /// Whether to use all mutation finders or only a randomly selected subset of
    /// them.
    bool enable_all_mutations = false;

    /// The maximum number of mutations applied during a single mutation session
    /// (i.e. a call to `ast_fuzzer::Mutate` function).
    uint32_t mutation_batch_size = 5;

    /// Compiler backends we want to fuzz.
    FuzzingTarget fuzzing_target = FuzzingTarget::kAll;
};

/// @brief Parses CLI parameters.
///
/// This function will exit the process with non-zero return code if some
/// parameters are invalid. This function will remove recognized parameters from
/// `argv` and adjust `argc` accordingly.
///
/// @param argc - the total number of parameters.
/// @param argv - array of all CLI parameters.
/// @return parsed parameters.
CliParams ParseCliParams(int* argc, char** argv);

}  // namespace tint::fuzzers::ast_fuzzer

#endif  // SRC_TINT_FUZZERS_TINT_AST_FUZZER_CLI_H_
