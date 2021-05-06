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

#include <string>

#include "src/ast/struct_block_decoration.h"
#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Struct);

namespace tint {
namespace ast {

Struct::Struct(ProgramID program_id,
               const Source& source,
               Symbol name,
               StructMemberList members,
               DecorationList decorations)
    : Base(program_id, source, name),
      members_(std::move(members)),
      decorations_(std::move(decorations)) {
  for (auto* mem : members_) {
    TINT_ASSERT(mem);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(mem, program_id);
  }
  for (auto* deco : decorations_) {
    TINT_ASSERT(deco);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(deco, program_id);
  }
}

Struct::Struct(Struct&&) = default;

Struct::~Struct() = default;

StructMember* Struct::get_member(const Symbol& symbol) const {
  for (auto* mem : members_) {
    if (mem->symbol() == symbol) {
      return mem;
    }
  }
  return nullptr;
}

bool Struct::IsBlockDecorated() const {
  return HasDecoration<StructBlockDecoration>(decorations_);
}

Struct* Struct::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source());
  auto n = ctx->Clone(name());
  auto mem = ctx->Clone(members());
  auto decos = ctx->Clone(decorations());
  return ctx->dst->create<Struct>(src, n, mem, decos);
}

void Struct::to_str(const sem::Info& sem,
                    std::ostream& out,
                    size_t indent) const {
  out << "Struct " << name().to_str() << " {" << std::endl;
  for (auto* deco : decorations_) {
    make_indent(out, indent + 2);
    out << "[[";
    deco->to_str(sem, out, 0);
    out << "]]" << std::endl;
  }
  for (auto* member : members_) {
    member->to_str(sem, out, indent + 2);
  }
  make_indent(out, indent);
  out << "}" << std::endl;
}

std::string Struct::type_name() const {
  return "__struct_" + name().to_str();
}

}  // namespace ast
}  // namespace tint
