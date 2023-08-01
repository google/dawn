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

#include "src/tint/lang/core/ir/transform/add_empty_entry_point.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"

namespace tint::ir::transform {

namespace {

void Run(ir::Module* ir) {
    for (auto* func : ir->functions) {
        if (func->Stage() != Function::PipelineStage::kUndefined) {
            return;
        }
    }

    ir::Builder builder(*ir);
    auto* ep = builder.Function("unused_entry_point", ir->Types().void_(),
                                Function::PipelineStage::kCompute, std::array{1u, 1u, 1u});
    ep->Block()->Append(builder.Return(ep));
}

}  // namespace

Result<SuccessType, std::string> AddEmptyEntryPoint(Module* ir) {
    auto result = ValidateAndDumpIfNeeded(*ir, "AddEmptyEntryPoint transform");
    if (!result) {
        return result;
    }

    Run(ir);

    return Success;
}

}  // namespace tint::ir::transform
