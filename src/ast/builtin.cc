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

#include "src/ast/builtin.h"

namespace tint {
namespace ast {

std::ostream& operator<<(std::ostream& out, Builtin builtin) {
  switch (builtin) {
    case Builtin::kNone: {
      out << "none";
      break;
    }
    case Builtin::kPosition: {
      out << "position";
      break;
    }
    case Builtin::kVertexIndex: {
      out << "vertex_index";
      break;
    }
    case Builtin::kInstanceIndex: {
      out << "instance_index";
      break;
    }
    case Builtin::kFrontFacing: {
      out << "front_facing";
      break;
    }
    case Builtin::kFragCoord: {
      out << "frag_coord";
      break;
    }
    case Builtin::kFragDepth: {
      out << "frag_depth";
      break;
    }
    case Builtin::kLocalInvocationId: {
      out << "local_invocation_id";
      break;
    }
    case Builtin::kLocalInvocationIndex: {
      out << "local_invocation_index";
      break;
    }
    case Builtin::kGlobalInvocationId: {
      out << "global_invocation_id";
      break;
    }
    case Builtin::kWorkgroupId: {
      out << "workgroup_id";
      break;
    }
    case Builtin::kSampleIndex: {
      out << "sample_index";
      break;
    }
    case Builtin::kSampleMask: {
      out << "sample_mask";
      break;
    }
    case Builtin::kSampleMaskIn: {
      out << "sample_mask_in";
      break;
    }
    case Builtin::kSampleMaskOut: {
      out << "sample_mask_out";
      break;
    }
    case Builtin::kPointSize: {
      out << "pointsize";
    }
  }
  return out;
}

}  // namespace ast
}  // namespace tint
