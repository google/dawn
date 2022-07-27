// Copyright 2020 The Tint Authors.
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

#include "src/tint/ast/builtin_value.h"

namespace tint::ast {

std::ostream& operator<<(std::ostream& out, BuiltinValue builtin) {
    switch (builtin) {
        case BuiltinValue::kNone: {
            out << "none";
            break;
        }
        case BuiltinValue::kPosition: {
            out << "position";
            break;
        }
        case BuiltinValue::kVertexIndex: {
            out << "vertex_index";
            break;
        }
        case BuiltinValue::kInstanceIndex: {
            out << "instance_index";
            break;
        }
        case BuiltinValue::kFrontFacing: {
            out << "front_facing";
            break;
        }
        case BuiltinValue::kFragDepth: {
            out << "frag_depth";
            break;
        }
        case BuiltinValue::kLocalInvocationId: {
            out << "local_invocation_id";
            break;
        }
        case BuiltinValue::kLocalInvocationIndex: {
            out << "local_invocation_index";
            break;
        }
        case BuiltinValue::kGlobalInvocationId: {
            out << "global_invocation_id";
            break;
        }
        case BuiltinValue::kWorkgroupId: {
            out << "workgroup_id";
            break;
        }
        case BuiltinValue::kNumWorkgroups: {
            out << "num_workgroups";
            break;
        }
        case BuiltinValue::kSampleIndex: {
            out << "sample_index";
            break;
        }
        case BuiltinValue::kSampleMask: {
            out << "sample_mask";
            break;
        }
        case BuiltinValue::kPointSize: {
            out << "pointsize";
        }
    }
    return out;
}

}  // namespace tint::ast
