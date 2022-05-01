// Copyright 2021 The Tint Authors.
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

#include "src/tint/transform/add_empty_entry_point.h"

#include <utility>

#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::AddEmptyEntryPoint);

namespace tint::transform {

AddEmptyEntryPoint::AddEmptyEntryPoint() = default;

AddEmptyEntryPoint::~AddEmptyEntryPoint() = default;

bool AddEmptyEntryPoint::ShouldRun(const Program* program, const DataMap&) const {
    for (auto* func : program->AST().Functions()) {
        if (func->IsEntryPoint()) {
            return false;
        }
    }
    return true;
}

void AddEmptyEntryPoint::Run(CloneContext& ctx, const DataMap&, DataMap&) const {
    ctx.dst->Func(ctx.dst->Symbols().New("unused_entry_point"), {}, ctx.dst->ty.void_(), {},
                  {ctx.dst->Stage(ast::PipelineStage::kCompute), ctx.dst->WorkgroupSize(1)});
    ctx.Clone();
}

}  // namespace tint::transform
