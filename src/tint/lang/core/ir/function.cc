// Copyright 2022 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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

void Function::Destroy() {
    Base::Destroy();
    block_->Destroy();
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
