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

#ifndef SRC_TINT_AST_FOR_LOOP_STATEMENT_H_
#define SRC_TINT_AST_FOR_LOOP_STATEMENT_H_

#include "src/tint/ast/block_statement.h"

namespace tint::ast {

class Expression;

/// A for loop statement
class ForLoopStatement final : public utils::Castable<ForLoopStatement, Statement> {
  public:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param source the for loop statement source
    /// @param initializer the optional loop initializer statement
    /// @param condition the optional loop condition expression
    /// @param continuing the optional continuing statement
    /// @param body the loop body
    /// @param attributes the while statement attributes
    ForLoopStatement(ProgramID pid,
                     NodeID nid,
                     const Source& source,
                     const Statement* initializer,
                     const Expression* condition,
                     const Statement* continuing,
                     const BlockStatement* body,
                     utils::VectorRef<const ast::Attribute*> attributes);

    /// Destructor
    ~ForLoopStatement() override;

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const ForLoopStatement* Clone(CloneContext* ctx) const override;

    /// The initializer statement
    const Statement* const initializer;

    /// The condition expression
    const Expression* const condition;

    /// The continuing statement
    const Statement* const continuing;

    /// The loop body block
    const BlockStatement* const body;

    /// The attribute list
    const utils::Vector<const Attribute*, 1> attributes;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_FOR_LOOP_STATEMENT_H_
