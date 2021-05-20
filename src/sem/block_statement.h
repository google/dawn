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

#ifndef SRC_SEM_BLOCK_STATEMENT_H_
#define SRC_SEM_BLOCK_STATEMENT_H_

#include <vector>

#include "src/sem/statement.h"

// Forward declarations
namespace tint {
namespace ast {
class BlockStatement;
class Function;
class Variable;
}  // namespace ast
}  // namespace tint

namespace tint {
namespace sem {

/// Holds semantic information about a block, such as parent block and variables
/// declared in the block.
class BlockStatement : public Castable<BlockStatement, Statement> {
 public:
  /// Constructor
  /// @param declaration the AST node for this block statement
  /// @param parent the owning statement
  BlockStatement(const ast::BlockStatement* declaration,
                 const Statement* parent);

  /// Destructor
  ~BlockStatement() override;

  /// @returns the AST block statement associated with this semantic block
  /// statement
  const ast::BlockStatement* Declaration() const;

  /// @returns the closest enclosing block that satisfies the given predicate,
  /// which may be the block itself, or nullptr if no match is found
  /// @param pred a predicate that the resulting block must satisfy
  template <typename Pred>
  const BlockStatement* FindFirstParent(Pred&& pred) const {
    const BlockStatement* curr = this;
    while (curr && !pred(curr)) {
      curr = curr->Block();
    }
    return curr;
  }

  /// @returns the statement itself if it matches the template type `T`,
  /// otherwise the nearest enclosing block that matches `T`, or nullptr if
  /// there is none.
  template <typename T>
  const T* FindFirstParent() const {
    const BlockStatement* curr = this;
    while (curr) {
      if (auto* block = curr->As<T>()) {
        return block;
      }
      curr = curr->Block();
    }
    return nullptr;
  }

  /// @returns the declarations associated with this block
  const std::vector<const ast::Variable*>& Decls() const { return decls_; }

  /// Associates a declaration with this block.
  /// @param var a variable declaration to be added to the block
  void AddDecl(ast::Variable* var);

 private:
  std::vector<const ast::Variable*> decls_;
};

/// The root block statement for a function
class FunctionBlockStatement
    : public Castable<FunctionBlockStatement, BlockStatement> {
 public:
  /// Constructor
  /// @param function the owning function
  explicit FunctionBlockStatement(const ast::Function* function);

  /// Destructor
  ~FunctionBlockStatement() override;

  /// @returns the function owning this block
  const ast::Function* Function() const { return function_; }

 private:
  ast::Function const* const function_;
};

/// Holds semantic information about a loop block
class LoopBlockStatement : public Castable<LoopBlockStatement, BlockStatement> {
 public:
  /// Constructor
  /// @param declaration the AST node for this block statement
  /// @param parent the owning statement
  LoopBlockStatement(const ast::BlockStatement* declaration,
                     const Statement* parent);

  /// Destructor
  ~LoopBlockStatement() override;

  /// @returns the index of the first variable declared after the first continue
  /// statement
  size_t FirstContinue() const { return first_continue_; }

  /// Requires that this is a loop block.
  /// Allows the resolver to set the index of the first variable declared after
  /// the first continue statement.
  /// @param first_continue index of the relevant variable
  void SetFirstContinue(size_t first_continue);

 private:
  // first_continue is set to the index of the first variable in decls
  // declared after the first continue statement in a loop block, if any.
  constexpr static size_t kNoContinue = size_t(~0);
  size_t first_continue_ = kNoContinue;
};

/// Holds semantic information about a loop continuing block
class LoopContinuingBlockStatement
    : public Castable<LoopContinuingBlockStatement, BlockStatement> {
 public:
  /// Constructor
  /// @param declaration the AST node for this block statement
  /// @param parent the owning statement
  LoopContinuingBlockStatement(const ast::BlockStatement* declaration,
                               const Statement* parent);

  /// Destructor
  ~LoopContinuingBlockStatement() override;
};

/// Holds semantic information about a switch case block
class SwitchCaseBlockStatement
    : public Castable<SwitchCaseBlockStatement, BlockStatement> {
 public:
  /// Constructor
  /// @param declaration the AST node for this block statement
  /// @param parent the owning statement
  SwitchCaseBlockStatement(const ast::BlockStatement* declaration,
                           const Statement* parent);

  /// Destructor
  ~SwitchCaseBlockStatement() override;
};

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_BLOCK_STATEMENT_H_
