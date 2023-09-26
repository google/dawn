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

#include "src/tint/lang/core/ir/load.h"

#include "src/tint/lang/core/ir/clone_context.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/utils/ice/ice.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::ir::Load);

namespace tint::core::ir {

Load::Load(InstructionResult* result, Value* from) {
    flags_.Add(Flag::kSequenced);

    TINT_ASSERT(from->Type()->Is<core::type::Pointer>());
    TINT_ASSERT(from && from->Type()->UnwrapPtr() == result->Type());

    AddOperand(Load::kFromOperandOffset, from);
    AddResult(result);
}

Load::~Load() = default;

Load* Load::Clone(CloneContext& ctx) {
    auto* new_result = ctx.Clone(Result());
    auto* from = ctx.Remap(From());
    return ctx.ir.instructions.Create<Load>(new_result, from);
}

}  // namespace tint::core::ir
