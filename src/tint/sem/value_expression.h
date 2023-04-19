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

#ifndef SRC_TINT_SEM_VALUE_EXPRESSION_H_
#define SRC_TINT_SEM_VALUE_EXPRESSION_H_

#include "src/tint/constant/value.h"
#include "src/tint/sem/behavior.h"
#include "src/tint/sem/evaluation_stage.h"
#include "src/tint/sem/expression.h"

// Forward declarations
namespace tint::sem {
class Statement;
class Variable;
}  // namespace tint::sem

namespace tint::sem {

/// ValueExpression holds the semantic information for expression nodes.
class ValueExpression : public utils::Castable<ValueExpression, Expression> {
  public:
    /// Constructor
    /// @param declaration the AST node
    /// @param type the resolved type of the expression
    /// @param stage the earliest evaluation stage for the expression
    /// @param statement the statement that owns this expression
    /// @param constant the constant value of the expression. May be null
    /// @param has_side_effects true if this expression may have side-effects
    /// @param root_ident the (optional) root identifier for this expression
    ValueExpression(const ast::Expression* declaration,
                    const type::Type* type,
                    EvaluationStage stage,
                    const Statement* statement,
                    const constant::Value* constant,
                    bool has_side_effects,
                    const Variable* root_ident = nullptr);

    /// Destructor
    ~ValueExpression() override;

    /// @return the resolved type of the expression
    const type::Type* Type() const { return type_; }

    /// @return the earliest evaluation stage for the expression
    EvaluationStage Stage() const { return stage_; }

    /// @return the constant value of this expression
    const constant::Value* ConstantValue() const { return constant_; }

    /// Returns the variable or parameter that this expression derives from.
    /// For reference and pointer expressions, this will either be the originating
    /// variable or a function parameter. For other types of expressions, it will
    /// either be the parameter or constant declaration, or nullptr.
    /// @return the root identifier of this expression, or nullptr
    const Variable* RootIdentifier() const { return root_identifier_; }

    /// @return the behaviors of this statement
    const sem::Behaviors& Behaviors() const { return behaviors_; }

    /// @return the behaviors of this statement
    sem::Behaviors& Behaviors() { return behaviors_; }

    /// @return true of this expression may have side effects
    bool HasSideEffects() const { return has_side_effects_; }

    /// @return the inner expression node if this is a Materialize, otherwise this.
    const ValueExpression* UnwrapMaterialize() const;

    /// @return the inner reference expression if this is a Load, otherwise this.
    const ValueExpression* UnwrapLoad() const;

    /// @return the inner expression node if this is a Materialize or Load, otherwise this.
    const ValueExpression* Unwrap() const;

  protected:
    /// The root identifier for this semantic expression, or nullptr
    const Variable* root_identifier_;

  private:
    const type::Type* const type_;
    const EvaluationStage stage_;
    const constant::Value* const constant_;
    sem::Behaviors behaviors_{sem::Behavior::kNext};
    const bool has_side_effects_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_VALUE_EXPRESSION_H_
