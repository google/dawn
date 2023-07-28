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

#include "src/tint/lang/wgsl/ast/for_loop_statement.h"

#include <utility>

#include "src/tint/lang/wgsl/program/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::ForLoopStatement);

namespace tint::ast {

ForLoopStatement::ForLoopStatement(GenerationID pid,
                                   NodeID nid,
                                   const Source& src,
                                   const Statement* init,
                                   const Expression* cond,
                                   const Statement* cont,
                                   const BlockStatement* b,
                                   VectorRef<const ast::Attribute*> attrs)
    : Base(pid, nid, src),
      initializer(init),
      condition(cond),
      continuing(cont),
      body(b),
      attributes(std::move(attrs)) {
    TINT_ASSERT(body);

    TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(initializer, generation_id);
    TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(condition, generation_id);
    TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(continuing, generation_id);
    TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(body, generation_id);
    for (auto* attr : attributes) {
        TINT_ASSERT(attr);
        TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(attr, generation_id);
    }
}

ForLoopStatement::~ForLoopStatement() = default;

const ForLoopStatement* ForLoopStatement::Clone(CloneContext* ctx) const {
    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx->Clone(source);

    auto* init = ctx->Clone(initializer);
    auto* cond = ctx->Clone(condition);
    auto* cont = ctx->Clone(continuing);
    auto* b = ctx->Clone(body);
    auto attrs = ctx->Clone(attributes);
    return ctx->dst->create<ForLoopStatement>(src, init, cond, cont, b, std::move(attrs));
}

}  // namespace tint::ast
