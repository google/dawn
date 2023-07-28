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

#ifndef SRC_TINT_LANG_CORE_IR_OPERAND_INSTRUCTION_H_
#define SRC_TINT_LANG_CORE_IR_OPERAND_INSTRUCTION_H_

#include <utility>

#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/instruction_result.h"
#include "src/tint/utils/ice/ice.h"

namespace tint::ir {

/// An instruction in the IR that expects one or more operands.
/// @tparam N the number of operands before spilling to the heap
/// @tparam R the number of result values before spilling to the heap
template <unsigned N, unsigned R>
class OperandInstruction : public Castable<OperandInstruction<N, R>, Instruction> {
  public:
    /// Destructor
    ~OperandInstruction() override = default;

    /// @copydoc tint::ir::Value::Destroy
    void Destroy() override {
        ClearOperands();
        Instruction::Destroy();
    }

    /// Set an operand at a given index.
    /// @param index the operand index
    /// @param value the value to use
    void SetOperand(size_t index, ir::Value* value) override {
        TINT_ASSERT(index < operands_.Length());
        if (operands_[index]) {
            operands_[index]->RemoveUsage({this, index});
        }
        operands_[index] = value;
        if (value) {
            value->AddUsage({this, index});
        }
    }

    /// Sets the operands to @p operands
    /// @param operands the new operands for the instruction
    void SetOperands(VectorRef<ir::Value*> operands) {
        ClearOperands();
        operands_ = std::move(operands);
        for (size_t i = 0; i < operands_.Length(); i++) {
            if (operands_[i]) {
                operands_[i]->AddUsage({this, static_cast<uint32_t>(i)});
            }
        }
    }

    /// Removes all operands from the instruction
    void ClearOperands() {
        for (uint32_t i = 0; i < operands_.Length(); i++) {
            if (!operands_[i]) {
                continue;
            }
            operands_[i]->RemoveUsage({this, i});
        }
        operands_.Clear();
    }

    /// @returns the operands of the instruction
    VectorRef<ir::Value*> Operands() override { return operands_; }

    /// @returns true if the instruction has result values
    bool HasResults() override { return !results_.IsEmpty(); }
    /// @returns true if the instruction has multiple values
    bool HasMultiResults() override { return results_.Length() > 1; }

    /// @returns the first result. Returns `nullptr` if there are no results, or if ther are
    /// multi-results
    InstructionResult* Result() override {
        if (!HasResults() || HasMultiResults()) {
            return nullptr;
        }
        return results_[0];
    }

    using Instruction::Result;

    /// @returns the result values for this instruction
    VectorRef<InstructionResult*> Results() override { return results_; }

  protected:
    /// Append a new operand to the operand list for this instruction.
    /// @param idx the index the operand should be at
    /// @param value the operand value to append
    void AddOperand(size_t idx, ir::Value* value) {
        TINT_ASSERT(idx == operands_.Length());

        if (value) {
            value->AddUsage({this, static_cast<uint32_t>(operands_.Length())});
        }
        operands_.Push(value);
    }

    /// Append a list of operands to the operand list for this instruction.
    /// @param start_idx the index from which the values should start
    /// @param values the operand values to append
    void AddOperands(size_t start_idx, VectorRef<ir::Value*> values) {
        size_t idx = start_idx;
        for (auto* val : values) {
            AddOperand(idx, val);
            idx += 1;
        }
    }

    /// Appends a result value to the instruction
    /// @param value the value to append
    void AddResult(InstructionResult* value) {
        if (value) {
            value->SetSource(this);
        }
        results_.Push(value);
    }

    /// The operands to this instruction.
    Vector<ir::Value*, N> operands_;
    /// The results of this instruction.
    Vector<ir::InstructionResult*, R> results_;
};

}  // namespace tint::ir

#endif  // SRC_TINT_LANG_CORE_IR_OPERAND_INSTRUCTION_H_
