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

#ifndef SRC_TINT_AST_IDENTIFIER_EXPRESSION_H_
#define SRC_TINT_AST_IDENTIFIER_EXPRESSION_H_

#include "src/tint/ast/expression.h"

// Forward declarations
namespace tint::ast {
class Identifier;
}

namespace tint::ast {

/// An identifier expression
class IdentifierExpression final : public utils::Castable<IdentifierExpression, Expression> {
  public:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param src the source of this node
    /// @param identifier the identifier
    IdentifierExpression(ProgramID pid,
                         NodeID nid,
                         const Source& src,
                         const Identifier* identifier);

    /// Destructor
    ~IdentifierExpression() override;

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const IdentifierExpression* Clone(CloneContext* ctx) const override;

    /// The identifier for the expression
    Identifier const* const identifier;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_IDENTIFIER_EXPRESSION_H_
