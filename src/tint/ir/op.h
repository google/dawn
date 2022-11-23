// Copyright 2022 The Tint Authors.
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

#ifndef SRC_TINT_IR_OP_H_
#define SRC_TINT_IR_OP_H_

#include <ostream>

#include "src/tint/ir/register.h"
#include "src/tint/utils/vector.h"

namespace tint::ir {

/// An operation in the IR.
class Op {
  public:
    /// The kind of operation.
    enum class Kind {
        kAdd,
        kSubtract,
        kMultiply,
        kDivide,
        kModulo,

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
        kShiftRight
    };

    /// Constructor
    Op();
    /// Constructor
    /// @param kind the kind of operation
    /// @param result the result register
    /// @param lhs the lhs of the operation
    /// @param rhs the rhs of the operation
    Op(Kind kind, Register result, Register lhs, Register rhs);
    /// Copy constructor
    /// @param o the op to copy from
    Op(const Op& o);
    /// Move constructor
    /// @param o the op to move from
    Op(Op&& o);
    /// Destructor
    ~Op();

    /// Copy assign
    /// @param o the op to copy from
    /// @returns a reference to this
    Op& operator=(const Op& o);
    /// Move assign
    /// @param o the op to move from
    /// @returns a reference to this
    Op& operator=(Op&& o);

    /// @returns the kind of operation
    Kind GetKind() const { return kind_; }

    /// @returns the result register for the operation
    const Register& Result() const { return result_; }

    /// @returns true if the op has a LHS
    bool HasLHS() const { return args_.Length() >= 1; }
    /// @returns the left-hand-side register for the operation
    const Register& LHS() const {
        TINT_ASSERT(IR, HasLHS());
        return args_[0];
    }

    /// @returns true if the op has a RHS
    bool HasRHS() const { return args_.Length() >= 2; }
    /// @returns the right-hand-side register for the operation
    const Register& RHS() const {
        TINT_ASSERT(IR, HasRHS());
        return args_[1];
    }

  private:
    Kind kind_;

    Register result_;
    utils::Vector<Register, 2> args_;
};

std::ostream& operator<<(std::ostream& out, const Op&);

}  // namespace tint::ir

#endif  // SRC_TINT_IR_OP_H_
