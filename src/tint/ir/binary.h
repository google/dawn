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

#ifndef SRC_TINT_IR_BINARY_H_
#define SRC_TINT_IR_BINARY_H_

#include <ostream>

#include "src/tint/castable.h"
#include "src/tint/ir/instruction.h"
#include "src/tint/symbol_table.h"
#include "src/tint/type/type.h"

namespace tint::ir {

/// An instruction in the IR.
class Binary : public Castable<Binary, Instruction> {
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
    /// @param kind the kind of binary instruction
    /// @param result the result value
    /// @param lhs the lhs of the instruction
    /// @param rhs the rhs of the instruction
    Binary(Kind kind, Value* result, Value* lhs, Value* rhs);
    Binary(const Binary& instr) = delete;
    Binary(Binary&& instr) = delete;
    ~Binary() override;

    Binary& operator=(const Binary& instr) = delete;
    Binary& operator=(Binary&& instr) = delete;

    /// @returns the kind of instruction
    Kind GetKind() const { return kind_; }

    /// @returns the left-hand-side value for the instruction
    const Value* LHS() const { return lhs_; }

    /// @returns the right-hand-side value for the instruction
    const Value* RHS() const { return rhs_; }

    /// Write the instruction to the given stream
    /// @param out the stream to write to
    /// @param st the symbol table
    /// @returns the stream
    std::ostream& ToString(std::ostream& out, const SymbolTable& st) const override;

  private:
    Kind kind_;
    Value* lhs_ = nullptr;
    Value* rhs_ = nullptr;
};

std::ostream& operator<<(std::ostream& out, const Binary&);

}  // namespace tint::ir

#endif  // SRC_TINT_IR_BINARY_H_
