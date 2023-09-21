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

#ifndef SRC_TINT_LANG_CORE_IR_BLOCK_H_
#define SRC_TINT_LANG_CORE_IR_BLOCK_H_

#include <utility>

#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/terminator.h"
#include "src/tint/utils/containers/vector.h"

// Forward declarations
namespace tint::core::ir {
class ControlInstruction;
}  // namespace tint::core::ir

namespace tint::core::ir {

/// A block of statements. The instructions in the block are a linear list of instructions to
/// execute. The block will terminate with a Terminator instruction at the end.
class Block : public Castable<Block> {
  public:
    /// Constructor
    Block();
    ~Block() override;

    /// @param ctx the CloneContext used to clone this block
    /// @returns a clone of this block
    virtual Block* Clone(CloneContext& ctx);

    /// Clones the block contents into the given block
    /// @param ctx the CloneContext used to clone
    /// @param out the block to clone into
    virtual void CloneInto(CloneContext& ctx, Block* out);

    /// @returns true if this is block has a terminator instruction
    bool HasTerminator() {
        return instructions_.last != nullptr && instructions_.last->Is<ir::Terminator>();
    }

    /// @return the terminator instruction for this block
    ir::Terminator* Terminator() {
        if (!HasTerminator()) {
            return nullptr;
        }
        return instructions_.last->As<ir::Terminator>();
    }

    /// @returns the instructions in the block
    Instruction* Instructions() { return instructions_.first; }

    /// Iterator for the instructions inside a block
    class Iterator {
      public:
        /// Constructor
        /// @param inst the instruction to start iterating from
        explicit Iterator(Instruction* inst) : inst_(inst) {}
        ~Iterator() = default;

        /// Dereference operator
        /// @returns the instruction for this iterator
        Instruction* operator*() const { return inst_; }

        /// Comparison operator
        /// @param itr to compare against
        /// @returns true if this iterator and @p itr point to the same instruction
        bool operator==(const Iterator& itr) const { return itr.inst_ == inst_; }

        /// Not equal operator
        /// @param itr to compare against
        /// @returns true if this iterator and @p itr point to different instructions
        bool operator!=(const Iterator& itr) const { return !(*this == itr); }

        /// Increment operator
        /// @returns this iterator advanced to the next element
        Iterator& operator++() {
            inst_ = inst_->next;
            return *this;
        }

      private:
        Instruction* inst_ = nullptr;
    };

    /// @returns the iterator pointing to the start of the instruction list
    Iterator begin() { return Iterator{instructions_.first}; }

    /// @returns the ending iterator
    Iterator end() { return Iterator{nullptr}; }

    /// @returns the first instruction in the instruction list
    Instruction* Front() { return instructions_.first; }

    /// @returns the last instruction in the instruction list
    Instruction* Back() { return instructions_.last; }

    /// Adds the instruction to the beginning of the block
    /// @param inst the instruction to add
    /// @returns the instruction to allow calls to be chained
    Instruction* Prepend(Instruction* inst);
    /// Adds the instruction to the end of the block
    /// @param inst the instruction to add
    /// @returns the instruction to allow calls to be chained
    Instruction* Append(Instruction* inst);
    /// Adds the new instruction before the given instruction
    /// @param before the instruction to insert before
    /// @param inst the instruction to insert
    void InsertBefore(Instruction* before, Instruction* inst);
    /// Adds the new instruction after the given instruction
    /// @param after the instruction to insert after
    /// @param inst the instruction to insert
    void InsertAfter(Instruction* after, Instruction* inst);
    /// Replaces the target instruction with the new instruction
    /// @param target the instruction to replace
    /// @param inst the instruction to insert
    void Replace(Instruction* target, Instruction* inst);
    /// Removes the target instruction
    /// @param inst the instruction to remove
    void Remove(Instruction* inst);

    /// @returns true if the block contains no instructions
    bool IsEmpty() { return Length() == 0; }

    /// @returns the number of instructions in the block
    size_t Length() { return instructions_.count; }

    /// @return the parent instruction that owns this block
    ControlInstruction* Parent() { return parent_; }

    /// @param parent the parent instruction that owns this block
    void SetParent(ControlInstruction* parent) { parent_ = parent; }

  private:
    struct {
        Instruction* first = nullptr;
        Instruction* last = nullptr;
        size_t count = 0;
    } instructions_;

    ControlInstruction* parent_ = nullptr;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_BLOCK_H_
