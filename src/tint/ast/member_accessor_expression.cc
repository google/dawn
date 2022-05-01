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

#include "src/tint/ast/member_accessor_expression.h"

#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::MemberAccessorExpression);

namespace tint::ast {

MemberAccessorExpression::MemberAccessorExpression(ProgramID pid,
                                                   const Source& src,
                                                   const Expression* str,
                                                   const IdentifierExpression* mem)
    : Base(pid, src), structure(str), member(mem) {
    TINT_ASSERT(AST, structure);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, structure, program_id);
    TINT_ASSERT(AST, member);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, member, program_id);
}

MemberAccessorExpression::MemberAccessorExpression(MemberAccessorExpression&&) = default;

MemberAccessorExpression::~MemberAccessorExpression() = default;

const MemberAccessorExpression* MemberAccessorExpression::Clone(CloneContext* ctx) const {
    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx->Clone(source);
    auto* str = ctx->Clone(structure);
    auto* mem = ctx->Clone(member);
    return ctx->dst->create<MemberAccessorExpression>(src, str, mem);
}

}  // namespace tint::ast
