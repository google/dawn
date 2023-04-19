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

#ifndef SRC_TINT_AST_UNARY_OP_EXPRESSION_H_
#define SRC_TINT_AST_UNARY_OP_EXPRESSION_H_

#include "src/tint/ast/expression.h"
#include "src/tint/ast/unary_op.h"

namespace tint::ast {

/// A unary op expression
class UnaryOpExpression final : public utils::Castable<UnaryOpExpression, Expression> {
  public:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param source the unary op expression source
    /// @param op the op
    /// @param expr the expr
    UnaryOpExpression(ProgramID pid,
                      NodeID nid,
                      const Source& source,
                      UnaryOp op,
                      const Expression* expr);

    /// Destructor
    ~UnaryOpExpression() override;

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const UnaryOpExpression* Clone(CloneContext* ctx) const override;

    /// The op
    const UnaryOp op;

    /// The expression
    const Expression* const expr;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_UNARY_OP_EXPRESSION_H_
