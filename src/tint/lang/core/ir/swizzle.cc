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

#include "src/tint/lang/core/ir/swizzle.h"

#include <utility>

#include "src/tint/lang/core/ir/clone_context.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/utils/ice/ice.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::ir::Swizzle);

namespace tint::core::ir {

Swizzle::Swizzle(InstructionResult* result, Value* object, VectorRef<uint32_t> indices)
    : indices_(std::move(indices)) {
    TINT_ASSERT(!indices.IsEmpty());
    TINT_ASSERT(indices.Length() <= 4);

    AddOperand(Swizzle::kObjectOperandOffset, object);
    AddResult(result);

    for (auto idx : indices_) {
        TINT_ASSERT(idx < 4);
    }
}

Swizzle::~Swizzle() = default;

Swizzle* Swizzle::Clone(CloneContext& ctx) {
    auto* result = ctx.Clone(Result());
    auto* new_obj = ctx.Clone(Object());
    return ctx.ir.instructions.Create<Swizzle>(result, new_obj, indices_);
}

}  // namespace tint::core::ir
