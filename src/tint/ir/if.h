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
    /// @param cond the if condition
    explicit If(Value* cond);
    If(const If&) = delete;
    If(If&&) = delete;
    ~If() override;

    If& operator=(const If&) = delete;
    If& operator=(If&&) = delete;

    /// @returns the if condition
    const Value* Condition() const { return condition_; }

    /// @returns the true branch block
    const Branch& True() const { return true_; }
    /// @returns the true branch block
    Branch& True() { return true_; }

    /// @returns the false branch block
    const Branch& False() const { return false_; }
    /// @returns the false branch block
    Branch& False() { return false_; }

    /// @returns the merge branch block
    const Branch& Merge() const { return merge_; }
    /// @returns the merge branch block
    Branch& Merge() { return merge_; }

  private:
    Branch true_ = {};
    Branch false_ = {};
    Branch merge_ = {};
    Value* condition_;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_IF_H_
