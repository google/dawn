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

#include "src/tint/ast/if_statement.h"
#include "src/tint/ir/flow_node.h"
#include "src/tint/ir/register.h"

// Forward declarations
namespace tint::ir {
class Block;
}  // namespace tint::ir

namespace tint::ir {

/// A flow node representing an if statement.
class If : public Castable<If, FlowNode> {
  public:
    /// Constructor
    /// @param stmt the ast::IfStatement or ast::BreakIfStatement
    explicit If(const ast::Statement* stmt);
    ~If() override;

    /// The ast::IfStatement or ast::BreakIfStatement source for this flow node.
    const ast::Statement* source;

    /// The true branch block
    Block* true_target = nullptr;
    /// The false branch block
    Block* false_target = nullptr;
    /// An block to reconvert the true/false barnches. The block always exists, but there maybe no
    /// branches into it. (e.g. if both branches `return`)
    Block* merge_target = nullptr;
    /// Register holding the condition result
    Register condition;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_IF_H_
