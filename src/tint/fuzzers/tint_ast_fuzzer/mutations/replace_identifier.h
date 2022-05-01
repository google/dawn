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

#ifndef SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATIONS_REPLACE_IDENTIFIER_H_
#define SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATIONS_REPLACE_IDENTIFIER_H_

#include "src/tint/fuzzers/tint_ast_fuzzer/mutation.h"

#include "src/tint/sem/variable.h"

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
               tint::CloneContext* clone_context,
               NodeIdMap* new_node_id_map) const override;

    protobufs::Mutation ToMessage() const override;

  private:
    protobufs::MutationReplaceIdentifier message_;
};

}  // namespace tint::fuzzers::ast_fuzzer

#endif  // SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATIONS_REPLACE_IDENTIFIER_H_
