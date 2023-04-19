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

#ifndef SRC_TINT_IR_IF_H_
#define SRC_TINT_IR_IF_H_

#include "src/tint/ir/branch.h"
#include "src/tint/ir/flow_node.h"
#include "src/tint/ir/value.h"

// Forward declarations
namespace tint::ir {
class Block;
}  // namespace tint::ir

namespace tint::ir {

/// A flow node representing an if statement.
class If : public utils::Castable<If, FlowNode> {
  public:
    /// Constructor
    If();
    ~If() override;

    /// The true branch block
    Branch true_ = {};
    /// The false branch block
    Branch false_ = {};
    /// An block to converge the true/false branches. The block always exists, but there maybe no
    /// branches into it. (e.g. if both branches `return`)
    Branch merge = {};
    /// Value holding the condition result
    const Value* condition = nullptr;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_IF_H_
