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

#include "src/ast/clone_context.h"
#include "src/ast/module.h"
#include "src/ast/struct_member_offset_decoration.h"

TINT_INSTANTIATE_CLASS_ID(tint::ast::StructMember);

namespace tint {
namespace ast {

StructMember::StructMember(const Source& source,
                           const Symbol& sym,
                           type::Type* type,
                           StructMemberDecorationList decorations)
    : Base(source),
      symbol_(sym),
      type_(type),
      decorations_(std::move(decorations)) {}

StructMember::StructMember(StructMember&&) = default;

StructMember::~StructMember() = default;

bool StructMember::has_offset_decoration() const {
  for (auto* deco : decorations_) {
    if (deco->Is<StructMemberOffsetDecoration>()) {
      return true;
    }
  }
  return false;
}

uint32_t StructMember::offset() const {
  for (auto* deco : decorations_) {
    if (auto* offset = deco->As<StructMemberOffsetDecoration>()) {
      return offset->offset();
    }
  }
  return 0;
}

StructMember* StructMember::Clone(CloneContext* ctx) const {
  return ctx->mod->create<StructMember>(ctx->Clone(source()),
                                        ctx->Clone(symbol_), ctx->Clone(type_),
                                        ctx->Clone(decorations_));
}

bool StructMember::IsValid() const {
  if (type_ == nullptr || !symbol_.IsValid()) {
    return false;
  }
  for (auto* deco : decorations_) {
    if (deco == nullptr) {
      return false;
    }
  }
  return true;
}

void StructMember::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "StructMember{";
  if (decorations_.size() > 0) {
    out << "[[ ";
    for (auto* deco : decorations_)
      out << deco->str() << " ";
    out << "]] ";
  }

  out << symbol_.to_str() << ": " << type_->type_name() << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
