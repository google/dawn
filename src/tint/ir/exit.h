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

#ifndef SRC_TINT_IR_EXIT_H_
#define SRC_TINT_IR_EXIT_H_

#include "src/tint/ir/branch.h"

// Forward declarations
namespace tint::ir {
class ControlInstruction;
}  // namespace tint::ir

namespace tint::ir {

/// The base class for all exit terminators.
class Exit : public utils::Castable<Exit, Branch> {
  public:
    ~Exit() override;

    /// @copydoc Value::Destroy
    void Destroy() override;

  protected:
    /// @return the control instruction that this exit is associated with
    ir::ControlInstruction* ControlInstruction() { return ctrl_inst_; }

    /// Sets control instruction that this exit is associated with
    /// @param ctrl_inst the new ControlInstruction that this exit is associated with
    void SetControlInstruction(ir::ControlInstruction* ctrl_inst);

  private:
    ir::ControlInstruction* ctrl_inst_ = nullptr;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_EXIT_H_
