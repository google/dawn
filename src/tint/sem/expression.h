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

#ifndef SRC_TINT_SEM_EXPRESSION_H_
#define SRC_TINT_SEM_EXPRESSION_H_

#include "src/tint/ast/expression.h"
#include "src/tint/sem/behavior.h"
#include "src/tint/sem/constant.h"
#include "src/tint/sem/node.h"

// Forward declarations
namespace tint::sem {
class Statement;
class Type;
class Variable;
}  // namespace tint::sem

namespace tint::sem {
/// Expression holds the semantic information for expression nodes.
class Expression : public Castable<Expression, Node> {
  public:
    /// Constructor
    /// @param declaration the AST node
    /// @param type the resolved type of the expression
    /// @param statement the statement that owns this expression
    /// @param constant the constant value of the expression. May be invalid
    /// @param has_side_effects true if this expression may have side-effects
    /// @param source_var the (optional) source variable for this expression
    Expression(const ast::Expression* declaration,
               const sem::Type* type,
               const Statement* statement,
               Constant constant,
               bool has_side_effects,
               const Variable* source_var = nullptr);

    /// Destructor
    ~Expression() override;

    /// @returns the AST node
    const ast::Expression* Declaration() const { return declaration_; }

    /// @return the resolved type of the expression
    const sem::Type* Type() const { return type_; }

    /// @return the statement that owns this expression
    const Statement* Stmt() const { return statement_; }

    /// @return the constant value of this expression
    const Constant& ConstantValue() const { return constant_; }

    /// Returns the variable or parameter that this expression derives from.
    /// For reference and pointer expressions, this will either be the originating
    /// variable or a function parameter. For other types of expressions, it will
    /// either be the parameter or constant declaration, or nullptr.
    /// @return the source variable of this expression, or nullptr
    const Variable* SourceVariable() const { return source_variable_; }

    /// @return the behaviors of this statement
    const sem::Behaviors& Behaviors() const { return behaviors_; }

    /// @return the behaviors of this statement
    sem::Behaviors& Behaviors() { return behaviors_; }

    /// @return true of this expression may have side effects
    bool HasSideEffects() const { return has_side_effects_; }

  protected:
    /// The AST expression node for this semantic expression
    const ast::Expression* const declaration_;
    /// The source variable for this semantic expression, or nullptr
    const Variable* source_variable_;

  private:
    const sem::Type* const type_;
    const Statement* const statement_;
    const Constant constant_;
    sem::Behaviors behaviors_{sem::Behavior::kNext};
    const bool has_side_effects_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_EXPRESSION_H_
