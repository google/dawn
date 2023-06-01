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

#ifndef SRC_TINT_IR_INSTRUCTION_H_
#define SRC_TINT_IR_INSTRUCTION_H_

#include "src/tint/ir/value.h"
#include "src/tint/utils/castable.h"

// Forward declarations
namespace tint::ir {
class Block;
}  // namespace tint::ir

namespace tint::ir {

/// An instruction in the IR.
class Instruction : public utils::Castable<Instruction, Value> {
  public:
    /// Destructor
    ~Instruction() override;

    /// Sets the block that owns this instruction
    /// @param block the new owner block
    void SetBlock(ir::Block* block) { block_ = block; }

    /// @returns the block that owns this instruction
    ir::Block* Block() { return block_; }
    /// @returns the block that owns this instruction
    const ir::Block* Block() const { return block_; }

    /// Adds the new instruction before the given instruction in the owning block
    /// @param before the instruction to insert before
    void InsertBefore(Instruction* before);
    /// Adds the new instruction after the given instruction in the owning block
    /// @param after the instruction to insert after
    void InsertAfter(Instruction* after);
    /// Replaces this instruction with @p replacement in the owning block owning this instruction
    /// @param replacement the instruction to replace with
    void Replace(Instruction* replacement);
    /// Removes this instruction from the owning block
    void Remove();

    /// Pointer to the next instruction in the list
    Instruction* next = nullptr;
    /// Pointer to the previous instruction in the list
    Instruction* prev = nullptr;

  protected:
    /// Constructor
    Instruction();

    /// The block that owns this instruction
    ir::Block* block_ = nullptr;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_INSTRUCTION_H_
