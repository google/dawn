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

#ifndef SRC_TINT_LANG_WGSL_SEM_ACCESSOR_EXPRESSION_H_
#define SRC_TINT_LANG_WGSL_SEM_ACCESSOR_EXPRESSION_H_

#include <vector>

#include "src/tint/lang/wgsl/ast/index_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"

namespace tint::sem {

/// AccessorExpression is the base class for all semantic information for an ast::AccessorExpression
/// node.
class AccessorExpression : public Castable<AccessorExpression, ValueExpression> {
  public:
    /// Destructor
    ~AccessorExpression() override;

    /// @returns the AST node
    const ast::AccessorExpression* Declaration() const {
        return static_cast<const ast::AccessorExpression*>(declaration_);
    }

    /// @returns the object expression that is being indexed
    ValueExpression const* Object() const { return object_; }

  protected:
    /// Constructor
    /// @param declaration the AST node
    /// @param type the resolved type of the expression
    /// @param stage the earliest evaluation stage for the expression
    /// @param object the object expression that is being indexed
    /// @param statement the statement that owns this expression
    /// @param constant the constant value of the expression. May be null
    /// @param has_side_effects whether this expression may have side effects
    /// @param root_ident the (optional) root identifier for this expression
    AccessorExpression(const ast::AccessorExpression* declaration,
                       const type::Type* type,
                       core::EvaluationStage stage,
                       const ValueExpression* object,
                       const Statement* statement,
                       const constant::Value* constant,
                       bool has_side_effects,
                       const Variable* root_ident = nullptr);

  private:
    ValueExpression const* const object_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_LANG_WGSL_SEM_ACCESSOR_EXPRESSION_H_
