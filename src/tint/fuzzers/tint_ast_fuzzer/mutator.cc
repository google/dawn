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
#include "src/tint/program_builder.h"

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
                        tint::Program* out_program,
                        NodeIdMap* out_node_id_map,
                        protobufs::MutationSequence* mutation_sequence) {
    assert(out_program && "`out_program` may not be a nullptr");
    assert(out_node_id_map && "`out_node_id_map` may not be a nullptr");

    if (!mutation.IsApplicable(program, node_id_map)) {
        return false;
    }

    // The mutated `program` will be copied into the `mutated` program builder.
    tint::ProgramBuilder mutated;
    tint::CloneContext clone_context(&mutated, &program);
    NodeIdMap new_node_id_map;
    clone_context.ReplaceAll(
        [&node_id_map, &new_node_id_map, &clone_context](const ast::Node* node) {
            // Make sure all `tint::ast::` nodes' ids are preserved.
            auto* cloned = tint::As<ast::Node>(node->Clone(&clone_context));
            new_node_id_map.Add(cloned, node_id_map.GetId(node));
            return cloned;
        });

    mutation.Apply(node_id_map, &clone_context, &new_node_id_map);
    if (mutation_sequence) {
        *mutation_sequence->add_mutation() = mutation.ToMessage();
    }

    clone_context.Clone();
    *out_program = tint::Program(std::move(mutated));
    *out_node_id_map = std::move(new_node_id_map);
    return true;
}

tint::Program Replay(tint::Program program, const protobufs::MutationSequence& mutation_sequence) {
    assert(program.IsValid() && "Initial program is invalid");

    NodeIdMap node_id_map(program);
    for (const auto& mutation_message : mutation_sequence.mutation()) {
        auto mutation = Mutation::FromMessage(mutation_message);
        auto status =
            MaybeApplyMutation(program, *mutation, node_id_map, &program, &node_id_map, nullptr);
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

            if (!MaybeApplyMutation(program, *mutation, node_id_map, &program, &node_id_map,
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
