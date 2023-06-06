// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_IR_OPERAND_INSTRUCTION_H_
#define SRC_TINT_IR_OPERAND_INSTRUCTION_H_

#include "src/tint/ir/instruction.h"

namespace tint::ir {

/// An instruction in the IR that expects one or more operands.
template <unsigned N>
class OperandInstruction : public utils::Castable<OperandInstruction<N>, Instruction> {
  public:
    /// Destructor
    ~OperandInstruction() override = default;

    /// Set an operand at a given index.
    /// @param index the operand index
    /// @param value the value to use
    void SetOperand(uint32_t index, ir::Value* value) override {
        TINT_ASSERT(IR, index < operands_.Length());
        if (operands_[index]) {
            operands_[index]->RemoveUsage({this, index});
        }
        operands_[index] = value;
        if (value) {
            value->AddUsage({this, index});
        }
        return;
    }

  protected:
    /// Append a new operand to the operand list for this instruction.
    /// @param value the operand value to append
    void AddOperand(ir::Value* value) {
        if (value) {
            value->AddUsage({this, static_cast<uint32_t>(operands_.Length())});
        }
        operands_.Push(value);
    }

    /// Append a list of non-null operands to the operand list for this instruction.
    /// @param values the operand values to append
    void AddOperands(utils::VectorRef<ir::Value*> values) {
        for (auto* val : values) {
            TINT_ASSERT(IR, val != nullptr);
            AddOperand(val);
        }
    }

    /// The operands to this instruction.
    utils::Vector<ir::Value*, N> operands_;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_OPERAND_INSTRUCTION_H_
