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

#ifndef SRC_AST_ASSIGNMENT_STATEMENT_H_
#define SRC_AST_ASSIGNMENT_STATEMENT_H_

#include <memory>
#include <utility>

#include "src/ast/expression.h"
#include "src/ast/literal.h"
#include "src/ast/statement.h"

namespace tint {
namespace ast {

/// An assignment statement
class AssignmentStatement : public Statement {
 public:
  /// Constructor
  AssignmentStatement();
  /// Constructor
  /// @param lhs the left side of the expression
  /// @param rhs the right side of the expression
  AssignmentStatement(std::unique_ptr<Expression> lhs,
                      std::unique_ptr<Expression> rhs);
  /// Constructor
  /// @param source the assignment statement source
  /// @param lhs the left side of the expression
  /// @param rhs the right side of the expression
  AssignmentStatement(const Source& source,
                      std::unique_ptr<Expression> lhs,
                      std::unique_ptr<Expression> rhs);
  /// Move constructor
  AssignmentStatement(AssignmentStatement&&);
  ~AssignmentStatement() override;

  /// Sets the left side of the statement
  /// @param lhs the left side to set
  void set_lhs(std::unique_ptr<Expression> lhs) { lhs_ = std::move(lhs); }
  /// @returns the left side expression
  Expression* lhs() const { return lhs_.get(); }

  /// Sets the right side of the statement
  /// @param rhs the right side to set
  void set_rhs(std::unique_ptr<Expression> rhs) { rhs_ = std::move(rhs); }
  /// @returns the right side expression
  Expression* rhs() const { return rhs_.get(); }

  /// @returns true if this is an assignment statement
  bool IsAssign() const override;

  /// @returns true if the node is valid
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  AssignmentStatement(const AssignmentStatement&) = delete;

  std::unique_ptr<Expression> lhs_;
  std::unique_ptr<Expression> rhs_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_ASSIGNMENT_STATEMENT_H_
