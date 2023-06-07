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

#ifndef SRC_TINT_IR_BLOCK_H_
#define SRC_TINT_IR_BLOCK_H_

#include <utility>

#include "src/tint/ir/block_param.h"
#include "src/tint/ir/branch.h"
#include "src/tint/ir/instruction.h"
#include "src/tint/utils/vector.h"

// Forward declarations
namespace tint::ir {
class ControlInstruction;
}  // namespace tint::ir

namespace tint::ir {

/// A block of statements. The instructions in the block are a linear list of instructions to
/// execute. The block will branch at the end. The only blocks which do not branch are the end
/// blocks of functions.
class Block : public utils::Castable<Block> {
  public:
    /// Constructor
    Block();
    ~Block() override;

    /// @returns true if this is block has a branch target set
    bool HasBranchTarget() const {
        return instructions_.last != nullptr && instructions_.last->Is<ir::Branch>();
    }

    /// @return the node this block branches to or nullptr if the block doesn't branch
    const ir::Branch* Branch() const {
        if (!HasBranchTarget()) {
            return nullptr;
        }
        return instructions_.last->As<ir::Branch>();
    }

    /// Sets the instructions in the block
    /// @param instructions the instructions to set
    void SetInstructions(utils::VectorRef<Instruction*> instructions);

    /// @returns the instructions in the block
    Instruction* Instructions() const { return instructions_.first; }

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
    Iterator begin() const { return Iterator{instructions_.first}; }

    /// @returns the ending iterator
    Iterator end() const { return Iterator{nullptr}; }

    /// @returns the first instruction in the instruction list
    Instruction* Front() const { return instructions_.first; }

    /// @returns the last instruction in the instruction list
    Instruction* Back() const { return instructions_.last; }

    /// Adds the instruction to the beginning of the block
    /// @param inst the instruction to add
    void Prepend(Instruction* inst);
    /// Adds the instruction to the end of the block
    /// @param inst the instruction to add
    void Append(Instruction* inst);
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
    bool IsEmpty() const { return Length() == 0; }

    /// @returns the number of instructions in the block
    size_t Length() const { return instructions_.count; }

    /// Sets the params to the block
    /// @param params the params for the block
    void SetParams(utils::VectorRef<const BlockParam*> params);
    /// @return the parameters passed into the block
    utils::VectorRef<const BlockParam*> Params() const { return params_; }
    /// @returns the params to the block
    utils::Vector<const BlockParam*, 0>& Params() { return params_; }

    /// @returns the inbound branch list for the block
    utils::VectorRef<ir::Branch*> InboundBranches() const { return inbound_branches_; }

    /// Adds the given node to the inbound branches
    /// @param node the node to add
    void AddInboundBranch(ir::Branch* node);

    /// @return the parent instruction that owns this block
    ControlInstruction* Parent() const { return parent_; }

    /// @param parent the parent instruction that owns this block
    void SetParent(ControlInstruction* parent) { parent_ = parent; }

  private:
    struct {
        Instruction* first = nullptr;
        Instruction* last = nullptr;
        size_t count = 0;
    } instructions_;

    utils::Vector<const BlockParam*, 0> params_;

    /// The list of branches into this node. This list maybe empty for several
    /// reasons:
    ///   - Node is a start node
    ///   - Node is a merge target outside control flow (e.g. an if that returns in both branches)
    ///   - Node is a continue target outside control flow (e.g. a loop that returns)
    utils::Vector<ir::Branch*, 2> inbound_branches_;

    ControlInstruction* parent_ = nullptr;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_BLOCK_H_
