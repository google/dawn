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

#ifndef SRC_AST_SCALAR_CONSTRUCTOR_EXPRESSION_H_
#define SRC_AST_SCALAR_CONSTRUCTOR_EXPRESSION_H_

#include "src/ast/constructor_expression.h"
#include "src/ast/literal.h"

namespace tint {
namespace ast {

/// A scalar constructor
class ScalarConstructorExpression
    : public Castable<ScalarConstructorExpression, ConstructorExpression> {
 public:
  /// Constructor
  /// @param pid the identifier of the program that owns this node
  /// @param src the source of this node
  /// @param literal the const literal
  ScalarConstructorExpression(ProgramID pid,
                              const Source& src,
                              const Literal* literal);
  /// Move constructor
  ScalarConstructorExpression(ScalarConstructorExpression&&);
  ~ScalarConstructorExpression() override;

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned node
  const ScalarConstructorExpression* Clone(CloneContext* ctx) const override;

  /// The literal value
  const Literal* const literal;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_SCALAR_CONSTRUCTOR_EXPRESSION_H_
