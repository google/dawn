// Copyright 2022 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0(the "License");
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

#ifndef SRC_TINT_SEM_WHILE_STATEMENT_H_
#define SRC_TINT_SEM_WHILE_STATEMENT_H_

#include "src/tint/sem/statement.h"

// Forward declarations
namespace tint::ast {
class WhileStatement;
}  // namespace tint::ast
namespace tint::sem {
class ValueExpression;
}  // namespace tint::sem

namespace tint::sem {

/// Holds semantic information about a while statement
class WhileStatement final : public utils::Castable<WhileStatement, CompoundStatement> {
  public:
    /// Constructor
    /// @param declaration the AST node for this while statement
    /// @param parent the owning statement
    /// @param function the owning function
    WhileStatement(const ast::WhileStatement* declaration,
                   const CompoundStatement* parent,
                   const sem::Function* function);

    /// Destructor
    ~WhileStatement() override;

    /// @returns the AST node
    const ast::WhileStatement* Declaration() const;

    /// @returns the whilecondition expression
    const ValueExpression* Condition() const { return condition_; }

    /// Sets the while condition expression
    /// @param condition the while condition expression
    void SetCondition(const ValueExpression* condition) { condition_ = condition; }

  private:
    const ValueExpression* condition_ = nullptr;
};

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_WHILE_STATEMENT_H_
