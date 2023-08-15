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

#ifndef SRC_TINT_LANG_CORE_IR_STORE_VECTOR_ELEMENT_H_
#define SRC_TINT_LANG_CORE_IR_STORE_VECTOR_ELEMENT_H_

#include "src/tint/lang/core/ir/operand_instruction.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::core::ir {

/// A store instruction for a single vector element in the IR.
class StoreVectorElement : public Castable<StoreVectorElement, OperandInstruction<3, 0>> {
  public:
    /// The offset in Operands() for the `to` value
    static constexpr size_t kToOperandOffset = 0;

    /// The offset in Operands() for the `index` value
    static constexpr size_t kIndexOperandOffset = 1;

    /// The offset in Operands() for the `value` value
    static constexpr size_t kValueOperandOffset = 2;

    /// Constructor
    /// @param to the vector pointer
    /// @param index the new vector element index
    /// @param value the new vector element value
    StoreVectorElement(ir::Value* to, ir::Value* index, ir::Value* value);
    ~StoreVectorElement() override;

    /// @returns the vector pointer value
    ir::Value* To() { return operands_[kToOperandOffset]; }

    /// @returns the new vector element index
    ir::Value* Index() { return operands_[kIndexOperandOffset]; }

    /// @returns the new vector element value
    ir::Value* Value() { return operands_[kValueOperandOffset]; }

    /// @returns the friendly name for the instruction
    std::string_view FriendlyName() override { return "store-vector-element"; }
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_STORE_VECTOR_ELEMENT_H_
