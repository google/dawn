// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_IR_BRANCH_H_
#define SRC_TINT_IR_BRANCH_H_

#include "src/tint/ir/instruction.h"
#include "src/tint/ir/value.h"
#include "src/tint/utils/castable.h"

// Forward declarations
namespace tint::ir {
class FlowNode;
}  // namespace tint::ir

namespace tint::ir {

/// A branch instruction. A branch is a walk terminating jump.
class Branch : public utils::Castable<Branch, Instruction> {
  public:
    /// Constructor
    /// @param to the block to branch too
    /// @param args the branch arguments
    explicit Branch(FlowNode* to, utils::VectorRef<Value*> args = {});
    ~Branch() override;

    /// @returns the block being branched too.
    const FlowNode* To() const { return to_; }

    /// @returns the branch arguments
    utils::VectorRef<Value*> Args() const { return args_; }

  private:
    FlowNode* to_;
    utils::Vector<Value*, 2> args_;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_BRANCH_H_
