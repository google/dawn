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

#include "src/tint/ir/function.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::Function);

namespace tint::ir {

Function::Function() : Base() {}

Function::~Function() = default;

utils::StringStream& operator<<(utils::StringStream& out, Function::PipelineStage value) {
    switch (value) {
        case Function::PipelineStage::kVertex:
            return out << "vertex";
        case Function::PipelineStage::kFragment:
            return out << "fragment";
        case Function::PipelineStage::kCompute:
            return out << "compute";
        default:
            break;
    }
    return out << "<unknown>";
}

utils::StringStream& operator<<(utils::StringStream& out, Function::ReturnAttribute value) {
    switch (value) {
        case Function::ReturnAttribute::kLocation:
            return out << "location";
        case Function::ReturnAttribute::kFragDepth:
            return out << "frag_depth";
        case Function::ReturnAttribute::kSampleMask:
            return out << "sample_mask";
        case Function::ReturnAttribute::kPosition:
            return out << "position";
        case Function::ReturnAttribute::kInvariant:
            return out << "invariant";
        default:
            break;
    }
    return out << "<unknown>";
}

}  // namespace tint::ir
