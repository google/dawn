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

#ifndef SRC_AST_UNARY_OP_EXPRESSION_H_
#define SRC_AST_UNARY_OP_EXPRESSION_H_

#include <memory>
#include <utility>

#include "src/ast/expression.h"
#include "src/ast/literal.h"
#include "src/ast/unary_op.h"

namespace tint {
namespace ast {

/// A unary op expression
class UnaryOpExpression : public Expression {
 public:
  /// Constructor
  UnaryOpExpression();
  /// Constructor
  /// @param op the op
  /// @param expr the expr
  UnaryOpExpression(UnaryOp op, std::unique_ptr<Expression> expr);
  /// Constructor
  /// @param source the unary op expression source
  /// @param op the op
  /// @param expr the expr
  UnaryOpExpression(const Source& source,
                    UnaryOp op,
                    std::unique_ptr<Expression> expr);
  /// Move constructor
  UnaryOpExpression(UnaryOpExpression&&);
  ~UnaryOpExpression() override;

  /// Sets the op
  /// @param op the op
  void set_op(UnaryOp op) { op_ = op; }
  /// @returns the op
  UnaryOp op() const { return op_; }

  /// Sets the expr
  /// @param expr the expression
  void set_expr(std::unique_ptr<Expression> expr) { expr_ = std::move(expr); }
  /// @returns the expression
  Expression* expr() const { return expr_.get(); }

  /// @returns true if this is an as expression
  bool IsUnaryOp() const override;

  /// @returns true if the node is valid
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  UnaryOpExpression(const UnaryOpExpression&) = delete;

  UnaryOp op_ = UnaryOp::kNegation;
  std::unique_ptr<Expression> expr_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_UNARY_OP_EXPRESSION_H_
