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

#include "src/ast/struct.h"

namespace tint {
namespace ast {

Struct::Struct() : Node() {}

Struct::Struct(StructDecoration decoration, StructMemberList members)
    : Node(), decoration_(decoration), members_(std::move(members)) {}

Struct::Struct(const Source& source,
               StructDecoration decoration,
               StructMemberList members)
    : Node(source), decoration_(decoration), members_(std::move(members)) {}

Struct::Struct(Struct&&) = default;

Struct::~Struct() = default;

StructMember* Struct::get_member(const std::string& name) const {
  for (auto& mem : members_) {
    if (mem->name() == name) {
      return mem.get();
    }
  }
  return nullptr;
}

bool Struct::IsValid() const {
  for (const auto& mem : members_) {
    if (mem == nullptr || !mem->IsValid()) {
      return false;
    }
  }
  return true;
}

void Struct::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  if (decoration_ != StructDecoration::kNone) {
    out << "[[" << decoration_ << "]] ";
  }
  out << "Struct{" << std::endl;
  for (const auto& member : members_) {
    member->to_str(out, indent + 2);
  }
  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
