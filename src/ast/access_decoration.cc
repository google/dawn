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

#include "src/ast/access_decoration.h"

namespace tint {
namespace ast {

constexpr const DecorationKind AccessDecoration::Kind;

AccessDecoration::AccessDecoration(AccessControl val, const Source& source)
    : TypeDecoration(source), value_(val) {}

AccessDecoration::~AccessDecoration() = default;

DecorationKind AccessDecoration::GetKind() const {
  return Kind;
}

bool AccessDecoration::IsKind(DecorationKind kind) const {
  return kind == Kind || TypeDecoration::IsKind(kind);
}

bool AccessDecoration::IsAccess() const {
  return true;
}

void AccessDecoration::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "AccessDecoration{" << value_ << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
