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

#include "src/ast/stride_decoration.h"

namespace tint {
namespace ast {

constexpr const DecorationKind StrideDecoration::Kind;

StrideDecoration::StrideDecoration(uint32_t stride, const Source& source)
    : Base(source), stride_(stride) {}

DecorationKind StrideDecoration::GetKind() const {
  return Kind;
}

bool StrideDecoration::IsKind(DecorationKind kind) const {
  return kind == Kind || Base::IsKind(kind);
}

bool StrideDecoration::IsStride() const {
  return true;
}

StrideDecoration::~StrideDecoration() = default;

void StrideDecoration::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "stride " << stride_;
}

}  // namespace ast
}  // namespace tint
