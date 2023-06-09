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

#ifndef SRC_TINT_IR_IF_H_
#define SRC_TINT_IR_IF_H_

#include "src/tint/ir/control_instruction.h"

// Forward declarations
namespace tint::ir {
class MultiInBlock;
}  // namespace tint::ir

namespace tint::ir {

/// If instruction.
///
/// ```
///                   in
///                    ┃
///         ┏━━━━━━━━━━┻━━━━━━━━━━┓
///         ▼                     ▼
///    ┌────────────┐      ┌────────────┐
///    │  True      │      │  False     │
///    | (optional) |      | (optional) |
///    └────────────┘      └────────────┘
///  ExitIf ┃     ┌──────────┐     ┃ ExitIf
///         ┗━━━━▶│  Merge   │◀━━━━┛
///               │(optional)│
///               └──────────┘
///                    ┃
///                    ▼
///                   out
/// ```
class If : public utils::Castable<If, ControlInstruction> {
  public:
    /// The index of the condition operand
    static constexpr size_t kConditionOperandOffset = 0;

    /// Constructor
    /// @param cond the if condition
    /// @param t the true block
    /// @param f the false block
    /// @param m the merge block
    If(Value* cond, ir::Block* t, ir::Block* f, ir::MultiInBlock* m);
    ~If() override;

    /// @returns the branch arguments
    utils::Slice<Value* const> Args() override { return utils::Slice<Value*>{}; }

    /// @returns the if condition
    Value* Condition() { return operands_[kConditionOperandOffset]; }

    /// @returns the true branch block
    ir::Block* True() { return true_; }

    /// @returns the false branch block
    ir::Block* False() { return false_; }

    /// @returns the merge branch block
    ir::MultiInBlock* Merge() { return merge_; }

  private:
    ir::Block* true_ = nullptr;
    ir::Block* false_ = nullptr;
    ir::MultiInBlock* merge_ = nullptr;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_IF_H_
