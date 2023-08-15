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

#ifndef SRC_TINT_LANG_CORE_IR_CONTROL_INSTRUCTION_H_
#define SRC_TINT_LANG_CORE_IR_CONTROL_INSTRUCTION_H_

#include <utility>

#include "src/tint/lang/core/ir/operand_instruction.h"

// Forward declarations
namespace tint::core::ir {
class Block;
class Exit;
}  // namespace tint::core::ir

namespace tint::core::ir {

/// Base class of instructions that perform control flow to two or more blocks, owned by the
/// ControlInstruction.
class ControlInstruction : public Castable<ControlInstruction, OperandInstruction<1, 1>> {
  public:
    /// Constructor
    ControlInstruction();

    /// Destructor
    ~ControlInstruction() override;

    /// Calls @p cb for each block owned by this control instruction
    /// @param cb the function to call once for each block
    virtual void ForeachBlock(const std::function<void(ir::Block*)>& cb) = 0;

    /// Sets the results of the control instruction
    /// @param values the new result values
    void SetResults(VectorRef<InstructionResult*> values) {
        for (auto* value : results_) {
            if (value) {
                value->SetSource(nullptr);
            }
        }
        results_ = std::move(values);
        for (auto* value : results_) {
            if (value) {
                value->SetSource(this);
            }
        }
    }

    /// Sets the results of the control instruction
    /// @param values the new result values
    template <typename... ARGS,
              typename = std::enable_if_t<!tint::IsVectorLike<
                  tint::traits::Decay<tint::traits::NthTypeOf<0, ARGS..., void>>>>>
    void SetResults(ARGS&&... values) {
        SetResults(Vector{std::forward<ARGS>(values)...});
    }

    /// @return All the exits for the flow control instruction
    const Hashset<Exit*, 2>& Exits() const { return exits_; }

    /// Adds the exit to the flow control instruction
    /// @param exit the exit instruction
    void AddExit(Exit* exit);

    /// Removes the exit to the flow control instruction
    /// @param exit the exit instruction
    void RemoveExit(Exit* exit);

  protected:
    /// The flow control exits
    Hashset<Exit*, 2> exits_;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_CONTROL_INSTRUCTION_H_
