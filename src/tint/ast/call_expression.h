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

#ifndef SRC_TINT_AST_CALL_EXPRESSION_H_
#define SRC_TINT_AST_CALL_EXPRESSION_H_

#include "src/tint/ast/expression.h"

// Forward declarations
namespace tint::ast {
class IdentifierExpression;
}  // namespace tint::ast

namespace tint::ast {

/// A call expression - represents either a:
/// * sem::Function
/// * sem::Builtin
/// * sem::ValueConstructor
/// * sem::ValueConversion
class CallExpression final : public utils::Castable<CallExpression, Expression> {
  public:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param source the call expression source
    /// @param target the target of the call
    /// @param args the arguments
    CallExpression(ProgramID pid,
                   NodeID nid,
                   const Source& source,
                   const IdentifierExpression* target,
                   utils::VectorRef<const Expression*> args);

    /// Destructor
    ~CallExpression() override;

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const CallExpression* Clone(CloneContext* ctx) const override;

    /// The target function or type
    const IdentifierExpression* target;

    /// The arguments
    const utils::Vector<const Expression*, 8> args;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_CALL_EXPRESSION_H_
