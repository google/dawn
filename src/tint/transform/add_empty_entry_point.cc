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

using namespace tint::number_suffixes;  // NOLINT

namespace tint::transform {
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
    CloneContext ctx{&b, src, /* auto_clone_symbols */ true};

    b.Func(b.Symbols().New("unused_entry_point"), {}, b.ty.void_(), {},
           utils::Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    ctx.Clone();
    return Program(std::move(b));
}

}  // namespace tint::transform
