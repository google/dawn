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

#ifndef SRC_TINT_LANG_CORE_IR_CONVERT_H_
#define SRC_TINT_LANG_CORE_IR_CONVERT_H_

#include <string>

#include "src/tint/lang/core/ir/call.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::core::ir {

/// A value conversion instruction in the IR.
class Convert final : public Castable<Convert, Call> {
  public:
    /// The offset in Operands() for the value
    static constexpr size_t kValueOperandOffset = 0;

    /// Constructor
    /// @param result the result value
    /// @param value the value to convert
    Convert(InstructionResult* result, Value* value);
    ~Convert() override;

    /// @copydoc Instruction::Clone()
    Convert* Clone(CloneContext& ctx) override;

    /// @returns the friendly name for the instruction
    std::string FriendlyName() override { return "convert"; }
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_CONVERT_H_
