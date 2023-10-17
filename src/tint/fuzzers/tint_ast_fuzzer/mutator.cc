// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/fuzzers/tint_ast_fuzzer/mutator.h"

#include <cassert>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/tint/fuzzers/tint_ast_fuzzer/mutation_finders/change_binary_operators.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/mutation_finders/change_unary_operators.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/mutation_finders/delete_statements.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/mutation_finders/replace_identifiers.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/mutation_finders/wrap_unary_operators.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/node_id_map.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"

namespace tint::fuzzers::ast_fuzzer {
namespace {

template <typename T, typename... Args>
void MaybeAddFinder(bool enable_all_mutations,
                    ProbabilityContext* probability_context,
                    MutationFinderList* finders,
                    Args&&... args) {
    if (enable_all_mutations || probability_context->RandomBool()) {
        finders->push_back(std::make_unique<T>(std::forward<Args>(args)...));
    }
}

MutationFinderList CreateMutationFinders(ProbabilityContext* probability_context,
                                         bool enable_all_mutations) {
    MutationFinderList result;
    do {
        MaybeAddFinder<MutationFinderChangeBinaryOperators>(enable_all_mutations,
                                                            probability_context, &result);
        MaybeAddFinder<MutationFinderChangeUnaryOperators>(enable_all_mutations,
                                                           probability_context, &result);
        MaybeAddFinder<MutationFinderDeleteStatements>(enable_all_mutations, probability_context,
                                                       &result);
        MaybeAddFinder<MutationFinderReplaceIdentifiers>(enable_all_mutations, probability_context,
                                                         &result);
        MaybeAddFinder<MutationFinderWrapUnaryOperators>(enable_all_mutations, probability_context,
                                                         &result);
    } while (result.empty());
    return result;
}

}  // namespace

bool MaybeApplyMutation(const tint::Program& program,
                        const Mutation& mutation,
                        const NodeIdMap& node_id_map,
                        tint::Program& out_program,
                        NodeIdMap* out_node_id_map,
                        protobufs::MutationSequence* mutation_sequence) {
    assert(out_node_id_map && "`out_node_id_map` may not be a nullptr");

    if (!mutation.IsApplicable(program, node_id_map)) {
        return false;
    }

    // The mutated `program` will be copied into the `mutated` program builder.
    tint::ProgramBuilder mutated;
    tint::program::CloneContext clone_context(&mutated, &program);
    NodeIdMap new_node_id_map;
    clone_context.ReplaceAll(
        [&node_id_map, &new_node_id_map, &clone_context](const ast::Node* node) {
            // Make sure all `tint::ast::` nodes' ids are preserved.
            auto* cloned = tint::As<ast::Node>(node->Clone(clone_context));
            new_node_id_map.Add(cloned, node_id_map.GetId(node));
            return cloned;
        });

    mutation.Apply(node_id_map, clone_context, &new_node_id_map);
    if (mutation_sequence) {
        *mutation_sequence->add_mutation() = mutation.ToMessage();
    }

    clone_context.Clone();
    out_program = tint::resolver::Resolve(mutated);
    *out_node_id_map = std::move(new_node_id_map);
    return true;
}

tint::Program Replay(tint::Program program, const protobufs::MutationSequence& mutation_sequence) {
    assert(program.IsValid() && "Initial program is invalid");

    NodeIdMap node_id_map(program);
    for (const auto& mutation_message : mutation_sequence.mutation()) {
        auto mutation = Mutation::FromMessage(mutation_message);
        auto status =
            MaybeApplyMutation(program, *mutation, node_id_map, program, &node_id_map, nullptr);
        (void)status;  // `status` will be unused in release mode.
        assert(status && "`mutation` is inapplicable - it's most likely a bug");
        if (!program.IsValid()) {
            // `mutation` has a bug.
            break;
        }
    }

    return program;
}

tint::Program Mutate(tint::Program program,
                     ProbabilityContext* probability_context,
                     bool enable_all_mutations,
                     uint32_t max_applied_mutations,
                     protobufs::MutationSequence* mutation_sequence) {
    assert(max_applied_mutations != 0 && "Maximum number of mutations is invalid");
    assert(program.IsValid() && "Initial program is invalid");

    // The number of allowed failed attempts to apply mutations. If this number is
    // exceeded, the mutator is considered stuck and the mutation session is
    // stopped.
    const uint32_t kMaxFailureToApply = 10;

    auto finders = CreateMutationFinders(probability_context, enable_all_mutations);
    NodeIdMap node_id_map(program);

    // Total number of applied mutations during this call to `Mutate`.
    uint32_t applied_mutations = 0;

    // The number of consecutively failed attempts to apply mutations.
    uint32_t failure_to_apply = 0;

    // Apply mutations as long as the `program` is valid, the limit on the number
    // of mutations is not reached and the mutator is not stuck (i.e. unable to
    // apply any mutations for some time).
    while (program.IsValid() && applied_mutations < max_applied_mutations &&
           failure_to_apply < kMaxFailureToApply) {
        // Get all applicable mutations from some mutation finder.
        const auto& mutation_finder = finders[probability_context->GetRandomIndex(finders)];
        auto mutations = mutation_finder->FindMutations(program, &node_id_map, probability_context);

        const auto old_applied_mutations = applied_mutations;
        for (const auto& mutation : mutations) {
            if (!probability_context->ChoosePercentage(
                    mutation_finder->GetChanceOfApplyingMutation(probability_context))) {
                // Skip this `mutation` probabilistically.
                continue;
            }

            if (!MaybeApplyMutation(program, *mutation, node_id_map, program, &node_id_map,
                                    mutation_sequence)) {
                // This `mutation` is inapplicable. This may happen if some of the
                // earlier mutations cancelled this one.
                continue;
            }

            applied_mutations++;
            if (!program.IsValid()) {
                // This `mutation` has a bug.
                return program;
            }
        }

        if (old_applied_mutations == applied_mutations) {
            // No mutation was applied. Increase the counter to prevent an infinite
            // loop.
            failure_to_apply++;
        } else {
            failure_to_apply = 0;
        }
    }

    return program;
}

}  // namespace tint::fuzzers::ast_fuzzer
