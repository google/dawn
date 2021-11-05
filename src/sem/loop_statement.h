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

#ifndef SRC_SEM_LOOP_STATEMENT_H_
#define SRC_SEM_LOOP_STATEMENT_H_

#include "src/sem/block_statement.h"

// Forward declarations
namespace tint {
namespace ast {
class LoopStatement;
}  // namespace ast
}  // namespace tint

namespace tint {
namespace sem {

/// Holds semantic information about a loop statement
class LoopStatement : public Castable<LoopStatement, CompoundStatement> {
 public:
  /// Constructor
  /// @param declaration the AST node for this loop statement
  /// @param parent the owning statement
  /// @param function the owning function
  LoopStatement(const ast::LoopStatement* declaration,
                const CompoundStatement* parent,
                const sem::Function* function);

  /// Destructor
  ~LoopStatement() override;
};

/// Holds semantic information about a loop continuing block
class LoopContinuingBlockStatement
    : public Castable<LoopContinuingBlockStatement, BlockStatement> {
 public:
  /// Constructor
  /// @param declaration the AST node for this block statement
  /// @param parent the owning statement
  /// @param function the owning function
  LoopContinuingBlockStatement(const ast::BlockStatement* declaration,
                               const CompoundStatement* parent,
                               const sem::Function* function);

  /// Destructor
  ~LoopContinuingBlockStatement() override;
};

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_LOOP_STATEMENT_H_
