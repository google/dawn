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

#ifndef SRC_AST_BLOCK_STATEMENT_H_
#define SRC_AST_BLOCK_STATEMENT_H_

#include <memory>
#include <utility>
#include <vector>

#include "src/ast/statement.h"

namespace tint {
namespace ast {

/// A block statement
class BlockStatement : public Castable<BlockStatement, Statement> {
 public:
  /// Constructor
  /// @param source the block statement source
  /// @param statements the statements
  BlockStatement(const Source& source, const StatementList& statements);
  /// Move constructor
  BlockStatement(BlockStatement&&);
  ~BlockStatement() override;

  /// @returns true if the block is empty
  bool empty() const { return statements_.empty(); }
  /// @returns the number of statements directly in the block
  size_t size() const { return statements_.size(); }

  /// @returns the last statement in the block or nullptr if block empty
  const Statement* last() const {
    return statements_.empty() ? nullptr : statements_.back();
  }
  /// @returns the last statement in the block or nullptr if block empty
  Statement* last() {
    return statements_.empty() ? nullptr : statements_.back();
  }

  /// Retrieves the statement at `idx`
  /// @param idx the index. The index is not bounds checked.
  /// @returns the statement at `idx`
  Statement* get(size_t idx) const { return statements_[idx]; }

  /// Retrieves the statement at `idx`
  /// @param idx the index. The index is not bounds checked.
  /// @returns the statement at `idx`
  Statement* operator[](size_t idx) const { return statements_[idx]; }

  /// @returns the beginning iterator
  StatementList::const_iterator begin() const { return statements_.begin(); }
  /// @returns the ending iterator
  StatementList::const_iterator end() const { return statements_.end(); }

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @note Semantic information such as resolved expression type and intrinsic
  /// information is not cloned.
  /// @param ctx the clone context
  /// @return the newly cloned node
  BlockStatement* Clone(CloneContext* ctx) const override;

  /// @returns true if the node is valid
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param sem the semantic info for the program
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(const semantic::Info& sem,
              std::ostream& out,
              size_t indent) const override;

 private:
  BlockStatement(const BlockStatement&) = delete;

  StatementList const statements_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_BLOCK_STATEMENT_H_
