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

#include "src/tint/fuzzers/tint_spirv_tools_fuzzer/spirv_reduce_mutator.h"

#include <fstream>

#include "source/fuzz/fuzzer_util.h"
#include "source/opt/build_module.h"
#include "source/reduce/conditional_branch_to_simple_conditional_branch_opportunity_finder.h"
#include "source/reduce/merge_blocks_reduction_opportunity_finder.h"
#include "source/reduce/operand_to_const_reduction_opportunity_finder.h"
#include "source/reduce/operand_to_dominating_id_reduction_opportunity_finder.h"
#include "source/reduce/operand_to_undef_reduction_opportunity_finder.h"
#include "source/reduce/remove_block_reduction_opportunity_finder.h"
#include "source/reduce/remove_function_reduction_opportunity_finder.h"
#include "source/reduce/remove_selection_reduction_opportunity_finder.h"
#include "source/reduce/remove_unused_instruction_reduction_opportunity_finder.h"
#include "source/reduce/remove_unused_struct_member_reduction_opportunity_finder.h"
#include "source/reduce/simple_conditional_branch_to_branch_opportunity_finder.h"
#include "source/reduce/structured_loop_to_selection_reduction_opportunity_finder.h"
#include "src/tint/fuzzers/tint_spirv_tools_fuzzer/util.h"

namespace tint::fuzzers::spvtools_fuzzer {

SpirvReduceMutator::SpirvReduceMutator(spv_target_env target_env,
                                       std::vector<uint32_t> binary,
                                       uint32_t seed,
                                       uint32_t reductions_batch_size,
                                       bool enable_all_reductions,
                                       bool validate_after_each_reduction)
    : ir_context_(nullptr),
      finders_(),
      generator_(seed),
      errors_(),
      is_valid_(true),
      reductions_batch_size_(reductions_batch_size),
      total_applied_reductions_(0),
      enable_all_reductions_(enable_all_reductions),
      validate_after_each_reduction_(validate_after_each_reduction),
      original_binary_(std::move(binary)),
      seed_(seed) {
    ir_context_ =
        spvtools::BuildModule(target_env, spvtools::fuzz::fuzzerutil::kSilentMessageConsumer,
                              original_binary_.data(), original_binary_.size());
    assert(ir_context_ && "|binary| is invalid");

    do {
        MaybeAddFinder<
            spvtools::reduce::ConditionalBranchToSimpleConditionalBranchOpportunityFinder>();
        MaybeAddFinder<spvtools::reduce::MergeBlocksReductionOpportunityFinder>();
        MaybeAddFinder<spvtools::reduce::OperandToConstReductionOpportunityFinder>();
        MaybeAddFinder<spvtools::reduce::OperandToDominatingIdReductionOpportunityFinder>();
        MaybeAddFinder<spvtools::reduce::OperandToUndefReductionOpportunityFinder>();
        MaybeAddFinder<spvtools::reduce::RemoveBlockReductionOpportunityFinder>();
        MaybeAddFinder<spvtools::reduce::RemoveFunctionReductionOpportunityFinder>();
        MaybeAddFinder<spvtools::reduce::RemoveSelectionReductionOpportunityFinder>();
        MaybeAddFinder<spvtools::reduce::RemoveUnusedInstructionReductionOpportunityFinder>(true);
        MaybeAddFinder<spvtools::reduce::RemoveUnusedStructMemberReductionOpportunityFinder>();
        MaybeAddFinder<spvtools::reduce::SimpleConditionalBranchToBranchOpportunityFinder>();
        MaybeAddFinder<spvtools::reduce::StructuredLoopToSelectionReductionOpportunityFinder>();
    } while (finders_.empty());
}

Mutator::Result SpirvReduceMutator::Mutate() {
    assert(is_valid_ && "Can't mutate invalid module");

    // The upper limit on the number of applied reduction passes.
    const uint32_t kMaxAppliedReductions = 500;
    const auto old_applied_reductions = total_applied_reductions_;

    // The upper limit on the number of failed attempts to apply reductions (i.e.
    // when no reduction was returned by the reduction finder).
    const uint32_t kMaxConsecutiveFailures = 10;
    uint32_t num_consecutive_failures = 0;

    // Iterate while we haven't exceeded the limit on the total number of applied
    // reductions, the limit on the number of reductions applied at once and limit
    // on the number of consecutive failed attempts.
    while (total_applied_reductions_ < kMaxAppliedReductions &&
           (reductions_batch_size_ == 0 ||
            total_applied_reductions_ - old_applied_reductions < reductions_batch_size_) &&
           num_consecutive_failures < kMaxConsecutiveFailures) {
        // Select an opportunity finder and get some reduction opportunities from
        // it.
        auto finder = GetRandomElement(&finders_);
        auto reduction_opportunities = finder->GetAvailableOpportunities(ir_context_.get(), 0);

        if (reduction_opportunities.empty()) {
            // There is nothing to reduce. We increase the counter to make sure we
            // don't stuck in this situation.
            num_consecutive_failures++;
        } else {
            // Apply a random reduction opportunity. The latter should be applicable.
            auto opportunity = GetRandomElement(&reduction_opportunities);
            assert(opportunity->PreconditionHolds() && "Preconditions should hold");
            total_applied_reductions_++;
            num_consecutive_failures = 0;
            if (!ApplyReduction(opportunity)) {
                // The module became invalid as a result of the applied reduction.
                is_valid_ = false;
                return {Mutator::Status::kInvalid,
                        total_applied_reductions_ != old_applied_reductions};
            }
        }
    }

    auto is_changed = total_applied_reductions_ != old_applied_reductions;
    if (total_applied_reductions_ == kMaxAppliedReductions) {
        return {Mutator::Status::kLimitReached, is_changed};
    }

    if (num_consecutive_failures == kMaxConsecutiveFailures) {
        return {Mutator::Status::kStuck, is_changed};
    }

    assert(is_changed && "This is the only way left to break the loop");
    return {Mutator::Status::kComplete, is_changed};
}

bool SpirvReduceMutator::ApplyReduction(
    spvtools::reduce::ReductionOpportunity* reduction_opportunity) {
    reduction_opportunity->TryToApply();
    return !validate_after_each_reduction_ || spvtools::fuzz::fuzzerutil::IsValidAndWellFormed(
                                                  ir_context_.get(), spvtools::ValidatorOptions(),
                                                  util::GetBufferMessageConsumer(&errors_));
}

std::vector<uint32_t> SpirvReduceMutator::GetBinary() const {
    std::vector<uint32_t> result;
    ir_context_->module()->ToBinary(&result, true);
    return result;
}

std::string SpirvReduceMutator::GetErrors() const {
    return errors_.str();
}

void SpirvReduceMutator::LogErrors(const std::string* path, uint32_t count) const {
    auto message = GetErrors();
    std::cout << count << " | SpirvReduceMutator (seed: " << seed_ << ")" << std::endl;
    std::cout << message << std::endl;

    if (path) {
        auto prefix = *path + std::to_string(count);

        // Write errors to file.
        std::ofstream(prefix + ".reducer.log") << "seed: " << seed_ << std::endl
                                               << message << std::endl;

        // Write the invalid SPIR-V binary.
        util::WriteBinary(prefix + ".reducer.invalid.spv", GetBinary());

        // Write the original SPIR-V binary.
        util::WriteBinary(prefix + ".reducer.original.spv", original_binary_);
    }
}

}  // namespace tint::fuzzers::spvtools_fuzzer
