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

#ifndef SRC_AST_BINARY_EXPRESSION_H_
#define SRC_AST_BINARY_EXPRESSION_H_

#include <memory>
#include <utility>

#include "src/ast/expression.h"
#include "src/ast/literal.h"

namespace tint {
namespace ast {

/// The operator type
enum class BinaryOp {
  kNone = 0,
  kAnd,
  kOr,
  kXor,
  kLogicalAnd,
  kLogicalOr,
  kEqual,
  kNotEqual,
  kLessThan,
  kGreaterThan,
  kLessThanEqual,
  kGreaterThanEqual,
  kShiftLeft,
  kShiftRight,
  kAdd,
  kSubtract,
  kMultiply,
  kDivide,
  kModulo,
};

/// An binary expression
class BinaryExpression : public Expression {
 public:
  /// Constructor
  BinaryExpression();
  /// Constructor
  /// @param op the operation type
  /// @param lhs the left side of the expression
  /// @param rhs the right side of the expression
  BinaryExpression(BinaryOp op,
                   std::unique_ptr<Expression> lhs,
                   std::unique_ptr<Expression> rhs);
  /// Constructor
  /// @param source the binary expression source
  /// @param op the operation type
  /// @param lhs the left side of the expression
  /// @param rhs the right side of the expression
  BinaryExpression(const Source& source,
                   BinaryOp op,
                   std::unique_ptr<Expression> lhs,
                   std::unique_ptr<Expression> rhs);
  /// Move constructor
  BinaryExpression(BinaryExpression&&);
  ~BinaryExpression() override;

  /// Sets the binary op type
  /// @param op the binary op type
  void set_op(BinaryOp op) { op_ = op; }
  /// @returns the binary op type
  BinaryOp op() const { return op_; }

  /// @returns true if the op is and
  bool IsAnd() const { return op_ == BinaryOp::kAnd; }
  /// @returns true if the op is or
  bool IsOr() const { return op_ == BinaryOp::kOr; }
  /// @returns true if the op is xor
  bool IsXor() const { return op_ == BinaryOp::kXor; }
  /// @returns true if the op is logical and
  bool IsLogicalAnd() const { return op_ == BinaryOp::kLogicalAnd; }
  /// @returns true if the op is logical or
  bool IsLogicalOr() const { return op_ == BinaryOp::kLogicalOr; }
  /// @returns true if the op is equal
  bool IsEqual() const { return op_ == BinaryOp::kEqual; }
  /// @returns true if the op is not equal
  bool IsNotEqual() const { return op_ == BinaryOp::kNotEqual; }
  /// @returns true if the op is less than
  bool IsLessThan() const { return op_ == BinaryOp::kLessThan; }
  /// @returns true if the op is greater than
  bool IsGreaterThan() const { return op_ == BinaryOp::kGreaterThan; }
  /// @returns true if the op is less than equal
  bool IsLessThanEqual() const { return op_ == BinaryOp::kLessThanEqual; }
  /// @returns true if the op is greater than equal
  bool IsGreaterThanEqual() const { return op_ == BinaryOp::kGreaterThanEqual; }
  /// @returns true if the op is shift left
  bool IsShiftLeft() const { return op_ == BinaryOp::kShiftLeft; }
  /// @returns true if the op is shift right
  bool IsShiftRight() const { return op_ == BinaryOp::kShiftRight; }
  /// @returns true if the op is add
  bool IsAdd() const { return op_ == BinaryOp::kAdd; }
  /// @returns true if the op is subtract
  bool IsSubtract() const { return op_ == BinaryOp::kSubtract; }
  /// @returns true if the op is multiply
  bool IsMultiply() const { return op_ == BinaryOp::kMultiply; }
  /// @returns true if the op is divide
  bool IsDivide() const { return op_ == BinaryOp::kDivide; }
  /// @returns true if the op is modulo
  bool IsModulo() const { return op_ == BinaryOp::kModulo; }

  /// Sets the left side of the expression
  /// @param lhs the left side to set
  void set_lhs(std::unique_ptr<Expression> lhs) { lhs_ = std::move(lhs); }
  /// @returns the left side expression
  Expression* lhs() const { return lhs_.get(); }

  /// Sets the right side of the expression
  /// @param rhs the right side to set
  void set_rhs(std::unique_ptr<Expression> rhs) { rhs_ = std::move(rhs); }
  /// @returns the right side expression
  Expression* rhs() const { return rhs_.get(); }

  /// @returns true if this is a op expression
  bool IsBinary() const override;

  /// @returns true if the node is valid
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  BinaryExpression(const BinaryExpression&) = delete;

  BinaryOp op_ = BinaryOp::kNone;
  std::unique_ptr<Expression> lhs_;
  std::unique_ptr<Expression> rhs_;
};

inline std::ostream& operator<<(std::ostream& out, BinaryOp op) {
  switch (op) {
    case BinaryOp::kNone:
      out << "none";
      break;
    case BinaryOp::kAnd:
      out << "and";
      break;
    case BinaryOp::kOr:
      out << "or";
      break;
    case BinaryOp::kXor:
      out << "xor";
      break;
    case BinaryOp::kLogicalAnd:
      out << "logical_and";
      break;
    case BinaryOp::kLogicalOr:
      out << "logical_or";
      break;
    case BinaryOp::kEqual:
      out << "equal";
      break;
    case BinaryOp::kNotEqual:
      out << "not_equal";
      break;
    case BinaryOp::kLessThan:
      out << "less_than";
      break;
    case BinaryOp::kGreaterThan:
      out << "greater_than";
      break;
    case BinaryOp::kLessThanEqual:
      out << "less_than_equal";
      break;
    case BinaryOp::kGreaterThanEqual:
      out << "greater_than_equal";
      break;
    case BinaryOp::kShiftLeft:
      out << "shift_left";
      break;
    case BinaryOp::kShiftRight:
      out << "shift_right";
      break;
    case BinaryOp::kAdd:
      out << "add";
      break;
    case BinaryOp::kSubtract:
      out << "subtract";
      break;
    case BinaryOp::kMultiply:
      out << "multiply";
      break;
    case BinaryOp::kDivide:
      out << "divide";
      break;
    case BinaryOp::kModulo:
      out << "modulo";
      break;
  }
  return out;
}

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_BINARY_EXPRESSION_H_
