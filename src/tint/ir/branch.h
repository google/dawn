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

#include "src/tint/ir/operand_instruction.h"
#include "src/tint/ir/value.h"
#include "src/tint/utils/castable.h"

// Forward declarations
namespace tint::ir {
class Block;
}  // namespace tint::ir

namespace tint::ir {

/// A branch instruction.
class Branch : public utils::Castable<Branch, OperandInstruction<1>> {
  public:
    ~Branch() override;

    /// @returns the branch arguments
    virtual utils::Slice<Value const* const> Args() const { return operands_.Slice(); }
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_BRANCH_H_
