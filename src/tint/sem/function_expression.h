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

#ifndef SRC_TINT_SEM_FUNCTION_EXPRESSION_H_
#define SRC_TINT_SEM_FUNCTION_EXPRESSION_H_

#include "src/tint/sem/expression.h"

// Forward declarations
namespace tint::sem {
class Function;
}  // namespace tint::sem

namespace tint::sem {

/// FunctionExpression holds the semantic information for expression nodes that resolve to
/// functions.
class FunctionExpression : public utils::Castable<FunctionExpression, Expression> {
  public:
    /// Constructor
    /// @param declaration the AST node
    /// @param statement the statement that owns this expression
    /// @param function the function that the expression resolved to
    FunctionExpression(const ast::Expression* declaration,
                       const Statement* statement,
                       const sem::Function* function);

    /// Destructor
    ~FunctionExpression() override;

    /// @return the function that the expression resolved to
    const sem::Function* Function() const { return function_; }

  private:
    sem::Function const* const function_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_FUNCTION_EXPRESSION_H_
