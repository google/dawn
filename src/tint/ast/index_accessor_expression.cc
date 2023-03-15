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

#include "src/tint/ast/index_accessor_expression.h"

#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::IndexAccessorExpression);

namespace tint::ast {

IndexAccessorExpression::IndexAccessorExpression(ProgramID pid,
                                                 NodeID nid,
                                                 const Source& src,
                                                 const Expression* obj,
                                                 const Expression* idx)
    : Base(pid, nid, src, obj), index(idx) {
    TINT_ASSERT(AST, idx);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, idx, program_id);
}

IndexAccessorExpression::~IndexAccessorExpression() = default;

const IndexAccessorExpression* IndexAccessorExpression::Clone(CloneContext* ctx) const {
    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx->Clone(source);
    auto* obj = ctx->Clone(object);
    auto* idx = ctx->Clone(index);
    return ctx->dst->create<IndexAccessorExpression>(src, obj, idx);
}

}  // namespace tint::ast
