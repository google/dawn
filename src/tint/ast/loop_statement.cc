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

#include "src/tint/ast/loop_statement.h"

#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::LoopStatement);

namespace tint::ast {

LoopStatement::LoopStatement(ProgramID pid,
                             NodeID nid,
                             const Source& src,
                             const BlockStatement* b,
                             const BlockStatement* cont)
    : Base(pid, nid, src), body(b), continuing(cont) {
    TINT_ASSERT(AST, body);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, body, program_id);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, continuing, program_id);
}

LoopStatement::LoopStatement(LoopStatement&&) = default;

LoopStatement::~LoopStatement() = default;

const LoopStatement* LoopStatement::Clone(CloneContext* ctx) const {
    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx->Clone(source);
    auto* b = ctx->Clone(body);
    auto* cont = ctx->Clone(continuing);
    return ctx->dst->create<LoopStatement>(src, b, cont);
}

}  // namespace tint::ast
