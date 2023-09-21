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

#ifndef SRC_TINT_LANG_CORE_IR_INSTRUCTION_H_
#define SRC_TINT_LANG_CORE_IR_INSTRUCTION_H_

#include <string>

#include "src/tint/lang/core/ir/instruction_result.h"
#include "src/tint/lang/core/ir/value.h"
#include "src/tint/utils/containers/enum_set.h"
#include "src/tint/utils/rtti/castable.h"

// Forward declarations
namespace tint::core::ir {
class Block;
class CloneContext;
}  // namespace tint::core::ir

namespace tint::core::ir {

/// An instruction in the IR.
class Instruction : public Castable<Instruction> {
  public:
    /// Destructor
    ~Instruction() override;

    /// Set an operand at a given index.
    /// @param index the operand index
    /// @param value the value to use
    virtual void SetOperand(size_t index, ir::Value* value) = 0;

    /// @returns the operands of the instruction
    virtual VectorRef<ir::Value*> Operands() = 0;

    /// @returns true if the instruction has result values
    virtual bool HasResults() { return false; }
    /// @returns true if the instruction has multiple values
    virtual bool HasMultiResults() { return false; }

    /// @returns the first result. Returns `nullptr` if there are no results, or if ther are
    /// multi-results
    virtual InstructionResult* Result() { return nullptr; }

    /// @returns the result values for this instruction
    virtual VectorRef<InstructionResult*> Results() { return tint::Empty; }

    /// Removes the instruction from the block, and destroys all the result values.
    /// The result values must not be in use.
    virtual void Destroy();

    /// @returns the friendly name for the instruction
    virtual std::string FriendlyName() = 0;

    /// @param ctx the CloneContext used to clone this instruction
    /// @returns a clone of this instruction
    virtual Instruction* Clone(CloneContext& ctx) = 0;

    /// @returns true if the Instruction has not been destroyed with Destroy()
    bool Alive() const { return !flags_.Contains(Flag::kDead); }

    /// @returns true if the Instruction is sequenced. Sequenced instructions cannot be implicitly
    /// reordered with other sequenced instructions.
    bool Sequenced() const { return flags_.Contains(Flag::kSequenced); }

    /// Sets the block that owns this instruction
    /// @param block the new owner block
    void SetBlock(ir::Block* block) { block_ = block; }

    /// @returns the block that owns this instruction
    ir::Block* Block() { return block_; }

    /// Adds the new instruction before the given instruction in the owning block
    /// @param before the instruction to insert before
    void InsertBefore(Instruction* before);
    /// Adds the new instruction after the given instruction in the owning block
    /// @param after the instruction to insert after
    void InsertAfter(Instruction* after);
    /// Replaces this instruction with @p replacement in the owning block owning this instruction
    /// @param replacement the instruction to replace with
    void ReplaceWith(Instruction* replacement);
    /// Removes this instruction from the owning block
    void Remove();

    /// @param idx the index of the result
    /// @returns the result with index @p idx, or `nullptr` if there are no results or the index is
    /// out of bounds.
    Value* Result(size_t idx) {
        auto res = Results();
        return idx < res.Length() ? res[idx] : nullptr;
    }

    /// Pointer to the next instruction in the list
    Instruction* next = nullptr;
    /// Pointer to the previous instruction in the list
    Instruction* prev = nullptr;

  protected:
    /// Flags applied to an Instruction
    enum class Flag {
        /// The instruction has been destroyed
        kDead,
        /// The instruction must not be reordered with another sequenced instruction
        kSequenced,
    };

    /// Constructor
    Instruction();

    /// The block that owns this instruction
    ir::Block* block_ = nullptr;

    /// Bitset of instruction flags
    tint::EnumSet<Flag> flags_;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_INSTRUCTION_H_
