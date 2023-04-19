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

#ifndef SRC_TINT_SEM_EXPRESSION_H_
#define SRC_TINT_SEM_EXPRESSION_H_

#include "src/tint/ast/expression.h"
#include "src/tint/sem/node.h"

// Forward declarations
namespace tint::sem {
class Statement;
}  // namespace tint::sem

namespace tint::sem {

/// Expression holds the semantic information for expression nodes.
class Expression : public utils::Castable<Expression, Node> {
  public:
    /// Constructor
    /// @param declaration the AST node
    /// @param statement the statement that owns this expression
    Expression(const ast::Expression* declaration, const Statement* statement);

    /// Destructor
    ~Expression() override;

    /// @returns the AST node
    const ast::Expression* Declaration() const { return declaration_; }

    /// @return the statement that owns this expression
    const Statement* Stmt() const { return statement_; }

  protected:
    /// The AST expression node for this semantic expression
    const ast::Expression* const declaration_;

  private:
    const Statement* const statement_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_EXPRESSION_H_
