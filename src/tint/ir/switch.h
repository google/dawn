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
#include "src/tint/ir/flow_node.h"
#include "src/tint/ir/register.h"

// Forward declarations
namespace tint::ast {
class CaseSelector;
class SwitchStatement;
}  // namespace tint::ast

namespace tint::ir {

/// Flow node representing a switch statement
class Switch : public Castable<Switch, FlowNode> {
  public:
    /// A case label in the struct
    struct Case {
        /// The case selector for this node
        const utils::VectorRef<const ast::CaseSelector*> selectors;
        /// The start block for the case block.
        Block* start_target;
    };

    /// Constructor
    /// @param stmt the originating ast switch statement
    explicit Switch(const ast::SwitchStatement* stmt);
    ~Switch() override;

    /// The originating switch statment in the AST
    const ast::SwitchStatement* source;

    /// The switch merge target
    Block* merge_target;

    /// The switch case statements
    utils::Vector<Case, 4> cases;

    /// Register holding the condition result
    Register condition;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_SWITCH_H_
