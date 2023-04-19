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

#include "src/tint/ir/block.h"
#include "src/tint/ir/branch.h"
#include "src/tint/ir/flow_node.h"

namespace tint::ir {

/// Flow node describing a loop.
class Loop : public utils::Castable<Loop, FlowNode> {
  public:
    /// Constructor
    Loop();
    ~Loop() override;

    /// The start block is the first block in a loop.
    Branch start = {};
    /// The continue target of the block.
    Branch continuing = {};
    /// The loop merge target. If the `loop` does a `return` then this block may not actually
    /// end up in the control flow. We need it if the loop does a `break` we know where to break
    /// too.
    Branch merge = {};
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_LOOP_H_
