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

#ifndef SRC_SEM_IF_STATEMENT_H_
#define SRC_SEM_IF_STATEMENT_H_

#include "src/sem/statement.h"

// Forward declarations
namespace tint {
namespace ast {
class IfStatement;
class ElseStatement;
}  // namespace ast
namespace sem {
class Expression;
}  // namespace sem
}  // namespace tint

namespace tint {
namespace sem {

/// Holds semantic information about an if statement
class IfStatement : public Castable<IfStatement, CompoundStatement> {
 public:
  /// Constructor
  /// @param declaration the AST node for this if statement
  /// @param parent the owning statement
  /// @param function the owning function
  IfStatement(const ast::IfStatement* declaration,
              const CompoundStatement* parent,
              const sem::Function* function);

  /// Destructor
  ~IfStatement() override;

  /// @returns the if-statement condition expression
  const Expression* Condition() const { return condition_; }

  /// Sets the if-statement condition expression
  /// @param condition the if condition expression
  void SetCondition(const Expression* condition) { condition_ = condition; }

 private:
  const Expression* condition_ = nullptr;
};

/// Holds semantic information about an else statement
class ElseStatement : public Castable<ElseStatement, CompoundStatement> {
 public:
  /// Constructor
  /// @param declaration the AST node for this else statement
  /// @param parent the owning statement
  /// @param function the owning function
  ElseStatement(const ast::ElseStatement* declaration,
                const CompoundStatement* parent,
                const sem::Function* function);

  /// Destructor
  ~ElseStatement() override;

  /// @returns the else-statement condition expression
  const Expression* Condition() const { return condition_; }

  /// Sets the else-statement condition expression
  /// @param condition the else condition expression
  void SetCondition(const Expression* condition) { condition_ = condition; }

 private:
  const Expression* condition_ = nullptr;
};

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_IF_STATEMENT_H_
