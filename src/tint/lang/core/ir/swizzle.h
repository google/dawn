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

#ifndef SRC_TINT_LANG_CORE_IR_SWIZZLE_H_
#define SRC_TINT_LANG_CORE_IR_SWIZZLE_H_

#include <string>

#include "src/tint/lang/core/ir/operand_instruction.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::core::ir {

/// A swizzle instruction in the IR.
class Swizzle final : public Castable<Swizzle, OperandInstruction<1, 1>> {
  public:
    /// The offset in Operands() for the object being swizzled
    static constexpr size_t kObjectOperandOffset = 0;

    /// Constructor
    /// @param result the result value
    /// @param object the object being swizzled
    /// @param indices the indices to swizzle
    Swizzle(InstructionResult* result, Value* object, VectorRef<uint32_t> indices);
    ~Swizzle() override;

    /// @copydoc Instruction::Clone()
    Swizzle* Clone(CloneContext& ctx) override;

    /// @returns the object used for the access
    Value* Object() { return operands_[kObjectOperandOffset]; }

    /// @returns the swizzle indices
    VectorRef<uint32_t> Indices() { return indices_; }

    /// @returns the friendly name for the instruction
    std::string FriendlyName() override { return "swizzle"; }

  private:
    Vector<uint32_t, 4> indices_;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_SWIZZLE_H_
