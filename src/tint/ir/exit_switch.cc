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

#include "src/tint/ir/exit_switch.h"

#include <utility>

#include "src/tint/ir/multi_in_block.h"
#include "src/tint/ir/switch.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::ExitSwitch);

namespace tint::ir {

ExitSwitch::ExitSwitch(ir::Switch* sw, utils::VectorRef<Value*> args /* = utils::Empty */) {
    SetSwitch(sw);
    AddOperands(ExitSwitch::kArgsOperandOffset, std::move(args));
}

ExitSwitch::~ExitSwitch() = default;

void ExitSwitch::SetSwitch(ir::Switch* s) {
    SetControlInstruction(s);
}

ir::Switch* ExitSwitch::Switch() {
    return static_cast<ir::Switch*>(ControlInstruction());
}

}  // namespace tint::ir
