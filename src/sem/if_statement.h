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
}  // namespace tint

namespace tint {
namespace sem {

/// Holds semantic information about an if statement
class IfStatement : public Castable<IfStatement, CompoundStatement> {
 public:
  /// Constructor
  /// @param declaration the AST node for this if statement
  /// @param parent the owning statement
  IfStatement(const ast::IfStatement* declaration, CompoundStatement* parent);

  /// Destructor
  ~IfStatement() override;
};

/// Holds semantic information about an else statement
class ElseStatement : public Castable<ElseStatement, CompoundStatement> {
 public:
  /// Constructor
  /// @param declaration the AST node for this else statement
  /// @param parent the owning statement
  ElseStatement(const ast::ElseStatement* declaration,
                CompoundStatement* parent);

  /// Destructor
  ~ElseStatement() override;
};

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_IF_STATEMENT_H_
