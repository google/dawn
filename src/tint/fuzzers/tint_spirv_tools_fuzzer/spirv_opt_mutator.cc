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

#include "src/tint/fuzzers/tint_spirv_tools_fuzzer/spirv_opt_mutator.h"

#include <fstream>
#include <iostream>
#include <unordered_set>
#include <utility>

#include "spirv-tools/optimizer.hpp"
#include "src/tint/fuzzers/tint_spirv_tools_fuzzer/util.h"

namespace tint::fuzzers::spvtools_fuzzer {

SpirvOptMutator::SpirvOptMutator(spv_target_env target_env,
                                 uint32_t seed,
                                 std::vector<uint32_t> binary,
                                 bool validate_after_each_opt,
                                 uint32_t opt_batch_size)
    : num_executions_(0),
      is_valid_(true),
      target_env_(target_env),
      original_binary_(std::move(binary)),
      seed_(seed),
      opt_passes_({"--combine-access-chains",
                   "--loop-unroll",
                   "--merge-blocks",
                   "--cfg-cleanup",
                   "--eliminate-dead-functions",
                   "--merge-return",
                   "--wrap-opkill",
                   "--eliminate-dead-code-aggressive",
                   "--if-conversion",
                   "--eliminate-local-single-store",
                   "--eliminate-local-single-block",
                   "--eliminate-dead-branches",
                   "--scalar-replacement=0",
                   "--eliminate-dead-inserts",
                   "--eliminate-dead-members",
                   "--simplify-instructions",
                   "--private-to-local",
                   "--ssa-rewrite",
                   "--ccp",
                   "--reduce-load-size",
                   "--vector-dce",
                   "--scalar-replacement=100",
                   "--inline-entry-points-exhaustive",
                   "--redundancy-elimination",
                   "--convert-local-access-chains",
                   "--copy-propagate-arrays",
                   "--fix-storage-class"}),
      optimized_binary_(),
      validate_after_each_opt_(validate_after_each_opt),
      opt_batch_size_(opt_batch_size),
      generator_(seed) {
    assert(spvtools::SpirvTools(target_env).Validate(original_binary_) &&
           "Initial binary is invalid");
    assert(!opt_passes_.empty() && "Must be at least one pass");
}

SpirvOptMutator::Result SpirvOptMutator::Mutate() {
    assert(is_valid_ && "The optimizer is not longer valid");

    const uint32_t kMaxNumExecutions = 100;
    const uint32_t kMaxNumStuck = 10;

    if (num_executions_ == kMaxNumExecutions) {
        // We've applied this mutator many times already. Indicate to the user that
        // it might be better to try a different mutator.
        return {Status::kLimitReached, false};
    }

    num_executions_++;

    // Get the input binary. If this is the first time we run this mutator, use
    // the `original_binary_`. Otherwise, one of the following will be true:
    // - the `optimized_binary_` is not empty.
    // - the previous call to the `Mutate` method returned `kStuck`.
    auto binary = num_executions_ == 1 ? original_binary_ : optimized_binary_;
    optimized_binary_.clear();

    assert(!binary.empty() && "Can't run the optimizer on an empty binary");

    // Number of times spirv-opt wasn't able to produce any new result.
    uint32_t num_stuck = 0;
    do {
        // Randomly select `opt_batch_size` optimization passes. If `opt_batch_size`
        // is equal to 0, we will use the number of passes equal to the number of
        // all available passes.
        auto num_of_passes = opt_batch_size_ ? opt_batch_size_ : opt_passes_.size();
        std::vector<std::string> passes;

        while (passes.size() < num_of_passes) {
            auto idx = generator_.GetUInt32(static_cast<uint32_t>(opt_passes_.size()));
            passes.push_back(opt_passes_[idx]);
        }

        // Run the `binary` into the `optimized_binary_`.
        spvtools::Optimizer optimizer(target_env_);
        optimizer.SetMessageConsumer(util::GetBufferMessageConsumer(&errors_));
        optimizer.SetValidateAfterAll(validate_after_each_opt_);
        optimizer.RegisterPassesFromFlags(passes);
        if (!optimizer.Run(binary.data(), binary.size(), &optimized_binary_)) {
            is_valid_ = false;
            return {Status::kInvalid, true};
        }
    } while (optimized_binary_.empty() && ++num_stuck < kMaxNumStuck);

    return {optimized_binary_.empty() ? Status::kStuck : Status::kComplete,
            !optimized_binary_.empty()};
}

std::vector<uint32_t> SpirvOptMutator::GetBinary() const {
    return optimized_binary_;
}

std::string SpirvOptMutator::GetErrors() const {
    return errors_.str();
}

void SpirvOptMutator::LogErrors(const std::string* path, uint32_t count) const {
    auto message = GetErrors();
    std::cout << count << " | SpirvOptMutator (seed: " << seed_ << ")" << std::endl;
    std::cout << message << std::endl;

    if (path) {
        auto prefix = *path + std::to_string(count);

        // Write errors to file.
        std::ofstream(prefix + ".opt.log") << "seed: " << seed_ << std::endl
                                           << message << std::endl;

        // Write the invalid SPIR-V binary.
        util::WriteBinary(prefix + ".opt.invalid.spv", optimized_binary_);

        // Write the original SPIR-V binary.
        util::WriteBinary(prefix + ".opt.original.spv", original_binary_);
    }
}

}  // namespace tint::fuzzers::spvtools_fuzzer
