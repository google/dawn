// Copyright 2020 The Tint Authors.
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

#ifndef SRC_TINT_AST_BLOCK_STATEMENT_H_
#define SRC_TINT_AST_BLOCK_STATEMENT_H_

#include <utility>

#include "src/tint/ast/statement.h"

namespace tint::ast {

/// A block statement
class BlockStatement final : public Castable<BlockStatement, Statement> {
  public:
    /// Constructor
    /// @param program_id the identifier of the program that owns this node
    /// @param source the block statement source
    /// @param statements the statements
    BlockStatement(ProgramID program_id, const Source& source, const StatementList& statements);
    /// Move constructor
    BlockStatement(BlockStatement&&);
    ~BlockStatement() override;

    /// @returns true if the block has no statements
    bool Empty() const { return statements.empty(); }

    /// @returns the last statement in the block or nullptr if block empty
    const Statement* Last() const { return statements.empty() ? nullptr : statements.back(); }

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const BlockStatement* Clone(CloneContext* ctx) const override;

    /// the statement list
    const StatementList statements;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_BLOCK_STATEMENT_H_
