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

#include "src/ast/expression.h"

namespace tint {
namespace ast {

/// The operator type
enum class BinaryOp {
  kNone = 0,
  kAnd,  // &
  kOr,   // |
  kXor,
  kLogicalAnd,  // &&
  kLogicalOr,   // ||
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
class BinaryExpression : public Castable<BinaryExpression, Expression> {
 public:
  /// Constructor
  /// @param program_id the identifier of the program that owns this node
  /// @param source the binary expression source
  /// @param op the operation type
  /// @param lhs the left side of the expression
  /// @param rhs the right side of the expression
  BinaryExpression(ProgramID program_id,
                   const Source& source,
                   BinaryOp op,
                   Expression* lhs,
                   Expression* rhs);
  /// Move constructor
  BinaryExpression(BinaryExpression&&);
  ~BinaryExpression() override;

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
  /// @returns true if the op is an arithmetic operation
  bool IsArithmetic() const;
  /// @returns true if the op is a comparison operation
  bool IsComparison() const;
  /// @returns true if the op is a bitwise operation
  bool IsBitwise() const;
  /// @returns true if the op is a bit shift operation
  bool IsBitshift() const;

  /// @returns the left side expression
  Expression* lhs() const { return lhs_; }
  /// @returns the right side expression
  Expression* rhs() const { return rhs_; }

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned node
  BinaryExpression* Clone(CloneContext* ctx) const override;

  /// Writes a representation of the node to the output stream
  /// @param sem the semantic info for the program
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(const sem::Info& sem,
              std::ostream& out,
              size_t indent) const override;

 private:
  BinaryExpression(const BinaryExpression&) = delete;

  BinaryOp const op_;
  Expression* const lhs_;
  Expression* const rhs_;
};

inline bool BinaryExpression::IsArithmetic() const {
  switch (op_) {
    case ast::BinaryOp::kAdd:
    case ast::BinaryOp::kSubtract:
    case ast::BinaryOp::kMultiply:
    case ast::BinaryOp::kDivide:
    case ast::BinaryOp::kModulo:
      return true;
    default:
      return false;
  }
}

inline bool BinaryExpression::IsComparison() const {
  switch (op_) {
    case ast::BinaryOp::kEqual:
    case ast::BinaryOp::kNotEqual:
    case ast::BinaryOp::kLessThan:
    case ast::BinaryOp::kLessThanEqual:
    case ast::BinaryOp::kGreaterThan:
    case ast::BinaryOp::kGreaterThanEqual:
      return true;
    default:
      return false;
  }
}

inline bool BinaryExpression::IsBitwise() const {
  switch (op_) {
    case ast::BinaryOp::kAnd:
    case ast::BinaryOp::kOr:
    case ast::BinaryOp::kXor:
      return true;
    default:
      return false;
  }
}

inline bool BinaryExpression::IsBitshift() const {
  switch (op_) {
    case ast::BinaryOp::kShiftLeft:
    case ast::BinaryOp::kShiftRight:
      return true;
    default:
      return false;
  }
}

/// @returns the human readable name of the given BinaryOp
/// @param op the BinaryOp
constexpr const char* FriendlyName(BinaryOp op) {
  switch (op) {
    case BinaryOp::kNone:
      return "none";
    case BinaryOp::kAnd:
      return "and";
    case BinaryOp::kOr:
      return "or";
    case BinaryOp::kXor:
      return "xor";
    case BinaryOp::kLogicalAnd:
      return "logical_and";
    case BinaryOp::kLogicalOr:
      return "logical_or";
    case BinaryOp::kEqual:
      return "equal";
    case BinaryOp::kNotEqual:
      return "not_equal";
    case BinaryOp::kLessThan:
      return "less_than";
    case BinaryOp::kGreaterThan:
      return "greater_than";
    case BinaryOp::kLessThanEqual:
      return "less_than_equal";
    case BinaryOp::kGreaterThanEqual:
      return "greater_than_equal";
    case BinaryOp::kShiftLeft:
      return "shift_left";
    case BinaryOp::kShiftRight:
      return "shift_right";
    case BinaryOp::kAdd:
      return "add";
    case BinaryOp::kSubtract:
      return "subtract";
    case BinaryOp::kMultiply:
      return "multiply";
    case BinaryOp::kDivide:
      return "divide";
    case BinaryOp::kModulo:
      return "modulo";
  }
  return "INVALID";
}

/// @param out the std::ostream to write to
/// @param op the BinaryOp
/// @return the std::ostream so calls can be chained
inline std::ostream& operator<<(std::ostream& out, BinaryOp op) {
  out << FriendlyName(op);
  return out;
}

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_BINARY_EXPRESSION_H_
