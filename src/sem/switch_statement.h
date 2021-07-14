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
  SwitchStatement(const ast::SwitchStatement* declaration,
                  CompoundStatement* parent);

  /// Destructor
  ~SwitchStatement() override;
};

/// Holds semantic information about a switch case block
class SwitchCaseBlockStatement
    : public Castable<SwitchCaseBlockStatement, BlockStatement> {
 public:
  /// Constructor
  /// @param declaration the AST node for this block statement
  /// @param parent the owning statement
  SwitchCaseBlockStatement(const ast::BlockStatement* declaration,
                           const CompoundStatement* parent);

  /// Destructor
  ~SwitchCaseBlockStatement() override;
};

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_SWITCH_STATEMENT_H_
