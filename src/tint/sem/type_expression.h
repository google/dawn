// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_SEM_TYPE_EXPRESSION_H_
#define SRC_TINT_SEM_TYPE_EXPRESSION_H_

#include "src/tint/sem/expression.h"

// Forward declarations
namespace tint::type {
class Type;
}  // namespace tint::type

namespace tint::sem {

/// TypeExpression holds the semantic information for expression nodes that resolve to types.
class TypeExpression : public utils::Castable<TypeExpression, Expression> {
  public:
    /// Constructor
    /// @param declaration the AST node
    /// @param statement the statement that owns this expression
    /// @param type the type that this expression resolved to
    TypeExpression(const ast::Expression* declaration,
                   const Statement* statement,
                   const type::Type* type);

    /// Destructor
    ~TypeExpression() override;

    /// @return the type that the expression resolved to
    const type::Type* Type() const { return type_; }

  private:
    type::Type const* const type_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_TYPE_EXPRESSION_H_
