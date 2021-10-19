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

#ifndef SRC_AST_CALL_STATEMENT_H_
#define SRC_AST_CALL_STATEMENT_H_

#include "src/ast/call_expression.h"
#include "src/ast/statement.h"

namespace tint {
namespace ast {

/// A call expression
class CallStatement : public Castable<CallStatement, Statement> {
 public:
  /// Constructor
  /// @param pid the identifier of the program that owns this node
  /// @param src the source of this node for the statement
  /// @param call the function
  CallStatement(ProgramID pid, const Source& src, const CallExpression* call);
  /// Move constructor
  CallStatement(CallStatement&&);
  ~CallStatement() override;

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned node
  const CallStatement* Clone(CloneContext* ctx) const override;

  /// The call expression
  const CallExpression* const expr;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_CALL_STATEMENT_H_
