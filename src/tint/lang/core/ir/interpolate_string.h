// Copyright 2026 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_CORE_IR_INTERPOLATE_STRING_H_
#define SRC_TINT_LANG_CORE_IR_INTERPOLATE_STRING_H_

#include <span>
#include <string>

#include "src/tint/lang/core/ir/operand_instruction.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::core::ir {

/// An interpolate string instruction in the IR.
class InterpolateString final : public Castable<InterpolateString, OperandInstruction<4, 1>> {
  public:
    /// The fixed number of results returned by this instruction
    static constexpr size_t kNumResults = 1;

    /// The minimum number of operands expected for this instruction
    static constexpr size_t kMinOperands = 0;

    /// Constructor (no results, no operands)
    /// @param id the instruction id
    explicit InterpolateString(Id id);

    /// Constructor
    /// @param id the instruction id
    /// @param result the result value
    /// @param operands the interpolate string operands
    InterpolateString(Id id, InstructionResult* result, VectorRef<Value*> operands = tint::Empty);

    ~InterpolateString() override;

    /// @copydoc Instruction::Clone()
    InterpolateString* Clone(CloneContext& ctx) override;

    /// @returns the interpolate string elements
    std::span<Value* const> Elements() { return operands_.AsSpan(); }

    /// @returns the interpolate string elements
    std::span<const Value* const> Elements() const { return operands_.AsSpan(); }

    /// @returns the friendly name for the instruction
    std::string FriendlyName() const override { return "interpolate_string"; }
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_INTERPOLATE_STRING_H_
