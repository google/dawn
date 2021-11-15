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

#ifndef SRC_AST_CALL_EXPRESSION_H_
#define SRC_AST_CALL_EXPRESSION_H_

#include "src/ast/expression.h"

namespace tint {
namespace ast {

// Forward declarations.
class Type;
class IdentifierExpression;

/// A call expression - represents either a:
/// * sem::Function
/// * sem::Intrinsic
/// * sem::TypeConstructor
/// * sem::TypeConversion
class CallExpression : public Castable<CallExpression, Expression> {
 public:
  /// Constructor
  /// @param program_id the identifier of the program that owns this node
  /// @param source the call expression source
  /// @param name the function or type name
  /// @param args the arguments
  CallExpression(ProgramID program_id,
                 const Source& source,
                 const IdentifierExpression* name,
                 ExpressionList args);

  /// Constructor
  /// @param program_id the identifier of the program that owns this node
  /// @param source the call expression source
  /// @param type the type
  /// @param args the arguments
  CallExpression(ProgramID program_id,
                 const Source& source,
                 const Type* type,
                 ExpressionList args);

  /// Move constructor
  CallExpression(CallExpression&&);
  ~CallExpression() override;

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned node
  const CallExpression* Clone(CloneContext* ctx) const override;

  /// Target is either an identifier, or a Type.
  /// One of these must be nullptr and the other a non-nullptr.
  struct Target {
    /// name is a function or intrinsic to call, or type name to construct or
    /// cast-to
    const IdentifierExpression* name = nullptr;
    /// type to construct or cast-to
    const Type* type = nullptr;
  };

  /// The target function
  const Target target;

  /// The arguments
  const ExpressionList args;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_CALL_EXPRESSION_H_
