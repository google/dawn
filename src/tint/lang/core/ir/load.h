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

#ifndef SRC_TINT_LANG_CORE_IR_LOAD_H_
#define SRC_TINT_LANG_CORE_IR_LOAD_H_

#include <string>

#include "src/tint/lang/core/ir/operand_instruction.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::core::ir {

/// A load instruction in the IR.
class Load : public Castable<Load, OperandInstruction<1, 1>> {
  public:
    /// The offset in Operands() for the from value
    static constexpr size_t kFromOperandOffset = 0;

    /// Constructor (infers type)
    /// @param result the result value
    /// @param from the value being loaded from
    Load(InstructionResult* result, Value* from);

    ~Load() override;

    /// @returns the value being loaded from
    Value* From() { return operands_[kFromOperandOffset]; }

    /// @returns the friendly name for the instruction
    std::string FriendlyName() override { return "load"; }
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_LOAD_H_
