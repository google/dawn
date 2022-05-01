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

#ifndef SRC_TINT_FUZZERS_TINT_SPIRV_TOOLS_FUZZER_SPIRV_REDUCE_MUTATOR_H_
#define SRC_TINT_FUZZERS_TINT_SPIRV_TOOLS_FUZZER_SPIRV_REDUCE_MUTATOR_H_

#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "src/tint/fuzzers/random_generator.h"
#include "src/tint/fuzzers/tint_spirv_tools_fuzzer/mutator.h"

#include "source/reduce/reduction_opportunity_finder.h"

namespace tint::fuzzers::spvtools_fuzzer {

/// Mutates SPIR-V binary by running spirv-reduce tool.
///
/// The initial `binary` must be valid according to `target_env`. Applies at
/// most `reductions_batch_size` reductions at a time. This parameter is ignored
/// if its value is 0. Uses a random subset of reduction opportunity finders by
/// default. This can be overridden with the `enable_all_reductions` parameter.
class SpirvReduceMutator : public Mutator {
  public:
    /// Constructor.
    /// @param target_env - the target environment for the `binary`.
    /// @param binary - SPIR-V binary. Must be valid.
    /// @param seed - the seed for the RNG.
    /// @param reductions_batch_size - the number of reduction passes that will be
    ///     applied during a single call to `Mutate`. If it's equal to 0 then we
    ///     apply the passes until we reach the threshold for the total number of
    ///     applied passes.
    /// @param enable_all_reductions - whether to use all reduction passes or only
    ///     a randomly selected subset of them.
    /// @param validate_after_each_reduction - whether to validate after each
    ///     applied reduction.
    SpirvReduceMutator(spv_target_env target_env,
                       std::vector<uint32_t> binary,
                       uint32_t seed,
                       uint32_t reductions_batch_size,
                       bool enable_all_reductions,
                       bool validate_after_each_reduction);

    Result Mutate() override;
    std::vector<uint32_t> GetBinary() const override;
    void LogErrors(const std::string* path, uint32_t count) const override;
    std::string GetErrors() const override;

  private:
    template <typename T, typename... Args>
    void MaybeAddFinder(Args&&... args) {
        if (enable_all_reductions_ || generator_.GetBool()) {
            finders_.push_back(std::make_unique<T>(std::forward<Args>(args)...));
        }
    }

    template <typename T>
    T* GetRandomElement(std::vector<T>* arr) {
        assert(!arr->empty() && "Can't get random element from an empty vector");
        auto index = generator_.GetUInt32(static_cast<uint32_t>(arr->size()));
        return &(*arr)[index];
    }

    template <typename T>
    T* GetRandomElement(std::vector<std::unique_ptr<T>>* arr) {
        assert(!arr->empty() && "Can't get random element from an empty vector");
        auto index = generator_.GetUInt32(static_cast<uint32_t>(arr->size()));
        return (*arr)[index].get();
    }

    bool ApplyReduction(spvtools::reduce::ReductionOpportunity* reduction_opportunity);

    // The SPIR-V binary that is being reduced.
    std::unique_ptr<spvtools::opt::IRContext> ir_context_;

    // The selected subset of reduction opportunity finders.
    std::vector<std::unique_ptr<spvtools::reduce::ReductionOpportunityFinder>> finders_;

    // Random number generator initialized with `seed_`.
    RandomGenerator generator_;

    // All the errors produced by the reducer.
    std::stringstream errors_;

    // Whether the last call to the `Mutate` method produced the valid binary.
    bool is_valid_;

    // The number of reductions to apply on a single call to `Mutate`.
    const uint32_t reductions_batch_size_;

    // The total number of applied reductions.
    uint32_t total_applied_reductions_;

    // Whether we want to use all the reduction opportunity finders and not just a
    // subset of them.
    const bool enable_all_reductions_;

    // Whether we want to validate all the binary after each reduction.
    const bool validate_after_each_reduction_;

    // The original binary that was used to initialize this mutator.
    // Useful for debugging.
    const std::vector<uint32_t> original_binary_;

    // The seed that was used to initialize the random number generator.
    // Useful for debugging.
    const uint32_t seed_;
};

}  // namespace tint::fuzzers::spvtools_fuzzer

#endif  // SRC_TINT_FUZZERS_TINT_SPIRV_TOOLS_FUZZER_SPIRV_REDUCE_MUTATOR_H_
