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

#ifndef SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATIONS_DELETE_STATEMENT_H_
#define SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATIONS_DELETE_STATEMENT_H_

#include "src/tint/ast/statement.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/jump_tracker.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/mutation.h"

namespace tint::fuzzers::ast_fuzzer {

/// @see MutationDeleteStatement::Apply
class MutationDeleteStatement : public Mutation {
  public:
    /// @brief Constructs an instance of this mutation from a protobuf message.
    /// @param message - protobuf message
    explicit MutationDeleteStatement(protobufs::MutationDeleteStatement message);

    /// @brief Constructor.
    /// @param statement_id - the id of the statement to delete.
    explicit MutationDeleteStatement(uint32_t statement_id);

    /// @copybrief Mutation::IsApplicable
    ///
    /// The mutation is applicable iff:
    /// - `statement_id` corresponds to a statement in the AST.
    /// - `statement_id` does not refer to a variable declaration, since the declared variables will
    ///   be inaccessible if the statement is deleted.
    /// - `statement_id` is not a return statement, since removing return statements arbitrarily can
    ///   make the program invalid.
    /// - `statement_id` is not a break statement, since removing break statements can lead to
    ///   syntactically infinite loops.
    ///
    /// @copydetails Mutation::IsApplicable
    bool IsApplicable(const tint::Program& program, const NodeIdMap& node_id_map) const override;

    /// @copybrief Mutation::Apply
    ///
    /// Delete the statement referenced by `statement_id`.
    ///
    /// @copydetails Mutation::Apply
    void Apply(const NodeIdMap& node_id_map,
               tint::CloneContext* clone_context,
               NodeIdMap* new_node_id_map) const override;

    protobufs::Mutation ToMessage() const override;

    /// Return whether the given statement is suitable for deletion.
    /// @param statement_node - the statement to be considered for deletion.
    /// @param program - the program containing  the statement.
    /// @param jump_tracker - information about jump statements for the program.
    /// @return true if and only if it is OK to delete the statement.
    static bool CanBeDeleted(const ast::Statement& statement_node,
                             const Program& program,
                             const JumpTracker& jump_tracker);

  private:
    protobufs::MutationDeleteStatement message_;
};

}  // namespace tint::fuzzers::ast_fuzzer

#endif  // SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATIONS_DELETE_STATEMENT_H_
