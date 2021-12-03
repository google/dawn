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

#ifndef SRC_SEM_SWITCH_STATEMENT_H_
#define SRC_SEM_SWITCH_STATEMENT_H_

#include "src/sem/block_statement.h"

// Forward declarations
namespace tint {
namespace ast {
class CaseStatement;
class SwitchStatement;
}  // namespace ast
}  // namespace tint

namespace tint {
namespace sem {

/// Holds semantic information about an switch statement
class SwitchStatement : public Castable<SwitchStatement, CompoundStatement> {
 public:
  /// Constructor
  /// @param declaration the AST node for this switch statement
  /// @param parent the owning statement
  /// @param function the owning function
  SwitchStatement(const ast::SwitchStatement* declaration,
                  const CompoundStatement* parent,
                  const sem::Function* function);

  /// Destructor
  ~SwitchStatement() override;

  /// @return the AST node for this statement
  const ast::SwitchStatement* Declaration() const;
};

/// Holds semantic information about a switch case statement
class CaseStatement : public Castable<CaseStatement, CompoundStatement> {
 public:
  /// Constructor
  /// @param declaration the AST node for this case statement
  /// @param parent the owning statement
  /// @param function the owning function
  CaseStatement(const ast::CaseStatement* declaration,
                const CompoundStatement* parent,
                const sem::Function* function);

  /// Destructor
  ~CaseStatement() override;

  /// @return the AST node for this statement
  const ast::CaseStatement* Declaration() const;

  /// @param body the case body block statement
  void SetBlock(const BlockStatement* body) { body_ = body; }

  /// @returns the case body block statement
  const BlockStatement* Body() const { return body_; }

 private:
  const BlockStatement* body_ = nullptr;
};

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_SWITCH_STATEMENT_H_
