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

#include "src/ast/builtin_decoration.h"

namespace tint {
namespace ast {

constexpr const DecorationKind BuiltinDecoration::Kind;

BuiltinDecoration::BuiltinDecoration(Builtin builtin, const Source& source)
    : VariableDecoration(source), builtin_(builtin) {}

BuiltinDecoration::~BuiltinDecoration() = default;

DecorationKind BuiltinDecoration::GetKind() const {
  return Kind;
}

bool BuiltinDecoration::IsKind(DecorationKind kind) const {
  return kind == Kind || VariableDecoration::IsKind(kind);
}

bool BuiltinDecoration::IsBuiltin() const {
  return true;
}

void BuiltinDecoration::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "BuiltinDecoration{" << builtin_ << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
