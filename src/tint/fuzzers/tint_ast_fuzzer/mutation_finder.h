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

#ifndef SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATION_FINDER_H_
#define SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATION_FINDER_H_

#include <memory>
#include <vector>

#include "src/tint/fuzzers/tint_ast_fuzzer/mutation.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/node_id_map.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/probability_context.h"

#include "src/tint/lang/wgsl/program/program.h"

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
