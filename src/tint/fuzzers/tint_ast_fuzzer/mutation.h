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

#ifndef SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATION_H_
#define SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATION_H_

#include <memory>
#include <vector>

#include "src/tint/fuzzers/tint_ast_fuzzer/node_id_map.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/protobufs/tint_ast_fuzzer.h"

#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program.h"

namespace tint::fuzzers::ast_fuzzer {

/// The base class for all the mutations in the fuzzer. Children must override
/// three methods:
/// - `IsApplicable` - checks whether it is possible to apply the mutation
///   in a manner that will lead to a valid program.
/// - `Apply` - applies the mutation.
/// - `ToMessage` - converts the mutation data into a protobuf message.
class Mutation {
  public:
    /// Virtual destructor.
    virtual ~Mutation();

    /// @brief Determines whether this mutation is applicable to the `program`.
    ///
    /// @param program - the program this mutation will be applied to. The program
    ///     must be valid.
    /// @param node_id_map - the map from `tint::ast::` nodes to their ids.
    /// @return `true` if `Apply` method can be called without breaking the
    ///     semantics of the `program`.
    /// @return `false` otherwise.
    virtual bool IsApplicable(const tint::Program& program, const NodeIdMap& node_id_map) const = 0;

    /// @brief Applies this mutation to the `clone_context`.
    ///
    /// Precondition: `IsApplicable` must return `true` when invoked on the same
    /// `node_id_map` and `clone_context.src` instance of `tint::Program`. A new
    /// `tint::Program` that arises in `clone_context` must be valid.
    ///
    /// @param node_id_map - the map from `tint::ast::` nodes to their ids.
    /// @param clone_context - the context that will clone the program with some
    ///     changes introduced by this mutation.
    /// @param new_node_id_map - this map will store ids for the mutated and
    ///     cloned program. This argument cannot be a `nullptr` nor can it point
    ///     to the same object as `node_id_map`.
    virtual void Apply(const NodeIdMap& node_id_map,
                       tint::program::CloneContext& clone_context,
                       NodeIdMap* new_node_id_map) const = 0;

    /// @return a protobuf message for this mutation.
    virtual protobufs::Mutation ToMessage() const = 0;

    /// @brief Converts a protobuf message into the mutation instance.
    ///
    /// @param message - a protobuf message.
    /// @return the instance of this class.
    static std::unique_ptr<Mutation> FromMessage(const protobufs::Mutation& message);
};

using MutationList = std::vector<std::unique_ptr<Mutation>>;

}  // namespace tint::fuzzers::ast_fuzzer

#endif  // SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATION_H_
