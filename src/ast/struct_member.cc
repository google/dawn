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

#include "src/ast/struct_member.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::StructMember);

namespace tint {
namespace ast {

StructMember::StructMember(ProgramID program_id,
                           const Source& source,
                           const Symbol& sym,
                           ast::Type* type,
                           DecorationList decorations)
    : Base(program_id, source),
      symbol_(sym),
      type_(type),
      decorations_(std::move(decorations)) {
  TINT_ASSERT(type);
  TINT_ASSERT(symbol_.IsValid());
  TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(symbol_, program_id);
  for (auto* deco : decorations_) {
    TINT_ASSERT(deco);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(deco, program_id);
  }
}

StructMember::StructMember(StructMember&&) = default;

StructMember::~StructMember() = default;

bool StructMember::has_offset_decoration() const {
  return HasDecoration<StructMemberOffsetDecoration>(decorations_);
}

uint32_t StructMember::offset() const {
  if (auto* offset =
          GetDecoration<StructMemberOffsetDecoration>(decorations_)) {
    return offset->offset();
  }
  return 0;
}

StructMember* StructMember::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source());
  auto sym = ctx->Clone(symbol_);
  auto* ty = ctx->Clone(type_);
  auto decos = ctx->Clone(decorations_);
  return ctx->dst->create<StructMember>(src, sym, ty, decos);
}

void StructMember::to_str(const sem::Info& sem,
                          std::ostream& out,
                          size_t indent) const {
  make_indent(out, indent);
  out << "StructMember{";
  if (decorations_.size() > 0) {
    out << "[[ ";
    for (auto* deco : decorations_)
      out << deco->str(sem) << " ";
    out << "]] ";
  }

  out << symbol_.to_str() << ": " << type_->type_name() << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
