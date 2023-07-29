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

#include "src/tint/lang/wgsl/ast/transform/add_empty_entry_point.h"

#include <utility>

#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::AddEmptyEntryPoint);

using namespace tint::number_suffixes;  // NOLINT

namespace tint::ast::transform {
namespace {

bool ShouldRun(const Program* program) {
    for (auto* func : program->AST().Functions()) {
        if (func->IsEntryPoint()) {
            return false;
        }
    }
    return true;
}

}  // namespace

AddEmptyEntryPoint::AddEmptyEntryPoint() = default;

AddEmptyEntryPoint::~AddEmptyEntryPoint() = default;

Transform::ApplyResult AddEmptyEntryPoint::Apply(const Program* src,
                                                 const DataMap&,
                                                 DataMap&) const {
    if (!ShouldRun(src)) {
        return SkipTransform;
    }

    ProgramBuilder b;
    program::CloneContext ctx{&b, src, /* auto_clone_symbols */ true};

    b.Func(b.Symbols().New("unused_entry_point"), {}, b.ty.void_(), {},
           tint::Vector{
               b.Stage(PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    ctx.Clone();
    return resolver::Resolve(b);
}

}  // namespace tint::ast::transform
