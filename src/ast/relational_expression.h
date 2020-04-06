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

#ifndef SRC_AST_RELATIONAL_EXPRESSION_H_
#define SRC_AST_RELATIONAL_EXPRESSION_H_

#include <memory>
#include <utility>

#include "src/ast/expression.h"
#include "src/ast/literal.h"

namespace tint {
namespace ast {

/// The relation type
enum class Relation {
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
  kShiftRightArith,
  kAdd,
  kSubtract,
  kMultiply,
  kDivide,
  kModulo,
};

/// A Relational Expression
class RelationalExpression : public Expression {
 public:
  /// Constructor
  RelationalExpression();
  /// Constructor
  /// @param relation the relation type
  /// @param lhs the left side of the expression
  /// @param rhs the right side of the expression
  RelationalExpression(Relation relation,
                       std::unique_ptr<Expression> lhs,
                       std::unique_ptr<Expression> rhs);
  /// Constructor
  /// @param source the relational expression source
  /// @param relation the relation type
  /// @param lhs the left side of the expression
  /// @param rhs the right side of the expression
  RelationalExpression(const Source& source,
                       Relation relation,
                       std::unique_ptr<Expression> lhs,
                       std::unique_ptr<Expression> rhs);
  /// Move constructor
  RelationalExpression(RelationalExpression&&) = default;
  ~RelationalExpression() override;

  /// Sets the relation type
  /// @param relation the relation type
  void set_relation(Relation relation) { relation_ = relation; }
  /// @returns the relation
  Relation relation() const { return relation_; }

  /// @returns true if the relation is and
  bool IsAnd() const { return relation_ == Relation::kAnd; }
  /// @returns true if the relation is or
  bool IsOr() const { return relation_ == Relation::kOr; }
  /// @returns true if the relation is xor
  bool IsXor() const { return relation_ == Relation::kXor; }
  /// @returns true if the relation is logical and
  bool IsLogicalAnd() const { return relation_ == Relation::kLogicalAnd; }
  /// @returns true if the relation is logical or
  bool IsLogicalOr() const { return relation_ == Relation::kLogicalOr; }
  /// @returns true if the relation is equal
  bool IsEqual() const { return relation_ == Relation::kEqual; }
  /// @returns true if the relation is not equal
  bool IsNotEqual() const { return relation_ == Relation::kNotEqual; }
  /// @returns true if the relation is less than
  bool IsLessThan() const { return relation_ == Relation::kLessThan; }
  /// @returns true if the relation is greater than
  bool IsGreaterThan() const { return relation_ == Relation::kGreaterThan; }
  /// @returns true if the relation is less than equal
  bool IsLessThanEqual() const { return relation_ == Relation::kLessThanEqual; }
  /// @returns true if the relation is greater than equal
  bool IsGreaterThanEqual() const {
    return relation_ == Relation::kGreaterThanEqual;
  }
  /// @returns true if the relation is shift left
  bool IsShiftLeft() const { return relation_ == Relation::kShiftLeft; }
  /// @returns true if the relation is shift right
  bool IsShiftRight() const { return relation_ == Relation::kShiftRight; }
  /// @returns true if the relation is shift right arith
  bool IsShiftRightArith() const {
    return relation_ == Relation::kShiftRightArith;
  }
  /// @returns true if the relation is add
  bool IsAdd() const { return relation_ == Relation::kAdd; }
  /// @returns true if the relation is subtract
  bool IsSubtract() const { return relation_ == Relation::kSubtract; }
  /// @returns true if the relation is multiply
  bool IsMultiply() const { return relation_ == Relation::kMultiply; }
  /// @returns true if the relation is divide
  bool IsDivide() const { return relation_ == Relation::kDivide; }
  /// @returns true if the relation is modulo
  bool IsModulo() const { return relation_ == Relation::kModulo; }

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

  /// @returns true if this is a relational expression
  bool IsRelational() const override { return true; }

  /// @returns true if the node is valid
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  RelationalExpression(const RelationalExpression&) = delete;

  Relation relation_ = Relation::kNone;
  std::unique_ptr<Expression> lhs_;
  std::unique_ptr<Expression> rhs_;
};

inline std::ostream& operator<<(std::ostream& out, Relation relation) {
  switch (relation) {
    case Relation::kNone:
      out << "none";
      break;
    case Relation::kAnd:
      out << "and";
      break;
    case Relation::kOr:
      out << "or";
      break;
    case Relation::kXor:
      out << "xor";
      break;
    case Relation::kLogicalAnd:
      out << "logical_and";
      break;
    case Relation::kLogicalOr:
      out << "logical_or";
      break;
    case Relation::kEqual:
      out << "equal";
      break;
    case Relation::kNotEqual:
      out << "not_equal";
      break;
    case Relation::kLessThan:
      out << "less_than";
      break;
    case Relation::kGreaterThan:
      out << "greater_than";
      break;
    case Relation::kLessThanEqual:
      out << "less_than_equal";
      break;
    case Relation::kGreaterThanEqual:
      out << "greater_than_equal";
      break;
    case Relation::kShiftLeft:
      out << "shift_left";
      break;
    case Relation::kShiftRight:
      out << "shift_right";
      break;
    case Relation::kShiftRightArith:
      out << "shift_right_arith";
      break;
    case Relation::kAdd:
      out << "add";
      break;
    case Relation::kSubtract:
      out << "subtract";
      break;
    case Relation::kMultiply:
      out << "multiply";
      break;
    case Relation::kDivide:
      out << "divide";
      break;
    case Relation::kModulo:
      out << "modulo";
      break;
  }
  return out;
}

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_RELATIONAL_EXPRESSION_H_
