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

#ifndef SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATIONS_REPLACE_IDENTIFIER_H_
#define SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATIONS_REPLACE_IDENTIFIER_H_

#include "src/tint/fuzzers/tint_ast_fuzzer/mutation.h"

#include "src/tint/lang/wgsl/sem/variable.h"

namespace tint::fuzzers::ast_fuzzer {

/// @see MutationReplaceIdentifier::Apply
class MutationReplaceIdentifier : public Mutation {
  public:
    /// @brief Constructs an instance of this mutation from a protobuf message.
    /// @param message - protobuf message
    explicit MutationReplaceIdentifier(protobufs::MutationReplaceIdentifier message);

    /// @brief Constructor.
    /// @param use_id - the id of a variable user.
    /// @param replacement_id - the id of a variable to replace the `use_id`.
    MutationReplaceIdentifier(uint32_t use_id, uint32_t replacement_id);

    /// @copybrief Mutation::IsApplicable
    ///
    /// The mutation is applicable iff:
    /// - `use_id` is a valid id of an `ast::IdentifierExpression`, that
    ///   references a variable.
    /// - `replacement_id` is a valid id of an `ast::Variable`.
    /// - The identifier expression doesn't reference the variable of a
    ///   `replacement_id`.
    /// - The variable with `replacement_id` is in scope of an identifier
    ///   expression with `use_id`.
    /// - The identifier expression and the variable have the same type.
    ///
    /// @copydetails Mutation::IsApplicable
    bool IsApplicable(const tint::Program& program, const NodeIdMap& node_id_map) const override;

    /// @copybrief Mutation::Apply
    ///
    /// Replaces the use of an identifier expression with `use_id` with a newly
    /// created identifier expression, that references a variable with
    /// `replacement_id`. The newly created identifier expression will have the
    /// same id as the old one (i.e. `use_id`).
    ///
    /// @copydetails Mutation::Apply
    void Apply(const NodeIdMap& node_id_map,
               tint::program::CloneContext& clone_context,
               NodeIdMap* new_node_id_map) const override;

    protobufs::Mutation ToMessage() const override;

  private:
    protobufs::MutationReplaceIdentifier message_;
};

}  // namespace tint::fuzzers::ast_fuzzer

#endif  // SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATIONS_REPLACE_IDENTIFIER_H_
