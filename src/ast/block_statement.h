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
class BlockStatement : public Statement {
 public:
  /// Constructor
  BlockStatement();
  /// Constructor
  /// @param source the block statement source
  explicit BlockStatement(const Source& source);
  /// Move constructor
  BlockStatement(BlockStatement&&);
  ~BlockStatement() override;

  /// Appends a statement to the block
  /// @param stmt the statement to append
  void append(std::unique_ptr<ast::Statement> stmt) {
    statements_.push_back(std::move(stmt));
  }

  /// Insert a statement to the block
  /// @param index the index to insert at
  /// @param stmt the statement to insert
  void insert(size_t index, std::unique_ptr<ast::Statement> stmt) {
    statements_.insert(statements_.begin() + static_cast<long>(index),
                       std::move(stmt));
  }

  /// @returns true if the block is empty
  bool empty() const { return statements_.empty(); }
  /// @returns the number of statements directly in the block
  size_t size() const { return statements_.size(); }

  /// @returns the last statement in the block or nullptr if block empty
  const ast::Statement* last() const {
    return statements_.empty() ? nullptr : statements_.back().get();
  }
  /// @returns the last statement in the block or nullptr if block empty
  ast::Statement* last() {
    return statements_.empty() ? nullptr : statements_.back().get();
  }

  /// Retrieves the statement at |idx|
  /// @param idx the index. The index is not bounds checked.
  /// @returns the statement at |idx|
  const ast::Statement* get(size_t idx) const { return statements_[idx].get(); }

  /// Retrieves the statement at |idx|
  /// @param idx the index. The index is not bounds checked.
  /// @returns the statement at |idx|
  ast::Statement* operator[](size_t idx) { return statements_[idx].get(); }
  /// Retrieves the statement at |idx|
  /// @param idx the index. The index is not bounds checked.
  /// @returns the statement at |idx|
  const ast::Statement* operator[](size_t idx) const {
    return statements_[idx].get();
  }

  /// @returns the beginning iterator
  std::vector<std::unique_ptr<ast::Statement>>::const_iterator begin() const {
    return statements_.begin();
  }
  /// @returns the ending iterator
  std::vector<std::unique_ptr<ast::Statement>>::const_iterator end() const {
    return statements_.end();
  }

  /// @returns true if this is a block statement
  bool IsBlock() const override;

  /// @returns true if the node is valid
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  BlockStatement(const BlockStatement&) = delete;

  std::vector<std::unique_ptr<ast::Statement>> statements_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_BLOCK_STATEMENT_H_
