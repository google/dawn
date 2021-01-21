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

#include "src/ast/member_accessor_expression.h"

#include "src/ast/module.h"
#include "src/clone_context.h"

TINT_INSTANTIATE_CLASS_ID(tint::ast::MemberAccessorExpression);

namespace tint {
namespace ast {

MemberAccessorExpression::MemberAccessorExpression(const Source& source,
                                                   Expression* structure,
                                                   IdentifierExpression* member)
    : Base(source), struct_(structure), member_(member) {}

MemberAccessorExpression::MemberAccessorExpression(MemberAccessorExpression&&) =
    default;

MemberAccessorExpression::~MemberAccessorExpression() = default;

MemberAccessorExpression* MemberAccessorExpression::Clone(
    CloneContext* ctx) const {
  return ctx->mod->create<MemberAccessorExpression>(
      ctx->Clone(source()), ctx->Clone(struct_), ctx->Clone(member_));
}

bool MemberAccessorExpression::IsValid() const {
  if (struct_ == nullptr || !struct_->IsValid()) {
    return false;
  }
  if (member_ == nullptr || !member_->IsValid()) {
    return false;
  }
  return true;
}

void MemberAccessorExpression::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "MemberAccessor[" << result_type_str() << "]{" << std::endl;
  struct_->to_str(out, indent + 2);
  member_->to_str(out, indent + 2);
  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
