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

#include "src/tint/ast/member_accessor_expression.h"
#include "src/tint/sem/member_accessor_expression.h"

#include <utility>

TINT_INSTANTIATE_TYPEINFO(tint::sem::MemberAccessorExpression);
TINT_INSTANTIATE_TYPEINFO(tint::sem::StructMemberAccess);
TINT_INSTANTIATE_TYPEINFO(tint::sem::Swizzle);

namespace tint::sem {

MemberAccessorExpression::MemberAccessorExpression(const ast::MemberAccessorExpression* declaration,
                                                   const sem::Type* type,
                                                   const Statement* statement,
                                                   bool has_side_effects,
                                                   const Variable* source_var /* = nullptr */)
    : Base(declaration, type, statement, Constant{}, has_side_effects, source_var) {}

MemberAccessorExpression::~MemberAccessorExpression() = default;

StructMemberAccess::StructMemberAccess(const ast::MemberAccessorExpression* declaration,
                                       const sem::Type* type,
                                       const Statement* statement,
                                       const StructMember* member,
                                       bool has_side_effects,
                                       const Variable* source_var /* = nullptr */)
    : Base(declaration, type, statement, has_side_effects, source_var), member_(member) {}

StructMemberAccess::~StructMemberAccess() = default;

Swizzle::Swizzle(const ast::MemberAccessorExpression* declaration,
                 const sem::Type* type,
                 const Statement* statement,
                 std::vector<uint32_t> indices,
                 bool has_side_effects,
                 const Variable* source_var /* = nullptr */)
    : Base(declaration, type, statement, has_side_effects, source_var),
      indices_(std::move(indices)) {}

Swizzle::~Swizzle() = default;

}  // namespace tint::sem
