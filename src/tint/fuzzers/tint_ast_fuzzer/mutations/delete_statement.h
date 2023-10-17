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

#ifndef SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATIONS_DELETE_STATEMENT_H_
#define SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATIONS_DELETE_STATEMENT_H_

#include "src/tint/fuzzers/tint_ast_fuzzer/jump_tracker.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/mutation.h"
#include "src/tint/lang/wgsl/ast/statement.h"

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
               tint::program::CloneContext& clone_context,
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
