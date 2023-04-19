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

// Forward declarations
namespace tint::ast {
class Attribute;
}  // namespace tint::ast

namespace tint::ast {

/// A block statement
class BlockStatement final : public utils::Castable<BlockStatement, Statement> {
  public:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param source the block statement source
    /// @param statements the statements
    /// @param attributes the block statement attributes
    BlockStatement(ProgramID pid,
                   NodeID nid,
                   const Source& source,
                   utils::VectorRef<const Statement*> statements,
                   utils::VectorRef<const Attribute*> attributes);

    /// Destructor
    ~BlockStatement() override;

    /// @returns true if the block has no statements
    bool Empty() const { return statements.IsEmpty(); }

    /// @returns the last statement in the block or nullptr if block empty
    const Statement* Last() const { return statements.IsEmpty() ? nullptr : statements.Back(); }

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const BlockStatement* Clone(CloneContext* ctx) const override;

    /// the statement list
    const utils::Vector<const Statement*, 8> statements;

    /// the attribute list
    const utils::Vector<const Attribute*, 4> attributes;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_BLOCK_STATEMENT_H_
