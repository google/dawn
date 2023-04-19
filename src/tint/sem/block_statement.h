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

#ifndef SRC_TINT_SEM_BLOCK_STATEMENT_H_
#define SRC_TINT_SEM_BLOCK_STATEMENT_H_

#include <cstddef>
#include <vector>

#include "src/tint/sem/statement.h"

// Forward declarations
namespace tint::ast {
class BlockStatement;
class ContinueStatement;
class Variable;
}  // namespace tint::ast

namespace tint::sem {

/// Holds semantic information about a block, such as parent block and variables
/// declared in the block.
class BlockStatement : public utils::Castable<BlockStatement, CompoundStatement> {
  public:
    /// Constructor
    /// @param declaration the AST node for this block statement
    /// @param parent the owning statement
    /// @param function the owning function
    BlockStatement(const ast::BlockStatement* declaration,
                   const CompoundStatement* parent,
                   const sem::Function* function);

    /// Destructor
    ~BlockStatement() override;

    /// @returns the AST block statement associated with this semantic block
    /// statement
    const ast::BlockStatement* Declaration() const;
};

/// The root block statement for a function
class FunctionBlockStatement final
    : public utils::Castable<FunctionBlockStatement, BlockStatement> {
  public:
    /// Constructor
    /// @param function the owning function
    explicit FunctionBlockStatement(const sem::Function* function);

    /// Destructor
    ~FunctionBlockStatement() override;
};

/// Holds semantic information about a loop body block or for-loop body block
class LoopBlockStatement final : public utils::Castable<LoopBlockStatement, BlockStatement> {
  public:
    /// Constructor
    /// @param declaration the AST node for this block statement
    /// @param parent the owning statement
    /// @param function the owning function
    LoopBlockStatement(const ast::BlockStatement* declaration,
                       const CompoundStatement* parent,
                       const sem::Function* function);

    /// Destructor
    ~LoopBlockStatement() override;

    /// @returns the first continue statement in this loop block, or nullptr if
    /// there are no continue statements in the block
    const ast::ContinueStatement* FirstContinue() const { return first_continue_; }

    /// @returns the number of variables declared before the first continue
    /// statement
    size_t NumDeclsAtFirstContinue() const { return num_decls_at_first_continue_; }

    /// Allows the resolver to record the first continue statement in the block
    /// and the number of variables declared prior to that statement.
    /// @param first_continue the first continue statement in the block
    /// @param num_decls the number of variable declarations before that continue
    void SetFirstContinue(const ast::ContinueStatement* first_continue, size_t num_decls);

  private:
    /// The first continue statement in this loop block.
    const ast::ContinueStatement* first_continue_ = nullptr;

    /// The number of variables declared before the first continue statement.
    size_t num_decls_at_first_continue_ = 0;
};

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_BLOCK_STATEMENT_H_
