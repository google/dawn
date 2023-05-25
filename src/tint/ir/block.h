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
        return !instructions_.IsEmpty() && instructions_.Back()->Is<ir::Branch>();
    }

    /// @return the node this block branches to or nullptr if the block doesn't branch
    const ir::Branch* Branch() const {
        if (!HasBranchTarget()) {
            return nullptr;
        }
        return instructions_.Back()->As<ir::Branch>();
    }

    /// @param target the block to see if we trampoline too
    /// @returns if this block just branches to the provided target.
    bool IsTrampoline(const Block* target) const {
        if (instructions_.Length() != 1) {
            return false;
        }
        if (auto* inst = instructions_.Front()->As<ir::Branch>()) {
            return inst->To() == target;
        }
        return false;
    }

    /// Sets the instructions in the block
    /// @param instructions the instructions to set
    void SetInstructions(utils::VectorRef<const Instruction*> instructions) {
        instructions_ = std::move(instructions);
    }

    /// @returns the instructions in the block
    utils::VectorRef<const Instruction*> Instructions() const { return instructions_; }
    /// @returns the instructions in the block
    utils::Vector<const Instruction*, 16>& Instructions() { return instructions_; }

    /// Sets the params to the block
    /// @param params the params for the block
    void SetParams(utils::VectorRef<const BlockParam*> params) { params_ = std::move(params); }
    /// @return the parameters passed into the block
    utils::VectorRef<const BlockParam*> Params() const { return params_; }
    /// @returns the params to the block
    utils::Vector<const BlockParam*, 0>& Params() { return params_; }

    /// @returns the inbound branch list for the block
    utils::VectorRef<ir::Branch*> InboundBranches() const { return inbound_branches_; }

    /// Adds the given node to the inbound branches
    /// @param node the node to add
    void AddInboundBranch(ir::Branch* node) { inbound_branches_.Push(node); }

  private:
    utils::Vector<const Instruction*, 16> instructions_;
    utils::Vector<const BlockParam*, 0> params_;

    /// The list of branches into this node. This list maybe empty for several
    /// reasons:
    ///   - Node is a start node
    ///   - Node is a merge target outside control flow (e.g. an if that returns in both branches)
    ///   - Node is a continue target outside control flow (e.g. a loop that returns)
    utils::Vector<ir::Branch*, 2> inbound_branches_;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_BLOCK_H_
