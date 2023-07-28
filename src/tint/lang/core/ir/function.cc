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

#include "src/tint/utils/containers/predicates.h"
#include "src/tint/utils/ice/ice.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::Function);

namespace tint::ir {

Function::Function(const type::Type* rt,
                   PipelineStage stage,
                   std::optional<std::array<uint32_t, 3>> wg_size)
    : pipeline_stage_(stage), workgroup_size_(wg_size) {
    TINT_ASSERT(rt != nullptr);

    return_.type = rt;
}

Function::~Function() = default;

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

}  // namespace tint::ir
