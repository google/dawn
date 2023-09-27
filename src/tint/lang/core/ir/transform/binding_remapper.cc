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

#include "src/tint/lang/core/ir/transform/binding_remapper.h"

#include <utility>

#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/utils/result/result.h"
#include "src/tint/utils/text/string.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {

namespace {

Result<SuccessType> Run(ir::Module& ir,
                        const std::unordered_map<BindingPoint, BindingPoint>& binding_points) {
    if (binding_points.empty()) {
        return Success;
    }
    if (ir.root_block->IsEmpty()) {
        return Success;
    }

    // Find binding resources.
    for (auto inst : *ir.root_block) {
        auto* var = inst->As<Var>();
        if (!var || !var->Alive()) {
            continue;
        }

        auto bp = var->BindingPoint();
        if (!bp) {
            continue;
        }

        // Replace group and binding index if requested.
        auto to = binding_points.find(bp.value());
        if (to != binding_points.end()) {
            var->SetBindingPoint(to->second.group, to->second.binding);
        }
    }

    return Success;
}

}  // namespace

Result<SuccessType> BindingRemapper(
    Module& ir,
    const std::unordered_map<BindingPoint, BindingPoint>& binding_points) {
    auto result = ValidateAndDumpIfNeeded(ir, "BindingRemapper transform");
    if (!result) {
        return result;
    }

    return Run(ir, binding_points);
}

}  // namespace tint::core::ir::transform
