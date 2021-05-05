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

#include "src/sem/call_target.h"

#include "src/symbol_table.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::CallTarget);

namespace tint {
namespace sem {

CallTarget::CallTarget(sem::Type* return_type, const ParameterList& parameters)
    : return_type_(return_type), parameters_(parameters) {
  TINT_ASSERT(return_type);
}

CallTarget::~CallTarget() = default;

int IndexOf(const ParameterList& parameters, Parameter::Usage usage) {
  for (size_t i = 0; i < parameters.size(); i++) {
    if (parameters[i].usage == usage) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

const char* str(Parameter::Usage usage) {
  switch (usage) {
    case Parameter::Usage::kArrayIndex:
      return "array_index";
    case Parameter::Usage::kBias:
      return "bias";
    case Parameter::Usage::kCoords:
      return "coords";
    case Parameter::Usage::kDepthRef:
      return "depth_ref";
    case Parameter::Usage::kDdx:
      return "ddx";
    case Parameter::Usage::kDdy:
      return "ddy";
    case Parameter::Usage::kLevel:
      return "level";
    case Parameter::Usage::kOffset:
      return "offset";
    case Parameter::Usage::kSampler:
      return "sampler";
    case Parameter::Usage::kSampleIndex:
      return "sample_index";
    case Parameter::Usage::kTexture:
      return "texture";
    case Parameter::Usage::kValue:
      return "value";
    default:
      return "<unknown>";
  }
}

std::ostream& operator<<(std::ostream& out, Parameter parameter) {
  out << "[type: " << parameter.type->FriendlyName(SymbolTable{ProgramID{}})
      << ", usage: " << str(parameter.usage) << "]";
  return out;
}

}  // namespace sem
}  // namespace tint
