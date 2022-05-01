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

#ifndef SRC_TINT_FUZZERS_TINT_SPIRV_TOOLS_FUZZER_CLI_H_
#define SRC_TINT_FUZZERS_TINT_SPIRV_TOOLS_FUZZER_CLI_H_

#include <string>
#include <vector>

#include "source/fuzz/fuzzer.h"

namespace tint::fuzzers::spvtools_fuzzer {

/// Default SPIR-V environment that will be used during fuzzing.
const auto kDefaultTargetEnv = SPV_ENV_VULKAN_1_1;

/// The type of the mutator to run.
enum class MutatorType {
    kNone = 0,
    kFuzz = 1 << 0,
    kReduce = 1 << 1,
    kOpt = 1 << 2,
    kAll = kFuzz | kReduce | kOpt
};

inline MutatorType operator|(MutatorType a, MutatorType b) {
    return static_cast<MutatorType>(static_cast<int>(a) | static_cast<int>(b));
}

inline MutatorType operator&(MutatorType a, MutatorType b) {
    return static_cast<MutatorType>(static_cast<int>(a) & static_cast<int>(b));
}

/// Shading language to target during fuzzing.
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

/// These parameters are accepted by various mutators and thus they are accepted
/// by both the fuzzer and the mutator debugger.
struct MutatorCliParams {
    /// SPIR-V target environment for fuzzing.
    spv_target_env target_env = kDefaultTargetEnv;

    /// The number of spirv-fuzz transformations to apply at a time.
    uint32_t transformation_batch_size = 3;

    /// The number of spirv-reduce reductions to apply at a time.
    uint32_t reduction_batch_size = 3;

    /// The number of spirv-opt optimizations to apply at a time.
    uint32_t opt_batch_size = 6;

    /// The vector of donors to use in spirv-fuzz (see the doc for spirv-fuzz to
    /// learn more).
    std::vector<spvtools::fuzz::fuzzerutil::ModuleSupplier> donors = {};

    /// The strategy to use during fuzzing in spirv-fuzz (see the doc for
    /// spirv-fuzz to learn more).
    spvtools::fuzz::RepeatedPassStrategy repeated_pass_strategy =
        spvtools::fuzz::RepeatedPassStrategy::kSimple;

    /// Whether to use all fuzzer passes or a randomly selected subset of them.
    bool enable_all_fuzzer_passes = false;

    /// Whether to use all reduction passes or a randomly selected subset of them.
    bool enable_all_reduce_passes = false;

    /// Whether to validate the SPIR-V binary after each optimization pass.
    bool validate_after_each_opt_pass = true;

    /// Whether to validate the SPIR-V binary after each fuzzer pass.
    bool validate_after_each_fuzzer_pass = true;

    /// Whether to validate the SPIR-V binary after each reduction pass.
    bool validate_after_each_reduce_pass = true;
};

/// Parameters specific to the fuzzer. Type `-tint_help` in the CLI to learn
/// more.
struct FuzzerCliParams {
    /// The size of the cache that records ongoing mutation sessions.
    uint32_t mutator_cache_size = 20;

    /// The type of the mutator to run.
    MutatorType mutator_type = MutatorType::kAll;

    /// Tint backend to fuzz.
    FuzzingTarget fuzzing_target = FuzzingTarget::kAll;

    /// The path to the directory, that will be used to output buggy shaders.
    std::string error_dir = "";

    /// Parameters for various mutators.
    MutatorCliParams mutator_params;
};

/// Parameters specific to the mutator debugger. Type `--help` in the CLI to
/// learn more.
struct MutatorDebuggerCliParams {
    /// The type of the mutator to debug.
    MutatorType mutator_type = MutatorType::kNone;

    /// The seed that was used to initialize the mutator.
    uint32_t seed = 0;

    /// The binary that triggered a bug in the mutator.
    std::vector<uint32_t> original_binary;

    /// Parameters for various mutators.
    MutatorCliParams mutator_params;
};

/// Parses CLI parameters for the fuzzer. This function exits with an error code
/// and a message is printed to the console if some parameter has invalid
/// format. You can pass `-tint_help` to check out all available parameters.
/// This function will remove recognized parameters from the `argv` and adjust
/// the `argc` accordingly.
///
/// @param argc - the number of parameters (identical to the `argc` in `main`
///     function).
/// @param argv - array of C strings of parameters.
/// @return the parsed parameters.
FuzzerCliParams ParseFuzzerCliParams(int* argc, char** argv);

/// Parses CLI parameters for the mutator debugger. This function exits with an
/// error code and a message is printed to the console if some parameter has
/// invalid format. You can pass `--help` to check out all available parameters.
///
/// @param argc - the number of parameters (identical to the `argc` in `main`
///     function).
/// @param argv - array of C strings of parameters.
/// @return the parsed parameters.
MutatorDebuggerCliParams ParseMutatorDebuggerCliParams(int argc, const char* const* argv);

}  // namespace tint::fuzzers::spvtools_fuzzer

#endif  // SRC_TINT_FUZZERS_TINT_SPIRV_TOOLS_FUZZER_CLI_H_
