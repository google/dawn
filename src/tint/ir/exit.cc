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

#include "src/tint/ir/exit.h"

#include "src/tint/ir/control_instruction.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::Exit);

namespace tint::ir {

Exit::~Exit() = default;

void Exit::Destroy() {
    SetControlInstruction(nullptr);
    Base::Destroy();
}

void Exit::SetControlInstruction(ir::ControlInstruction* ctrl_inst) {
    if (ctrl_inst_ == ctrl_inst) {
        return;
    }
    if (ctrl_inst_) {
        ctrl_inst_->RemoveExit(this);
    }
    ctrl_inst_ = ctrl_inst;
    if (ctrl_inst_) {
        ctrl_inst_->AddExit(this);
    }
}

}  // namespace tint::ir
