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
    ~Block() override;

    /// @returns true if this is a dead block. This can happen in the case like a loop merge block
    /// which is never reached.
    bool IsDead() const override { return branch.target == nullptr; }

    /// The node this block branches too.
    Branch branch = {};

    /// The instructions in the block
    utils::Vector<const Instruction*, 16> instructions;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_BLOCK_H_
