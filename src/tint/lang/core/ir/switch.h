// Copyright 2022 The Tint Authors.
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

#ifndef SRC_TINT_LANG_CORE_IR_SWITCH_H_
#define SRC_TINT_LANG_CORE_IR_SWITCH_H_

#include <string>

#include "src/tint/lang/core/ir/control_instruction.h"

// Forward declarations
namespace tint::core::ir {
class Constant;
class MultiInBlock;
}  // namespace tint::core::ir

namespace tint::core::ir {
/// Switch instruction.
///
/// ```
///                           in
///                            ┃
///     ╌╌╌╌╌╌╌╌┲━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━┱╌╌╌╌╌╌╌╌
///             ▼              ▼              ▼
///        ┌────────┐     ┌────────┐     ┌────────┐
///        │ Case A │     │ Case B │     │ Case C │
///        └────────┘     └────────┘     └────────┘
///  ExitSwitch ┃   ExitSwitch ┃   ExitSwitch ┃
///             ┃              ┃              ┃
///     ╌╌╌╌╌╌╌╌┺━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━┹╌╌╌╌╌╌╌╌
///                            ┃
///                            ▼
///                           out
/// ```
class Switch final : public Castable<Switch, ControlInstruction> {
  public:
    /// The offset in Operands() for the condition
    static constexpr size_t kConditionOperandOffset = 0;

    /// A case selector
    struct CaseSelector {
        /// @returns true if this is a default selector
        bool IsDefault() { return val == nullptr; }

        /// The selector value, or nullptr if this is the default selector
        Constant* val = nullptr;
    };

    /// A case label in the struct
    struct Case {
        /// The case selector for this node
        Vector<CaseSelector, 4> selectors;
        /// The case block.
        ir::Block* block = nullptr;

        /// @returns the case block
        ir::Block* Block() { return block; }
    };

    /// Constructor
    /// @param cond the condition
    explicit Switch(Value* cond);
    ~Switch() override;

    /// @copydoc Instruction::Clone()
    Switch* Clone(CloneContext& ctx) override;

    /// @copydoc ControlInstruction::ForeachBlock
    void ForeachBlock(const std::function<void(ir::Block*)>& cb) override;

    /// @returns the switch cases
    Vector<Case, 4>& Cases() { return cases_; }

    /// @returns the condition
    Value* Condition() { return operands_[kConditionOperandOffset]; }

    /// @returns the friendly name for the instruction
    std::string FriendlyName() override { return "switch"; }

  private:
    Vector<Case, 4> cases_;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_SWITCH_H_
