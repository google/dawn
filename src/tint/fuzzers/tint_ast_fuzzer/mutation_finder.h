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

#ifndef SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATION_FINDER_H_
#define SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATION_FINDER_H_

#include <memory>
#include <vector>

#include "src/tint/fuzzers/tint_ast_fuzzer/mutation.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/node_id_map.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/probability_context.h"

#include "src/tint/program.h"

namespace tint::fuzzers::ast_fuzzer {

/// Instances of this class traverse the `tint::Program`, looking for
/// opportunities to apply mutations and return them to the caller.
///
/// Ideally, the behaviour of this class (precisely, its `FindMutations` method)
/// should not be probabilistic. This is useful when mutation finders are used
/// for test case reduction, because it enables the test case reducer to
/// systematically explore all available mutations. There may be some
/// exceptions, however. For example, if a huge number of mutations is returned,
/// it would make sense to apply only a probabilistically selected subset of
/// them.
class MutationFinder {
  public:
    /// Virtual destructor.
    virtual ~MutationFinder();

    /// @brief Traverses the `program`, looking for opportunities to apply
    /// mutations.
    ///
    /// @param program - the program being fuzzed.
    /// @param node_id_map - a map from `tint::ast::` nodes in the `program` to
    ///     their unique ids.
    /// @param probability_context - determines various probabilistic stuff in the
    ///     mutator. This should ideally be used as less as possible.
    /// @return all the found mutations.
    virtual MutationList FindMutations(const tint::Program& program,
                                       NodeIdMap* node_id_map,
                                       ProbabilityContext* probability_context) const = 0;

    /// @brief Compute a probability of applying a single mutation, returned by
    /// this class.
    ///
    /// @param probability_context - contains information about various
    ///     non-deterministic stuff in the fuzzer.
    /// @return a number in the range [0; 100] which is a chance of applying a
    ///     mutation.
    virtual uint32_t GetChanceOfApplyingMutation(ProbabilityContext* probability_context) const = 0;
};

using MutationFinderList = std::vector<std::unique_ptr<MutationFinder>>;

}  // namespace tint::fuzzers::ast_fuzzer

#endif  // SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATION_FINDER_H_
