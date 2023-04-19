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

#ifndef SRC_TINT_AST_BINARY_EXPRESSION_H_
#define SRC_TINT_AST_BINARY_EXPRESSION_H_

#include "src/tint/ast/expression.h"

namespace tint::ast {

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
class BinaryExpression final : public utils::Castable<BinaryExpression, Expression> {
  public:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param source the binary expression source
    /// @param op the operation type
    /// @param lhs the left side of the expression
    /// @param rhs the right side of the expression
    BinaryExpression(ProgramID pid,
                     NodeID nid,
                     const Source& source,
                     BinaryOp op,
                     const Expression* lhs,
                     const Expression* rhs);
    /// Move constructor
    BinaryExpression(BinaryExpression&&);
    ~BinaryExpression() override;

    /// @returns true if the op is and
    bool IsAnd() const { return op == BinaryOp::kAnd; }
    /// @returns true if the op is or
    bool IsOr() const { return op == BinaryOp::kOr; }
    /// @returns true if the op is xor
    bool IsXor() const { return op == BinaryOp::kXor; }
    /// @returns true if the op is logical and
    bool IsLogicalAnd() const { return op == BinaryOp::kLogicalAnd; }
    /// @returns true if the op is logical or
    bool IsLogicalOr() const { return op == BinaryOp::kLogicalOr; }
    /// @returns true if the op is equal
    bool IsEqual() const { return op == BinaryOp::kEqual; }
    /// @returns true if the op is not equal
    bool IsNotEqual() const { return op == BinaryOp::kNotEqual; }
    /// @returns true if the op is less than
    bool IsLessThan() const { return op == BinaryOp::kLessThan; }
    /// @returns true if the op is greater than
    bool IsGreaterThan() const { return op == BinaryOp::kGreaterThan; }
    /// @returns true if the op is less than equal
    bool IsLessThanEqual() const { return op == BinaryOp::kLessThanEqual; }
    /// @returns true if the op is greater than equal
    bool IsGreaterThanEqual() const { return op == BinaryOp::kGreaterThanEqual; }
    /// @returns true if the op is shift left
    bool IsShiftLeft() const { return op == BinaryOp::kShiftLeft; }
    /// @returns true if the op is shift right
    bool IsShiftRight() const { return op == BinaryOp::kShiftRight; }
    /// @returns true if the op is add
    bool IsAdd() const { return op == BinaryOp::kAdd; }
    /// @returns true if the op is subtract
    bool IsSubtract() const { return op == BinaryOp::kSubtract; }
    /// @returns true if the op is multiply
    bool IsMultiply() const { return op == BinaryOp::kMultiply; }
    /// @returns true if the op is divide
    bool IsDivide() const { return op == BinaryOp::kDivide; }
    /// @returns true if the op is modulo
    bool IsModulo() const { return op == BinaryOp::kModulo; }
    /// @returns true if the op is an arithmetic operation
    bool IsArithmetic() const;
    /// @returns true if the op is a comparison operation
    bool IsComparison() const;
    /// @returns true if the op is a bitwise operation
    bool IsBitwise() const;
    /// @returns true if the op is a bit shift operation
    bool IsBitshift() const;
    /// @returns true if the op is a logical expression
    bool IsLogical() const;

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const BinaryExpression* Clone(CloneContext* ctx) const override;

    /// the binary op type
    const BinaryOp op;
    /// the left side expression
    const Expression* const lhs;
    /// the right side expression
    const Expression* const rhs;
};

/// @param op the operator
/// @returns true if the op is an arithmetic operation
inline bool IsArithmetic(BinaryOp op) {
    switch (op) {
        case BinaryOp::kAdd:
        case BinaryOp::kSubtract:
        case BinaryOp::kMultiply:
        case BinaryOp::kDivide:
        case BinaryOp::kModulo:
            return true;
        default:
            return false;
    }
}

/// @param op the operator
/// @returns true if the op is a comparison operation
inline bool IsComparison(BinaryOp op) {
    switch (op) {
        case BinaryOp::kEqual:
        case BinaryOp::kNotEqual:
        case BinaryOp::kLessThan:
        case BinaryOp::kLessThanEqual:
        case BinaryOp::kGreaterThan:
        case BinaryOp::kGreaterThanEqual:
            return true;
        default:
            return false;
    }
}

/// @param op the operator
/// @returns true if the op is a bitwise operation
inline bool IsBitwise(BinaryOp op) {
    switch (op) {
        case BinaryOp::kAnd:
        case BinaryOp::kOr:
        case BinaryOp::kXor:
            return true;
        default:
            return false;
    }
}

/// @param op the operator
/// @returns true if the op is a bit shift operation
inline bool IsBitshift(BinaryOp op) {
    switch (op) {
        case BinaryOp::kShiftLeft:
        case BinaryOp::kShiftRight:
            return true;
        default:
            return false;
    }
}

inline bool BinaryExpression::IsLogical() const {
    switch (op) {
        case BinaryOp::kLogicalAnd:
        case BinaryOp::kLogicalOr:
            return true;
        default:
            return false;
    }
}

inline bool BinaryExpression::IsArithmetic() const {
    return ast::IsArithmetic(op);
}

inline bool BinaryExpression::IsComparison() const {
    return ast::IsComparison(op);
}

inline bool BinaryExpression::IsBitwise() const {
    return ast::IsBitwise(op);
}

inline bool BinaryExpression::IsBitshift() const {
    return ast::IsBitshift(op);
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
    return "<invalid>";
}

/// @returns the WGSL operator of the BinaryOp
/// @param op the BinaryOp
constexpr const char* Operator(BinaryOp op) {
    switch (op) {
        case BinaryOp::kAnd:
            return "&";
        case BinaryOp::kOr:
            return "|";
        case BinaryOp::kXor:
            return "^";
        case BinaryOp::kLogicalAnd:
            return "&&";
        case BinaryOp::kLogicalOr:
            return "||";
        case BinaryOp::kEqual:
            return "==";
        case BinaryOp::kNotEqual:
            return "!=";
        case BinaryOp::kLessThan:
            return "<";
        case BinaryOp::kGreaterThan:
            return ">";
        case BinaryOp::kLessThanEqual:
            return "<=";
        case BinaryOp::kGreaterThanEqual:
            return ">=";
        case BinaryOp::kShiftLeft:
            return "<<";
        case BinaryOp::kShiftRight:
            return ">>";
        case BinaryOp::kAdd:
            return "+";
        case BinaryOp::kSubtract:
            return "-";
        case BinaryOp::kMultiply:
            return "*";
        case BinaryOp::kDivide:
            return "/";
        case BinaryOp::kModulo:
            return "%";
        default:
            break;
    }
    return "<invalid>";
}

/// @param out the stream to write to
/// @param op the BinaryOp
/// @return the stream so calls can be chained
inline utils::StringStream& operator<<(utils::StringStream& out, BinaryOp op) {
    out << FriendlyName(op);
    return out;
}

}  // namespace tint::ast

#endif  // SRC_TINT_AST_BINARY_EXPRESSION_H_
