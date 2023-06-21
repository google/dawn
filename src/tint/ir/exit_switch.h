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

#ifndef SRC_TINT_IR_EXIT_SWITCH_H_
#define SRC_TINT_IR_EXIT_SWITCH_H_

#include "src/tint/ir/exit.h"
#include "src/tint/utils/castable.h"

// Forward declarations
namespace tint::ir {
class Switch;
}  // namespace tint::ir

namespace tint::ir {

/// A exit switch instruction.
class ExitSwitch : public utils::Castable<ExitSwitch, Exit> {
  public:
    /// The base offset in Operands() for the args
    static constexpr size_t kArgsOperandOffset = 0;

    /// Constructor
    /// @param sw the switch being exited
    /// @param args the target MultiInBlock arguments
    explicit ExitSwitch(ir::Switch* sw, utils::VectorRef<Value*> args = utils::Empty);
    ~ExitSwitch() override;

    /// Re-associates the exit with the given switch instruction
    /// @param s the new switch to exit from
    void SetSwitch(ir::Switch* s);

    /// @returns the switch being exited
    ir::Switch* Switch();
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_EXIT_SWITCH_H_
