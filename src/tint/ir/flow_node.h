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

#ifndef SRC_TINT_IR_FLOW_NODE_H_
#define SRC_TINT_IR_FLOW_NODE_H_

#include "src/tint/utils/castable.h"
#include "src/tint/utils/vector.h"

namespace tint::ir {

/// Base class for flow nodes
class FlowNode : public utils::Castable<FlowNode> {
  public:
    ~FlowNode() override;

    /// The list of flow nodes which branch into this node. This list maybe empty for several
    /// reasons:
    ///   - Node is a start node
    ///   - Node is a merge target outside control flow (if that returns in both branches)
    ///   - Node is a continue target outside control flow (loop that returns)
    utils::Vector<FlowNode*, 2> inbound_branches;

    /// @returns true if this node has inbound branches and branches out
    bool IsConnected() const { return !IsDead() && !inbound_branches.IsEmpty(); }

    /// @returns true if the node does not branch out
    virtual bool IsDead() const { return false; }

  protected:
    /// Constructor
    FlowNode();
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_FLOW_NODE_H_
