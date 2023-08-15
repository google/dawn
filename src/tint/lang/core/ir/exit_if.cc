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

#include "src/tint/lang/core/ir/exit_if.h"

#include <utility>

#include "src/tint/lang/core/ir/if.h"
#include "src/tint/lang/core/ir/multi_in_block.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::ir::ExitIf);

namespace tint::core::ir {

ExitIf::ExitIf(ir::If* i, VectorRef<Value*> args) {
    SetIf(i);
    AddOperands(ExitIf::kArgsOperandOffset, std::move(args));
}

ExitIf::~ExitIf() = default;

void ExitIf::SetIf(ir::If* i) {
    SetControlInstruction(i);
}

ir::If* ExitIf::If() {
    return static_cast<ir::If*>(ControlInstruction());
}

}  // namespace tint::core::ir
