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

#ifndef FUZZERS_TINT_SPIRV_TOOLS_FUZZER_CLI_H_
#define FUZZERS_TINT_SPIRV_TOOLS_FUZZER_CLI_H_

#include <string>
#include <vector>

#include "source/fuzz/fuzzer.h"

namespace tint {
namespace fuzzers {
namespace spvtools_fuzzer {

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
  spv_target_env target_env = kDefaultTargetEnv;
  uint32_t transformation_batch_size = 3;
  uint32_t reduction_batch_size = 3;
  uint32_t opt_batch_size = 6;
  std::vector<spvtools::fuzz::fuzzerutil::ModuleSupplier> donors = {};
  spvtools::fuzz::RepeatedPassStrategy repeated_pass_strategy =
      spvtools::fuzz::RepeatedPassStrategy::kSimple;
  bool enable_all_fuzzer_passes = false;
  bool enable_all_reduce_passes = false;
  bool validate_after_each_opt_pass = true;
  bool validate_after_each_fuzzer_pass = true;
  bool validate_after_each_reduce_pass = true;
};

/// Parameters specific to the fuzzer.
struct FuzzerCliParams {
  uint32_t mutator_cache_size = 20;
  MutatorType mutator_type = MutatorType::kAll;
  FuzzingTarget fuzzing_target = FuzzingTarget::kAll;
  std::string error_dir;
  MutatorCliParams mutator_params;
};

/// Parameters specific to the mutator debugger.
struct MutatorDebuggerCliParams {
  MutatorType mutator_type = MutatorType::kNone;
  uint32_t seed = 0;
  std::vector<uint32_t> original_binary;
  MutatorCliParams mutator_params;
};

/// Parses CLI parameters for the fuzzer. This function exits with an error code
/// and a message is printed to the console if some parameter has invalid
/// format. You can pass `--help` to check out all available parameters.
///
/// @param argc - the number of parameters (identical to the `argc` in `main`
///     function).
/// @param argv - array of C strings of parameters.
/// @return the parsed parameters.
FuzzerCliParams ParseFuzzerCliParams(int argc, const char* const* argv);

/// Parses CLI parameters for the mutator debugger. This function exits with an
/// error code and a message is printed to the console if some parameter has
/// invalid format. You can pass `--help` to check out all available parameters.
///
/// @param argc - the number of parameters (identical to the `argc` in `main`
///     function).
/// @param argv - array of C strings of parameters.
/// @return the parsed parameters.
MutatorDebuggerCliParams ParseMutatorDebuggerCliParams(int argc,
                                                       const char* const* argv);

}  // namespace spvtools_fuzzer
}  // namespace fuzzers
}  // namespace tint

#endif  // FUZZERS_TINT_SPIRV_TOOLS_FUZZER_CLI_H_
