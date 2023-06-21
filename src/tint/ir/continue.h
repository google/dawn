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

#ifndef SRC_TINT_IR_CONTINUE_H_
#define SRC_TINT_IR_CONTINUE_H_

#include "src/tint/ir/terminator.h"
#include "src/tint/utils/castable.h"

// Forward declarations
namespace tint::ir {
class Loop;
}  // namespace tint::ir

namespace tint::ir {

/// A continue instruction.
class Continue : public utils::Castable<Continue, Terminator> {
  public:
    /// The base offset in Operands() for the args
    static constexpr size_t kArgsOperandOffset = 0;

    /// Constructor
    /// @param loop the loop owning the continue block
    /// @param args the arguments for the MultiInBlock
    explicit Continue(ir::Loop* loop, utils::VectorRef<Value*> args = utils::Empty);
    ~Continue() override;

    /// @returns the loop owning the continue block
    ir::Loop* Loop() { return loop_; }

  private:
    ir::Loop* loop_ = nullptr;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_CONTINUE_H_
