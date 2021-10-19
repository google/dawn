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

#ifndef SRC_AST_ELSE_STATEMENT_H_
#define SRC_AST_ELSE_STATEMENT_H_

#include <vector>

#include "src/ast/block_statement.h"
#include "src/ast/expression.h"

namespace tint {
namespace ast {

/// An else statement
class ElseStatement : public Castable<ElseStatement, Statement> {
 public:
  /// Constructor
  /// @param pid the identifier of the program that owns this node
  /// @param src the source of this node
  /// @param condition the else condition
  /// @param body the else body
  ElseStatement(ProgramID pid,
                const Source& src,
                const Expression* condition,
                const BlockStatement* body);
  /// Move constructor
  ElseStatement(ElseStatement&&);
  ~ElseStatement() override;

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned node
  const ElseStatement* Clone(CloneContext* ctx) const override;

  /// The else condition or nullptr if none set
  const Expression* const condition;

  /// The else body
  const BlockStatement* const body;
};

/// A list of else statements
using ElseStatementList = std::vector<const ElseStatement*>;

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_ELSE_STATEMENT_H_
