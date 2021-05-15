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

#include "src/debug.h"
#include "src/sem/statement.h"

namespace tint {

// Forward declarations
namespace ast {
class BlockStatement;
class Variable;
}  // namespace ast

namespace sem {

/// Holds semantic information about a block, such as parent block and variables
/// declared in the block.
class BlockStatement : public Castable<BlockStatement, Statement> {
 public:
  enum class Type { kGeneric, kLoop, kLoopContinuing, kSwitchCase };

  /// Constructor
  /// @param declaration the AST node for this block statement
  /// @param parent the owning statement
  /// @param type the type of block this is
  BlockStatement(const ast::BlockStatement* declaration,
                 const Statement* parent,
                 Type type);

  /// Destructor
  ~BlockStatement() override;

  /// @returns the AST block statement associated with this semantic block
  /// statement
  const ast::BlockStatement* Declaration() const;

  /// @returns the closest enclosing block that satisfies the given predicate,
  ///          which may be the block itself, or nullptr if no match is found
  /// @param pred a predicate that the resulting block must satisfy
  template <typename Pred>
  const BlockStatement* FindFirstParent(Pred&& pred) const {
    const BlockStatement* curr = this;
    while (curr && !pred(curr)) {
      curr = curr->Block();
    }
    return curr;
  }

  /// @returns the closest enclosing block that matches the given type, which
  ///          may be the block itself, or nullptr if no match is found
  /// @param ty the type of block to be searched for
  const BlockStatement* FindFirstParent(BlockStatement::Type ty) const;

  /// @returns the declarations associated with this block
  const std::vector<const ast::Variable*>& Decls() const { return decls_; }

  /// Requires that this is a loop block.
  /// @returns the index of the first variable declared after the first continue
  ///          statement
  size_t FirstContinue() const {
    TINT_ASSERT(type_ == Type::kLoop);
    return first_continue_;
  }

  /// Requires that this is a loop block.
  /// Allows the resolver to set the index of the first variable declared after
  /// the first continue statement.
  /// @param first_continue index of the relevant variable
  void SetFirstContinue(size_t first_continue);

  /// Allows the resolver to associate a declaration with this block.
  /// @param var a variable declaration to be added to the block
  void AddDecl(ast::Variable* var);

 private:
  Type const type_;
  std::vector<const ast::Variable*> decls_;

  // first_continue is set to the index of the first variable in decls
  // declared after the first continue statement in a loop block, if any.
  constexpr static size_t kNoContinue = size_t(~0);
  size_t first_continue_ = kNoContinue;
};

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_BLOCK_STATEMENT_H_
