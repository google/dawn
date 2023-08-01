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

using namespace tint::number_suffixes;  // NOLINT

namespace tint::ir::transform {

namespace {

Result<SuccessType, std::string> Run(ir::Module* ir, const BindingRemapperOptions& options) {
    if (!options.access_controls.empty()) {
        return std::string("remapping access controls is currently unsupported");
    }
    if (options.binding_points.empty()) {
        return Success;
    }
    if (!ir->root_block) {
        return Success;
    }

    // Find binding resources.
    for (auto inst : *ir->root_block) {
        auto* var = inst->As<Var>();
        if (!var || !var->Alive()) {
            continue;
        }

        auto bp = var->BindingPoint();
        if (!bp) {
            continue;
        }

        // Replace group and binding index if requested.
        tint::BindingPoint from{bp->group, bp->binding};
        auto to = options.binding_points.find(from);
        if (to != options.binding_points.end()) {
            var->SetBindingPoint(to->second.group, to->second.binding);
        }
    }

    return Success;
}

}  // namespace

Result<SuccessType, std::string> BindingRemapper(Module* ir,
                                                 const BindingRemapperOptions& options) {
    auto result = ValidateAndDumpIfNeeded(*ir, "BindingRemapper transform");
    if (!result) {
        return result;
    }

    return Run(ir, options);
}

}  // namespace tint::ir::transform
