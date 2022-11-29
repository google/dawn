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

#ifndef SRC_TINT_IR_INSTRUCTION_H_
#define SRC_TINT_IR_INSTRUCTION_H_

#include <ostream>

#include "src/tint/castable.h"
#include "src/tint/debug.h"
#include "src/tint/ir/value.h"
#include "src/tint/utils/vector.h"

namespace tint::ir {

/// An instruction in the IR.
class Instruction : public Castable<Instruction> {
  public:
    /// The kind of instruction.
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
    Instruction();
    /// Constructor
    /// @param kind the kind of instruction
    /// @param result the result value
    /// @param lhs the lhs of the instruction
    /// @param rhs the rhs of the instruction
    Instruction(Kind kind, const Value* result, const Value* lhs, const Value* rhs);
    /// Copy constructor
    /// @param instr the instruction to copy from
    Instruction(const Instruction& instr);
    /// Move constructor
    /// @param instr the instruction to move from
    Instruction(Instruction&& instr);
    /// Destructor
    ~Instruction();

    /// Copy assign
    /// @param instr the instruction to copy from
    /// @returns a reference to this
    Instruction& operator=(const Instruction& instr);
    /// Move assign
    /// @param instr the instruction to move from
    /// @returns a reference to this
    Instruction& operator=(Instruction&& instr);

    /// @returns the kind of instruction
    Kind GetKind() const { return kind_; }

    /// @returns the result value for the instruction
    const Value* Result() const { return result_; }

    /// @returns true if the instruction has a LHS
    bool HasLHS() const { return args_.Length() >= 1; }
    /// @returns the left-hand-side value for the instruction
    const Value* LHS() const {
        TINT_ASSERT(IR, HasLHS());
        return args_[0];
    }

    /// @returns true if the instruction has a RHS
    bool HasRHS() const { return args_.Length() >= 2; }
    /// @returns the right-hand-side value for the instruction
    const Value* RHS() const {
        TINT_ASSERT(IR, HasRHS());
        return args_[1];
    }

  private:
    Kind kind_;

    const Value* result_;
    utils::Vector<const Value*, 2> args_;
};

std::ostream& operator<<(std::ostream& out, const Instruction&);

}  // namespace tint::ir

#endif  // SRC_TINT_IR_INSTRUCTION_H_
