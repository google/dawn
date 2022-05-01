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

#ifndef SRC_TINT_FUZZERS_TINT_SPIRV_TOOLS_FUZZER_SPIRV_OPT_MUTATOR_H_
#define SRC_TINT_FUZZERS_TINT_SPIRV_TOOLS_FUZZER_SPIRV_OPT_MUTATOR_H_

#include <sstream>
#include <string>
#include <vector>

#include "spirv-tools/libspirv.h"
#include "src/tint/fuzzers/random_generator.h"
#include "src/tint/fuzzers/tint_spirv_tools_fuzzer/mutator.h"

namespace tint::fuzzers::spvtools_fuzzer {

/// Mutates the SPIR-V module using the spirv-opt tool.
///
/// The initial `binary` must be valid according to `target_env`. On each call
/// to the `Mutate` method the mutator selects `opt_batch_size` random
/// optimization passes (with substitutions) and applies them to the binary.
class SpirvOptMutator : public Mutator {
  public:
    /// Constructor.
    /// @param target_env - target environment for the `binary`.
    /// @param seed - seed for the RNG.
    /// @param binary - SPIR-V binary. Must be valid.
    /// @param validate_after_each_opt - whether to validate the binary after each
    ///     optimization pass.
    /// @param opt_batch_size - the maximum number of optimization passes that
    ///     will be applied in a single call to `Mutate`. If it's equal to 0 then
    ///     all available optimization passes are applied.
    SpirvOptMutator(spv_target_env target_env,
                    uint32_t seed,
                    std::vector<uint32_t> binary,
                    bool validate_after_each_opt,
                    uint32_t opt_batch_size);

    Result Mutate() override;
    std::vector<uint32_t> GetBinary() const override;
    void LogErrors(const std::string* path, uint32_t count) const override;
    std::string GetErrors() const override;

  private:
    // Number of times this mutator was executed.
    uint32_t num_executions_;

    // Whether the last execution left it in a valid state.
    bool is_valid_;

    // Target environment for the SPIR-V binary.
    const spv_target_env target_env_;

    // The original SPIR-V binary. Useful for debugging.
    const std::vector<uint32_t> original_binary_;

    // The seed for the RNG. Useful for debugging.
    const uint32_t seed_;

    // All the optimization passes available.
    const std::vector<std::string> opt_passes_;

    // The result of the optimization.
    std::vector<uint32_t> optimized_binary_;

    // Whether we need to validate the binary after each optimization pass.
    const bool validate_after_each_opt_;

    // The number of optimization passes to apply at once.
    const uint32_t opt_batch_size_;

    // All the errors produced by the optimizer.
    std::stringstream errors_;

    // The random number generator initialized with `seed_`.
    RandomGenerator generator_;
};

}  // namespace tint::fuzzers::spvtools_fuzzer

#endif  // SRC_TINT_FUZZERS_TINT_SPIRV_TOOLS_FUZZER_SPIRV_OPT_MUTATOR_H_
