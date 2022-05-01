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

#ifndef SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATOR_H_
#define SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATOR_H_

#include "src/tint/fuzzers/tint_ast_fuzzer/mutation.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/mutation_finder.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/node_id_map.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/probability_context.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/protobufs/tint_ast_fuzzer.h"

#include "src/tint/program.h"

namespace tint::fuzzers::ast_fuzzer {
/// @file

/// @brief Tries to apply a `mutation` to the `program`.
///
/// If the `mutation` is inapplicable, this function will return `false` and
/// `out_program`, `out_node_id_map` and `mutation_sequence` won't be modified.
///
/// The `mutation` is required to produce a valid program when the
/// `Mutation::Apply` method is called. This guarantees that this function
/// returns a valid program as well.
///
/// @param program - the initial program (must be valid).
/// @param mutation - the mutation that will be applied.
/// @param node_id_map - a map from `tint::ast::` nodes in the `program` to
///     their unique ids.
/// @param out_program - the resulting mutated program will be written through
///     this pointer. It may *not* be a `nullptr`. It _may_ point to `program`,
///     so that a program can be updated in place.
/// @param out_node_id_map - will contain new ids for the AST nodes in the
///     mutated program. It may *not* be a `nullptr`. It _may_ point to
///     `node_id_map`, so that a map can be updated in place.
/// @param mutation_sequence - the message about this mutation will be recorded
///     here. It may be a `nullptr`, in which case it's ignored.
/// @return `true` if the `mutation` was applied.
/// @return `false` if the `mutation` is inapplicable.
bool MaybeApplyMutation(const tint::Program& program,
                        const Mutation& mutation,
                        const NodeIdMap& node_id_map,
                        tint::Program* out_program,
                        NodeIdMap* out_node_id_map,
                        protobufs::MutationSequence* mutation_sequence);

/// @brief Applies mutations from `mutations_sequence` to the `program`.
///
/// All mutations in `mutation_sequence` must be applicable. Additionally, all
/// mutations must produce a valid program when the `Mutation::Apply` method is
/// called. This guarantees that this function returns a valid program as well.
///
/// @param program - the initial program - must be valid.
/// @param mutation_sequence - a sequence of mutations.
/// @return the mutated program.
tint::Program Replay(tint::Program program, const protobufs::MutationSequence& mutation_sequence);

/// @brief Applies up to `max_applied_mutations` mutations to the `program`.
///
/// All applied mutations must produce valid programs. This guarantees that the
/// returned program is valid as well. The returned program may be identical to
/// the initial `program` if no mutation was applied.
///
/// @param program - initial program - must be valid.
/// @param probability_context - contains information about various
///     probabilistic behaviour of the fuzzer.
/// @param enable_all_mutations - if `false`, only mutations from a
///     probabilistically selected set of mutation types are applied. If `true`,
///     all mutation types are considered.
/// @param max_applied_mutations - the maximum number of applied mutations. This
///     may not be 0.
/// @param mutation_sequence - applied mutations will be recorded into this
///     protobuf message. This argument may be `nullptr`, in which case it's
///     ignored.
/// @return the mutated program.
tint::Program Mutate(tint::Program program,
                     ProbabilityContext* probability_context,
                     bool enable_all_mutations,
                     uint32_t max_applied_mutations,
                     protobufs::MutationSequence* mutation_sequence);

}  // namespace tint::fuzzers::ast_fuzzer

#endif  // SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATOR_H_
