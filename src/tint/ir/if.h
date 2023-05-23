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

#include "src/tint/ir/block.h"
#include "src/tint/ir/branch.h"
#include "src/tint/ir/value.h"

// Forward declarations
namespace tint::ir {
class Block;
}  // namespace tint::ir

namespace tint::ir {

/// An if instruction
class If : public utils::Castable<If, Branch> {
  public:
    /// Constructor
    /// @param cond the if condition
    /// @param t the true block
    /// @param f the false block
    /// @param m the merge block
    explicit If(Value* cond, Block* t, Block* f, Block* m);
    ~If() override;

    /// @returns the if condition
    const Value* Condition() const { return condition_; }
    /// @returns the if condition
    Value* Condition() { return condition_; }

    /// @returns the true branch block
    const Block* True() const { return true_; }
    /// @returns the true branch block
    Block* True() { return true_; }

    /// @returns the false branch block
    const Block* False() const { return false_; }
    /// @returns the false branch block
    Block* False() { return false_; }

    /// @returns the merge branch block
    const Block* Merge() const { return merge_; }
    /// @returns the merge branch block
    Block* Merge() { return merge_; }

  private:
    Value* condition_ = nullptr;
    Block* true_ = nullptr;
    Block* false_ = nullptr;
    Block* merge_ = nullptr;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_IF_H_
