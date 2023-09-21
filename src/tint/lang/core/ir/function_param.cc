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

#include "src/tint/lang/core/ir/function_param.h"

#include "src/tint/lang/core/ir/clone_context.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/utils/ice/ice.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::ir::FunctionParam);

namespace tint::core::ir {

FunctionParam::FunctionParam(const core::type::Type* ty) : type_(ty) {
    TINT_ASSERT(ty != nullptr);
}

FunctionParam::~FunctionParam() = default;

std::string_view ToString(enum FunctionParam::Builtin value) {
    switch (value) {
        case FunctionParam::Builtin::kVertexIndex:
            return "vertex_index";
        case FunctionParam::Builtin::kInstanceIndex:
            return "instance_index";
        case FunctionParam::Builtin::kPosition:
            return "position";
        case FunctionParam::Builtin::kFrontFacing:
            return "front_facing";
        case FunctionParam::Builtin::kLocalInvocationId:
            return "local_invocation_id";
        case FunctionParam::Builtin::kLocalInvocationIndex:
            return "local_invocation_index";
        case FunctionParam::Builtin::kGlobalInvocationId:
            return "global_invocation_id";
        case FunctionParam::Builtin::kWorkgroupId:
            return "workgroup_id";
        case FunctionParam::Builtin::kNumWorkgroups:
            return "num_workgroups";
        case FunctionParam::Builtin::kSampleIndex:
            return "sample_index";
        case FunctionParam::Builtin::kSampleMask:
            return "sample_mask";
        case FunctionParam::Builtin::kSubgroupInvocationId:
            return "subgroup_invocation_id";
        case FunctionParam::Builtin::kSubgroupSize:
            return "subgroup_size";
    }
    return "<unknown>";
}

FunctionParam* FunctionParam::Clone(CloneContext& ctx) {
    auto* out = ctx.ir.values.Create<FunctionParam>(type_);
    out->builtin_ = builtin_;
    out->location_ = location_;
    out->binding_point_ = binding_point_;
    out->invariant_ = invariant_;

    auto name = ctx.ir.NameOf(this);
    if (name.IsValid()) {
        ctx.ir.SetName(out, name);
    }
    return out;
}

}  // namespace tint::core::ir
