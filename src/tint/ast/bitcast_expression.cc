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

#include "src/tint/ast/bitcast_expression.h"

#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::BitcastExpression);

namespace tint::ast {

BitcastExpression::BitcastExpression(ProgramID pid,
                                     NodeID nid,
                                     const Source& src,
                                     Type t,
                                     const Expression* e)
    : Base(pid, nid, src), type(t), expr(e) {
    TINT_ASSERT(AST, type);
    TINT_ASSERT(AST, expr);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, expr, program_id);
}

BitcastExpression::~BitcastExpression() = default;

const BitcastExpression* BitcastExpression::Clone(CloneContext* ctx) const {
    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx->Clone(source);
    auto t = ctx->Clone(type);
    auto* e = ctx->Clone(expr);
    return ctx->dst->create<BitcastExpression>(src, t, e);
}

}  // namespace tint::ast
