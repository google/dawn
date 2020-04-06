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

#ifndef SRC_AST_REGARDLESS_STATEMENT_H_
#define SRC_AST_REGARDLESS_STATEMENT_H_

#include <memory>
#include <utility>

#include "src/ast/expression.h"
#include "src/ast/statement.h"

namespace tint {
namespace ast {

/// A regardless statement
class RegardlessStatement : public Statement {
 public:
  /// Constructor
  RegardlessStatement();
  /// Constructor
  /// @param condition the condition expression
  /// @param body the body statements
  RegardlessStatement(std::unique_ptr<Expression> condition,
                      StatementList body);
  /// Constructor
  /// @param source the regardless statement source
  /// @param condition the condition expression
  /// @param body the body statements
  RegardlessStatement(const Source& source,
                      std::unique_ptr<Expression> condition,
                      StatementList body);
  /// Move constructor
  RegardlessStatement(RegardlessStatement&&) = default;
  ~RegardlessStatement() override;

  /// Sets the condition expression
  /// @param condition the condition expression
  void set_condition(std::unique_ptr<Expression> condition) {
    condition_ = std::move(condition);
  }
  /// @returns the condition statements
  Expression* condition() const { return condition_.get(); }

  /// Sets the body statements
  /// @param body the body statements
  void set_body(StatementList body) { body_ = std::move(body); }
  /// @returns the body statements
  const StatementList& body() const { return body_; }

  /// @returns true if this is an regardless statement
  bool IsRegardless() const override { return true; }

  /// @returns true if the node is valid
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  RegardlessStatement(const RegardlessStatement&) = delete;

  std::unique_ptr<Expression> condition_;
  StatementList body_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_REGARDLESS_STATEMENT_H_
