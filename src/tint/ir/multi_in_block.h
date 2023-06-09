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

#ifndef SRC_TINT_IR_MULTI_IN_BLOCK_H_
#define SRC_TINT_IR_MULTI_IN_BLOCK_H_

#include <utility>

#include "src/tint/ir/block.h"

// Forward declarations
namespace tint::ir {
class BlockParam;
}

namespace tint::ir {

/// A block that can be the target of multiple branches.
/// MultiInBlocks maintain a list of inbound branches from branch instructions excluding ir::If,
/// ir::Switch and ir::Loop which implicitly branch to the internal block.
/// MultiInBlocks hold a number of BlockParam parameters, used to pass values from the branch source
/// to this target.
class MultiInBlock : public utils::Castable<MultiInBlock, Block> {
  public:
    /// Constructor
    MultiInBlock();
    ~MultiInBlock() override;

    /// Sets the params to the block
    /// @param params the params for the block
    void SetParams(utils::VectorRef<BlockParam*> params);

    /// Sets the params to the block
    /// @param params the params for the block
    void SetParams(std::initializer_list<BlockParam*> params);

    /// @returns the params to the block
    const utils::Vector<BlockParam*, 2>& Params() { return params_; }

    /// @returns branches made to this block by sibling blocks
    const utils::VectorRef<ir::Branch*> InboundSiblingBranches() {
        return inbound_sibling_branches_;
    }

    /// Adds the given branch to the list of branches made to this block by sibling blocks
    /// @param branch the branch to add
    void AddInboundSiblingBranch(ir::Branch* branch);

  private:
    utils::Vector<BlockParam*, 2> params_;
    utils::Vector<ir::Branch*, 2> inbound_sibling_branches_;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_MULTI_IN_BLOCK_H_
