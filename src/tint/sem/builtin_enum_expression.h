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

#ifndef SRC_TINT_SEM_BUILTIN_ENUM_EXPRESSION_H_
#define SRC_TINT_SEM_BUILTIN_ENUM_EXPRESSION_H_

#include "src/tint/sem/expression.h"

// Forward declarations
namespace tint::type {
class Type;
}  // namespace tint::type

namespace tint::sem {

/// Base class for BuiltinEnumExpression.
/// Useful for Is() queries.
class BuiltinEnumExpressionBase : public utils::Castable<BuiltinEnumExpressionBase, Expression> {
  public:
    /// Constructor
    /// @param declaration the AST node
    /// @param statement the statement that owns this expression
    BuiltinEnumExpressionBase(const ast::Expression* declaration, const Statement* statement);

    /// Destructor
    ~BuiltinEnumExpressionBase() override;
};

/// BuiltinEnumExpression holds the semantic information for expression nodes that resolve to a
/// builtin enumerator value.
template <typename ENUM>
class BuiltinEnumExpression
    : public utils::Castable<BuiltinEnumExpression<ENUM>, BuiltinEnumExpressionBase> {
  public:
    /// Constructor
    /// @param declaration the AST node
    /// @param statement the statement that owns this expression
    /// @param value the enumerator value
    BuiltinEnumExpression(const ast::Expression* declaration,
                          const Statement* statement,
                          ENUM value)
        : BuiltinEnumExpression<ENUM>::Base(declaration, statement), value_(value) {}

    /// Destructor
    ~BuiltinEnumExpression() override = default;

    /// @return the enumerator value
    ENUM Value() const { return value_; }

  private:
    const ENUM value_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_BUILTIN_ENUM_EXPRESSION_H_
