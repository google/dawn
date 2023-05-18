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

#ifndef SRC_TINT_IR_SWITCH_H_
#define SRC_TINT_IR_SWITCH_H_

#include "src/tint/ir/block.h"
#include "src/tint/ir/branch.h"
#include "src/tint/ir/constant.h"
#include "src/tint/ir/flow_node.h"
#include "src/tint/ir/value.h"

namespace tint::ir {

/// Flow node representing a switch statement
class Switch : public utils::Castable<Switch, FlowNode> {
  public:
    /// A case selector
    struct CaseSelector {
        /// @returns true if this is a default selector
        bool IsDefault() const { return val == nullptr; }

        /// The selector value, or nullptr if this is the default selector
        Constant* val = nullptr;
    };

    /// A case label in the struct
    struct Case {
        /// The case selector for this node
        utils::Vector<CaseSelector, 4> selectors;
        /// The start block for the case block.
        Branch start = {};

        /// @returns the case start target
        const Branch& Start() const { return start; }
        /// @returns the case start target
        Branch& Start() { return start; }
    };

    /// Constructor
    /// @param cond the condition
    explicit Switch(Value* cond);
    Switch(const Switch&) = delete;
    Switch(Switch&&) = delete;
    ~Switch() override;

    Switch& operator=(const Switch&) = delete;
    Switch& operator=(Switch&&) = delete;

    /// @returns the switch merge branch
    const Branch& Merge() const { return merge_; }
    /// @returns the switch merge branch
    Branch& Merge() { return merge_; }

    /// @returns the switch cases
    utils::VectorRef<Case> Cases() const { return cases_; }
    /// @returns the switch cases
    utils::Vector<Case, 4>& Cases() { return cases_; }

    /// @returns the condition
    const Value* Condition() const { return condition_; }

  private:
    Branch merge_ = {};
    utils::Vector<Case, 4> cases_;
    Value* condition_;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_SWITCH_H_
