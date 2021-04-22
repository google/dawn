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
  Expression(ast::Expression* declaration,
             const sem::Type* type,
             Statement* statement);

  /// @return the resolved type of the expression
  sem::Type* Type() const { return const_cast<sem::Type*>(type_); }

  /// @return the statement that owns this expression
  Statement* Stmt() const { return statement_; }

  /// @returns the AST node
  ast::Expression* Declaration() const { return declaration_; }

 private:
  ast::Expression* declaration_;
  const sem::Type* const type_;
  Statement* const statement_;
};

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_EXPRESSION_H_
