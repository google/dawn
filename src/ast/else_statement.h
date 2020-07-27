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

#ifndef SRC_AST_ELSE_STATEMENT_H_
#define SRC_AST_ELSE_STATEMENT_H_

#include <memory>
#include <utility>
#include <vector>

#include "src/ast/block_statement.h"
#include "src/ast/expression.h"
#include "src/ast/statement.h"

namespace tint {
namespace ast {

/// An else statement
class ElseStatement : public Statement {
 public:
  /// Constructor
  ElseStatement();
  /// Constructor
  /// @param body the else body
  explicit ElseStatement(std::unique_ptr<BlockStatement> body);
  /// Constructor
  /// @param condition the else condition
  /// @param body the else body
  ElseStatement(std::unique_ptr<Expression> condition,
                std::unique_ptr<BlockStatement> body);
  /// Constructor
  /// @param source the source information
  /// @param body the else body
  ElseStatement(const Source& source, std::unique_ptr<BlockStatement> body);
  /// Constructor
  /// @param source the source information
  /// @param condition the else condition
  /// @param body the else body
  ElseStatement(const Source& source,
                std::unique_ptr<Expression> condition,
                std::unique_ptr<BlockStatement> body);
  /// Move constructor
  ElseStatement(ElseStatement&&);
  ~ElseStatement() override;

  /// Sets the condition for the else statement
  /// @param condition the condition to set
  void set_condition(std::unique_ptr<Expression> condition) {
    condition_ = std::move(condition);
  }
  /// @returns the else condition or nullptr if none set
  Expression* condition() const { return condition_.get(); }
  /// @returns true if the else has a condition
  bool HasCondition() const { return condition_ != nullptr; }

  /// Sets the else body
  /// @param body the else body
  void set_body(std::unique_ptr<BlockStatement> body) {
    body_ = std::move(body);
  }
  /// @returns the else body
  const BlockStatement* body() const { return body_.get(); }

  /// @returns true if this is a else statement
  bool IsElse() const override;

  /// @returns true if the node is valid
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  ElseStatement(const ElseStatement&) = delete;

  std::unique_ptr<Expression> condition_;
  std::unique_ptr<BlockStatement> body_;
};

/// A list of unique else statements
using ElseStatementList = std::vector<std::unique_ptr<ElseStatement>>;

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_ELSE_STATEMENT_H_
