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

#ifndef SRC_TINT_LANG_CORE_IR_LET_H_
#define SRC_TINT_LANG_CORE_IR_LET_H_

#include <string>

#include "src/tint/lang/core/ir/operand_instruction.h"

namespace tint::core::ir {

/// A no-op instruction in the IR, used to position and name a value
class Let : public Castable<Let, OperandInstruction<1, 1>> {
  public:
    /// The offset in Operands() for the value
    static constexpr size_t kValueOperandOffset = 0;

    /// Constructor
    /// @param result the result value
    /// @param value the let's value
    Let(InstructionResult* result, Value* value);
    ~Let() override;

    /// @returns the value
    ir::Value* Value() { return operands_[kValueOperandOffset]; }

    /// @returns the friendly name for the instruction
    std::string FriendlyName() override { return "let"; }
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_LET_H_
