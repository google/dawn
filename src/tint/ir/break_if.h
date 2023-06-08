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

#ifndef SRC_TINT_IR_BREAK_IF_H_
#define SRC_TINT_IR_BREAK_IF_H_

#include "src/tint/ir/branch.h"
#include "src/tint/ir/value.h"
#include "src/tint/utils/castable.h"

// Forward declarations
namespace tint::ir {
class Loop;
}  // namespace tint::ir

namespace tint::ir {

/// A break-if iteration instruction.
class BreakIf : public utils::Castable<BreakIf, Branch> {
  public:
    /// Constructor
    /// @param condition the break condition
    /// @param loop the loop containing the break-if
    /// @param args the branch arguments
    BreakIf(Value* condition, ir::Loop* loop, utils::VectorRef<Value*> args = utils::Empty);
    ~BreakIf() override;

    /// @returns the branch arguments
    utils::Slice<Value* const> Args() override { return operands_.Slice().Offset(1); }

    /// @returns the break condition
    Value* Condition() { return operands_[0]; }

    /// @returns the loop containing the break-if
    ir::Loop* Loop() { return loop_; }

  private:
    ir::Loop* loop_ = nullptr;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_BREAK_IF_H_
