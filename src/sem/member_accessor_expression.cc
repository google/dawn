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

#include "src/ast/member_accessor_expression.h"
#include "src/sem/member_accessor_expression.h"

#include <utility>

TINT_INSTANTIATE_TYPEINFO(tint::sem::MemberAccessorExpression);
TINT_INSTANTIATE_TYPEINFO(tint::sem::StructMemberAccess);
TINT_INSTANTIATE_TYPEINFO(tint::sem::Swizzle);

namespace tint {
namespace sem {

MemberAccessorExpression::MemberAccessorExpression(
    ast::MemberAccessorExpression* declaration,
    const sem::Type* type,
    Statement* statement)
    : Base(declaration, type, statement) {}

MemberAccessorExpression::~MemberAccessorExpression() = default;

StructMemberAccess::StructMemberAccess(
    ast::MemberAccessorExpression* declaration,
    const sem::Type* type,
    Statement* statement,
    const StructMember* member)
    : Base(declaration, type, statement), member_(member) {}

StructMemberAccess::~StructMemberAccess() = default;

Swizzle::Swizzle(ast::MemberAccessorExpression* declaration,
                 const sem::Type* type,
                 Statement* statement,
                 std::vector<uint32_t> indices)
    : Base(declaration, type, statement), indices_(std::move(indices)) {}

Swizzle::~Swizzle() = default;

}  // namespace sem
}  // namespace tint
