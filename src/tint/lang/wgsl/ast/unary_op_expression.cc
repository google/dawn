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

#include "src/tint/lang/wgsl/ast/unary_op_expression.h"

#include "src/tint/lang/wgsl/program/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::UnaryOpExpression);

namespace tint::ast {

UnaryOpExpression::UnaryOpExpression(GenerationID pid,
                                     NodeID nid,
                                     const Source& src,
                                     UnaryOp o,
                                     const Expression* e)
    : Base(pid, nid, src), op(o), expr(e) {
    TINT_ASSERT(expr);
    TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(expr, generation_id);
}

UnaryOpExpression::~UnaryOpExpression() = default;

const UnaryOpExpression* UnaryOpExpression::Clone(CloneContext* ctx) const {
    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx->Clone(source);
    auto* e = ctx->Clone(expr);
    return ctx->dst->create<UnaryOpExpression>(src, op, e);
}

}  // namespace tint::ast
