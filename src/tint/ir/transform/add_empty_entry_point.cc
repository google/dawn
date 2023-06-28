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

#include "src/tint/ir/transform/add_empty_entry_point.h"

#include <utility>

#include "src/tint/ir/builder.h"
#include "src/tint/ir/module.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::transform::AddEmptyEntryPoint);

namespace tint::ir::transform {

AddEmptyEntryPoint::AddEmptyEntryPoint() = default;

AddEmptyEntryPoint::~AddEmptyEntryPoint() = default;

void AddEmptyEntryPoint::Run(ir::Module* ir, const DataMap&, DataMap&) const {
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

}  // namespace tint::ir::transform
