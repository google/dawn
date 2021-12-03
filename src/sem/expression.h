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

#ifndef SRC_SEM_EXPRESSION_H_
#define SRC_SEM_EXPRESSION_H_

#include "src/ast/expression.h"
#include "src/sem/behavior.h"
#include "src/sem/constant.h"
#include "src/sem/node.h"

namespace tint {
namespace sem {
// Forward declarations
class Statement;
class Type;

/// Expression holds the semantic information for expression nodes.
class Expression : public Castable<Expression, Node> {
 public:
  /// Constructor
  /// @param declaration the AST node
  /// @param type the resolved type of the expression
  /// @param statement the statement that owns this expression
  /// @param constant the constant value of the expression. May be invalid
  Expression(const ast::Expression* declaration,
             const sem::Type* type,
             const Statement* statement,
             Constant constant);

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

  /// @return the behaviors of this statement
  const sem::Behaviors& Behaviors() const { return behaviors_; }

  /// @return the behaviors of this statement
  sem::Behaviors& Behaviors() { return behaviors_; }

 protected:
  /// The AST expression node for this semantic expression
  const ast::Expression* const declaration_;

 private:
  const sem::Type* const type_;
  const Statement* const statement_;
  const Constant constant_;
  sem::Behaviors behaviors_{sem::Behavior::kNext};
};

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_EXPRESSION_H_
