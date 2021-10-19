// Copyright 2020 The Tint Authors.
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

#ifndef SRC_AST_IF_STATEMENT_H_
#define SRC_AST_IF_STATEMENT_H_

#include <utility>

#include "src/ast/else_statement.h"

namespace tint {
namespace ast {

/// An if statement
class IfStatement : public Castable<IfStatement, Statement> {
 public:
  /// Constructor
  /// @param pid the identifier of the program that owns this node
  /// @param src the source of this node
  /// @param condition the if condition
  /// @param body the if body
  /// @param else_stmts the else statements
  IfStatement(ProgramID pid,
              const Source& src,
              const Expression* condition,
              const BlockStatement* body,
              ElseStatementList else_stmts);
  /// Move constructor
  IfStatement(IfStatement&&);
  ~IfStatement() override;

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned node
  const IfStatement* Clone(CloneContext* ctx) const override;

  /// The if condition or nullptr if none set
  const Expression* const condition;

  /// The if body
  const BlockStatement* const body;

  /// The else statements
  const ElseStatementList else_statements;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_IF_STATEMENT_H_
