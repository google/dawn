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

#ifndef SRC_AST_BREAK_STATEMENT_H_
#define SRC_AST_BREAK_STATEMENT_H_

#include <memory>
#include <utility>

#include "src/ast/expression.h"
#include "src/ast/statement.h"
#include "src/ast/statement_condition.h"

namespace tint {
namespace ast {

/// An break statement
class BreakStatement : public Statement {
 public:
  /// Constructor
  BreakStatement();
  /// Constructor
  /// @param source the initializer source
  explicit BreakStatement(const Source& source);
  /// Constructor
  /// @param condition the condition type
  /// @param conditional the condition expression
  BreakStatement(StatementCondition condition,
                 std::unique_ptr<Expression> conditional);
  /// Constructor
  /// @param source the initializer source
  /// @param condition the condition type
  /// @param conditional the condition expression
  BreakStatement(const Source& source,
                 StatementCondition condition,
                 std::unique_ptr<Expression> conditional);
  /// Move constructor
  BreakStatement(BreakStatement&&) = default;
  ~BreakStatement() override;

  /// Sets the condition type
  /// @param condition the condition type
  void set_condition(StatementCondition condition) { condition_ = condition; }
  /// @returns the condition type
  StatementCondition condition() const { return condition_; }

  /// Sets the conditional expression
  /// @param conditional the conditional expression
  void set_conditional(std::unique_ptr<Expression> conditional) {
    conditional_ = std::move(conditional);
  }
  /// @returns the conditional expression
  Expression* conditional() const { return conditional_.get(); }

  /// @returns true if this is an break statement
  bool IsBreak() const override { return true; }

  /// @returns true if the node is valid
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  BreakStatement(const BreakStatement&) = delete;

  StatementCondition condition_ = StatementCondition::kNone;
  std::unique_ptr<Expression> conditional_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_BREAK_STATEMENT_H_
