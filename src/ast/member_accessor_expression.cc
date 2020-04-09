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

namespace tint {
namespace ast {

MemberAccessorExpression::MemberAccessorExpression() = default;

MemberAccessorExpression::MemberAccessorExpression(
    std::unique_ptr<Expression> structure,
    std::unique_ptr<IdentifierExpression> member)
    : Expression(), struct_(std::move(structure)), member_(std::move(member)) {}

MemberAccessorExpression::MemberAccessorExpression(
    const Source& source,
    std::unique_ptr<Expression> structure,
    std::unique_ptr<IdentifierExpression> member)
    : Expression(source),
      struct_(std::move(structure)),
      member_(std::move(member)) {}

MemberAccessorExpression::MemberAccessorExpression(MemberAccessorExpression&&) =
    default;

MemberAccessorExpression::~MemberAccessorExpression() = default;

bool MemberAccessorExpression::IsMemberAccessor() const {
  return true;
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
  out << "MemberAccessor{" << std::endl;
  struct_->to_str(out, indent + 2);
  member_->to_str(out, indent + 2);
  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
