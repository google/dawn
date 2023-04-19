// Copyright 2022 The Tint Authors.
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

#ifndef SRC_TINT_SEM_INDEX_ACCESSOR_EXPRESSION_H_
#define SRC_TINT_SEM_INDEX_ACCESSOR_EXPRESSION_H_

#include <vector>

#include "src/tint/ast/index_accessor_expression.h"
#include "src/tint/sem/value_expression.h"

namespace tint::sem {

/// IndexAccessorExpression holds the semantic information for a ast::IndexAccessorExpression node.
class IndexAccessorExpression final
    : public utils::Castable<IndexAccessorExpression, ValueExpression> {
  public:
    /// Constructor
    /// @param declaration the AST node
    /// @param type the resolved type of the expression
    /// @param stage the earliest evaluation stage for the expression
    /// @param object the object expression that is being indexed
    /// @param index the index expression
    /// @param statement the statement that owns this expression
    /// @param constant the constant value of the expression. May be null
    /// @param has_side_effects whether this expression may have side effects
    /// @param root_ident the (optional) root identifier for this expression
    IndexAccessorExpression(const ast::IndexAccessorExpression* declaration,
                            const type::Type* type,
                            EvaluationStage stage,
                            const ValueExpression* object,
                            const ValueExpression* index,
                            const Statement* statement,
                            const constant::Value* constant,
                            bool has_side_effects,
                            const Variable* root_ident = nullptr);

    /// Destructor
    ~IndexAccessorExpression() override;

    /// @returns the AST node
    const ast::IndexAccessorExpression* Declaration() const {
        return static_cast<const ast::IndexAccessorExpression*>(declaration_);
    }

    /// @returns the object expression that is being indexed
    ValueExpression const* Object() const { return object_; }

    /// @returns the index expression
    ValueExpression const* Index() const { return index_; }

  private:
    ValueExpression const* const object_;
    ValueExpression const* const index_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_INDEX_ACCESSOR_EXPRESSION_H_
