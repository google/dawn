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

#ifndef SRC_TINT_LANG_CORE_IR_BREAK_IF_H_
#define SRC_TINT_LANG_CORE_IR_BREAK_IF_H_

#include <string>

#include "src/tint/lang/core/ir/terminator.h"
#include "src/tint/lang/core/ir/value.h"
#include "src/tint/utils/rtti/castable.h"

// Forward declarations
namespace tint::core::ir {
class Loop;
}  // namespace tint::core::ir

namespace tint::core::ir {

/// A break-if iteration instruction.
class BreakIf final : public Castable<BreakIf, Terminator> {
  public:
    /// The offset in Operands() for the condition
    static constexpr size_t kConditionOperandOffset = 0;

    /// The base offset in Operands() for the arguments
    static constexpr size_t kArgsOperandOffset = 1;

    /// Constructor
    /// @param condition the break condition
    /// @param loop the loop containing the break-if
    /// @param args the MultiInBlock arguments
    BreakIf(Value* condition, ir::Loop* loop, VectorRef<Value*> args = tint::Empty);
    ~BreakIf() override;

    /// @copydoc Instruction::Clone()
    BreakIf* Clone(CloneContext& ctx) override;

    /// @returns the MultiInBlock arguments
    tint::Slice<Value* const> Args() override {
        return operands_.Slice().Offset(kArgsOperandOffset);
    }

    /// @returns the break condition
    Value* Condition() { return operands_[kConditionOperandOffset]; }

    /// @returns the loop containing the break-if
    ir::Loop* Loop() { return loop_; }

    /// @returns the friendly name for the instruction
    std::string FriendlyName() override { return "break_if"; }

  private:
    ir::Loop* loop_ = nullptr;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_BREAK_IF_H_
