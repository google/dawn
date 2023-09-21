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

#ifndef SRC_TINT_LANG_CORE_IR_ACCESS_H_
#define SRC_TINT_LANG_CORE_IR_ACCESS_H_

#include <string>

#include "src/tint/lang/core/ir/operand_instruction.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::core::ir {

/// An access instruction in the IR.
class Access final : public Castable<Access, OperandInstruction<3, 1>> {
  public:
    /// The offset in Operands() for the object being accessed
    static constexpr size_t kObjectOperandOffset = 0;

    /// The base offset in Operands() for the access indices
    static constexpr size_t kIndicesOperandOffset = 1;

    /// Constructor
    /// @param result the result value
    /// @param object the accessor object
    /// @param indices the indices to access
    Access(InstructionResult* result, Value* object, VectorRef<Value*> indices);
    ~Access() override;

    /// @copydoc Instruction::Clone()
    Access* Clone(CloneContext& ctx) override;

    /// @returns the object used for the access
    Value* Object() { return operands_[kObjectOperandOffset]; }

    /// Adds the given index to the end of the access chain
    /// @param idx the index to add
    void AddIndex(Value* idx) { AddOperand(operands_.Length(), idx); }

    /// @returns the accessor indices
    tint::Slice<Value*> Indices() { return operands_.Slice().Offset(kIndicesOperandOffset); }

    /// @returns the friendly name for the instruction
    std::string FriendlyName() override { return "access"; }
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_ACCESS_H_
