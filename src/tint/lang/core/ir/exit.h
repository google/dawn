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

#ifndef SRC_TINT_LANG_CORE_IR_EXIT_H_
#define SRC_TINT_LANG_CORE_IR_EXIT_H_

#include "src/tint/lang/core/ir/terminator.h"

// Forward declarations
namespace tint::core::ir {
class ControlInstruction;
}  // namespace tint::core::ir

namespace tint::core::ir {

/// The base class for all exit terminators.
class Exit : public Castable<Exit, Terminator> {
  public:
    ~Exit() override;

    /// @copydoc Value::Destroy
    void Destroy() override;

    /// @return the control instruction that this exit is associated with
    ir::ControlInstruction* ControlInstruction() { return ctrl_inst_; }

  protected:
    /// Sets control instruction that this exit is associated with
    /// @param ctrl_inst the new ControlInstruction that this exit is associated with
    void SetControlInstruction(ir::ControlInstruction* ctrl_inst);

  private:
    ir::ControlInstruction* ctrl_inst_ = nullptr;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_EXIT_H_
