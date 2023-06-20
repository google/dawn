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

#ifndef SRC_TINT_IR_LOOP_H_
#define SRC_TINT_IR_LOOP_H_

#include "src/tint/ir/control_instruction.h"

// Forward declarations
namespace tint::ir {
class MultiInBlock;
}  // namespace tint::ir

namespace tint::ir {

/// Loop instruction.
///
/// ```
///                     in
///                      ┃
///                      ┣━━━━━━━━━━━┓
///                      ▼           ┃
///             ┌─────────────────┐  ┃
///             │   Initializer   │  ┃
///             │    (optional)   │  ┃
///             └─────────────────┘  ┃
///        NextIteration ┃           ┃
///                      ┃◀━━━━━━━━━━┫
///                      ▼           ┃
///             ┌─────────────────┐  ┃
///          ┏━━│       Body      │  ┃
///          ┃  └─────────────────┘  ┃
///          ┃  Continue ┃           ┃ NextIteration
///          ┃           ▼           ┃
///          ┃  ┌─────────────────┐  ┃ BreakIf(false)
/// ExitLoop ┃  │   Continuing    │━━┛
///             │  (optional)     │
///          ┃  └─────────────────┘
///          ┃           ┃
///          ┃           ┃ BreakIf(true)
///          ┗━━━━━━━━━━▶┃
///                      ▼
///                     out
///
/// ```
class Loop : public utils::Castable<Loop, ControlInstruction> {
  public:
    /// Constructor
    /// @param i the initializer block
    /// @param b the body block
    /// @param c the continuing block
    Loop(ir::Block* i, ir::MultiInBlock* b, ir::MultiInBlock* c);
    ~Loop() override;

    /// @copydoc ControlInstruction::ForeachBlock
    void ForeachBlock(const std::function<void(ir::Block*)>& cb) override;

    /// @returns the switch initializer block
    ir::Block* Initializer() { return initializer_; }

    /// @returns true if the loop uses an initializer block. If true, then the Loop first branches
    /// to the initializer block, otherwise it first branches to the body block.
    bool HasInitializer();

    /// @returns the switch start block
    ir::MultiInBlock* Body() { return body_; }

    /// @returns the switch continuing block
    ir::MultiInBlock* Continuing() { return continuing_; }

  private:
    ir::Block* initializer_ = nullptr;
    ir::MultiInBlock* body_ = nullptr;
    ir::MultiInBlock* continuing_ = nullptr;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_LOOP_H_
