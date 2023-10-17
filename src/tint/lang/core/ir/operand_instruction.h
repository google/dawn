// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_LANG_CORE_IR_OPERAND_INSTRUCTION_H_
#define SRC_TINT_LANG_CORE_IR_OPERAND_INSTRUCTION_H_

#include <utility>

#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/instruction_result.h"
#include "src/tint/utils/ice/ice.h"

namespace tint::core::ir {

/// An instruction in the IR that expects one or more operands.
/// @tparam N the number of operands before spilling to the heap
/// @tparam R the number of result values before spilling to the heap
template <unsigned N, unsigned R>
class OperandInstruction : public Castable<OperandInstruction<N, R>, Instruction> {
  public:
    /// Destructor
    ~OperandInstruction() override = default;

    /// @copydoc tint::core::ir::Value::Destroy
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

    /// Removes all results from the instruction.
    void ClearResults() { results_.Clear(); }

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

    /// The default number of operands
    static constexpr size_t kDefaultNumOperands = N;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_OPERAND_INSTRUCTION_H_
