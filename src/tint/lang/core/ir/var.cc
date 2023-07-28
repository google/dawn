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

#include "src/tint/lang/core/ir/var.h"

#include "src/tint/lang/core/ir/store.h"
#include "src/tint/utils/ice/ice.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::Var);

namespace tint::ir {

Var::Var(InstructionResult* result) {
    if (result && result->Type()) {
        TINT_ASSERT(result->Type()->Is<type::Pointer>());
    }

    // Default to no initializer.
    AddOperand(Var::kInitializerOperandOffset, nullptr);
    AddResult(result);
}

Var::~Var() = default;

void Var::SetInitializer(Value* initializer) {
    SetOperand(Var::kInitializerOperandOffset, initializer);
}

void Var::DestroyIfOnlyAssigned() {
    auto* result = Result();
    if (result->Usages().All([](const Usage& u) { return u.instruction->Is<ir::Store>(); })) {
        while (!result->Usages().IsEmpty()) {
            auto& usage = *result->Usages().begin();
            usage.instruction->Destroy();
        }
        Destroy();
    }
}

}  // namespace tint::ir
