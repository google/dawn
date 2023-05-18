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
#include "src/tint/ir/flow_node.h"
#include "src/tint/ir/instruction.h"
#include "src/tint/utils/vector.h"

namespace tint::ir {

/// A flow node comprising a block of statements. The instructions in the block are a linear list of
/// instructions to execute. The block will branch at the end. The only blocks which do not branch
/// are the end blocks of functions.
class Block : public utils::Castable<Block, FlowNode> {
  public:
    /// Constructor
    Block();
    Block(const Block&) = delete;
    Block(Block&&) = delete;
    ~Block() override;

    Block& operator=(const Block&) = delete;
    Block& operator=(Block&&) = delete;

    /// Sets the blocks branch target to the given node.
    /// @param to the node to branch too
    /// @param args the branch arguments
    void BranchTo(FlowNode* to, utils::VectorRef<Value*> args = {});

    /// @returns true if this is block has a branch target set
    bool HasBranchTarget() const override { return branch_.target != nullptr; }

    /// @return the node this block branches too.
    const ir::Branch& Branch() const { return branch_; }

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
    /// @returns the params to the block
    utils::Vector<const BlockParam*, 0>& Params() { return params_; }

    /// @return the parameters passed into the block
    utils::VectorRef<const BlockParam*> Params() const { return params_; }

  private:
    ir::Branch branch_ = {};
    utils::Vector<const Instruction*, 16> instructions_;
    utils::Vector<const BlockParam*, 0> params_;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_BLOCK_H_
