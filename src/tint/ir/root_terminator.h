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

#ifndef SRC_TINT_IR_ROOT_TERMINATOR_H_
#define SRC_TINT_IR_ROOT_TERMINATOR_H_

#include "src/tint/ir/flow_node.h"

namespace tint::ir {

/// Flow node used as the end of a function. Must only be used as the `end_target` in a function
/// flow node. There are no instructions and no branches from this node.
class RootTerminator : public utils::Castable<RootTerminator, FlowNode> {
  public:
    /// Constructor
    RootTerminator();
    ~RootTerminator() override;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_ROOT_TERMINATOR_H_
