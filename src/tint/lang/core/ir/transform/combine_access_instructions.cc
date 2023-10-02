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

#include "src/tint/lang/core/ir/transform/combine_access_instructions.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {

namespace {

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    Module& ir;

    /// Process the module.
    void Process() {
        // Loop over every instruction looking for access instructions.
        for (auto* inst : ir.instructions.Objects()) {
            if (auto* access = inst->As<ir::Access>(); access && access->Alive()) {
                // Look for places where the result of this access instruction is used as a base
                // pointer for another access instruction.
                access->Result()->ForEachUse([&](Usage use) {
                    auto* child = use.instruction->As<ir::Access>();
                    if (child && use.operand_index == ir::Access::kObjectOperandOffset) {
                        // Push the indices of the parent access instruction into the child.
                        Vector<ir::Value*, 4> operands;
                        operands.Push(access->Object());
                        for (auto* idx : access->Indices()) {
                            operands.Push(idx);
                        }
                        for (auto* idx : child->Indices()) {
                            operands.Push(idx);
                        }
                        child->SetOperands(std::move(operands));
                    }
                });

                // If there are no other uses of the access instruction, remove it.
                if (access->Result()->Usages().IsEmpty()) {
                    access->Destroy();
                }
            }
        }
    }
};

}  // namespace

Result<SuccessType> CombineAccessInstructions(Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "CombineAccessInstructions transform");
    if (!result) {
        return result;
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
