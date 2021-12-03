// Copyright 2021 The Tint Authors.
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

#ifndef SRC_SEM_FOR_LOOP_STATEMENT_H_
#define SRC_SEM_FOR_LOOP_STATEMENT_H_

#include "src/sem/statement.h"

namespace tint {
namespace ast {
class ForLoopStatement;
}  // namespace ast
namespace sem {
class Expression;
}  // namespace sem
}  // namespace tint

namespace tint {
namespace sem {

/// Holds semantic information about a for-loop statement
class ForLoopStatement : public Castable<ForLoopStatement, CompoundStatement> {
 public:
  /// Constructor
  /// @param declaration the AST node for this for-loop statement
  /// @param parent the owning statement
  /// @param function the owning function
  ForLoopStatement(const ast::ForLoopStatement* declaration,
                   const CompoundStatement* parent,
                   const sem::Function* function);

  /// Destructor
  ~ForLoopStatement() override;

  /// @returns the AST node
  const ast::ForLoopStatement* Declaration() const;

  /// @returns the for-loop condition expression
  const Expression* Condition() const { return condition_; }

  /// Sets the for-loop condition expression
  /// @param condition the for-loop condition expression
  void SetCondition(const Expression* condition) { condition_ = condition; }

 private:
  const Expression* condition_ = nullptr;
};

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_FOR_LOOP_STATEMENT_H_
