// Copyright 2022 The Tint Authors.
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

#include "src/tint/lang/core/ir/function.h"

#include "src/tint/lang/core/ir/clone_context.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/utils/containers/predicates.h"
#include "src/tint/utils/ice/ice.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::ir::Function);

namespace tint::core::ir {

Function::Function(const core::type::Type* rt,
                   PipelineStage stage,
                   std::optional<std::array<uint32_t, 3>> wg_size)
    : pipeline_stage_(stage), workgroup_size_(wg_size) {
    TINT_ASSERT(rt != nullptr);

    return_.type = rt;
}

Function::~Function() = default;

Function* Function::Clone(CloneContext& ctx) {
    auto* new_func = ctx.ir.values.Create<Function>(return_.type, pipeline_stage_, workgroup_size_);
    new_func->block_ = ctx.ir.blocks.Create<ir::Block>();
    new_func->params_ = ctx.Clone<1>(params_.Slice());
    new_func->return_.builtin = return_.builtin;
    new_func->return_.location = return_.location;
    new_func->return_.invariant = return_.invariant;

    ctx.Replace(this, new_func);
    block_->CloneInto(ctx, new_func->block_);

    ctx.ir.SetName(new_func, ctx.ir.NameOf(this).Name());
    ctx.ir.functions.Push(new_func);
    return new_func;
}

void Function::SetParams(VectorRef<FunctionParam*> params) {
    params_ = std::move(params);
    TINT_ASSERT(!params_.Any(IsNull));
}

void Function::SetParams(std::initializer_list<FunctionParam*> params) {
    params_ = params;
    TINT_ASSERT(!params_.Any(IsNull));
}

std::string_view ToString(Function::PipelineStage value) {
    switch (value) {
        case Function::PipelineStage::kVertex:
            return "vertex";
        case Function::PipelineStage::kFragment:
            return "fragment";
        case Function::PipelineStage::kCompute:
            return "compute";
        default:
            break;
    }
    return "<unknown>";
}

std::string_view ToString(enum Function::ReturnBuiltin value) {
    switch (value) {
        case Function::ReturnBuiltin::kFragDepth:
            return "frag_depth";
        case Function::ReturnBuiltin::kSampleMask:
            return "sample_mask";
        case Function::ReturnBuiltin::kPosition:
            return "position";
    }
    return "<unknown>";
}

}  // namespace tint::core::ir
