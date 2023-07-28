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

#include "src/tint/lang/wgsl/ast/block_statement.h"

#include "src/tint/lang/wgsl/program/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::BlockStatement);

namespace tint::ast {

BlockStatement::BlockStatement(GenerationID pid,
                               NodeID nid,
                               const Source& src,
                               VectorRef<const Statement*> stmts,
                               VectorRef<const Attribute*> attrs)
    : Base(pid, nid, src), statements(std::move(stmts)), attributes(attrs) {
    for (auto* stmt : statements) {
        TINT_ASSERT(stmt);
        TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(stmt, generation_id);
    }
    for (auto* attr : attributes) {
        TINT_ASSERT(attr);
        TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(attr, generation_id);
    }
}

BlockStatement::~BlockStatement() = default;

const BlockStatement* BlockStatement::Clone(CloneContext* ctx) const {
    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx->Clone(source);
    auto stmts = ctx->Clone(statements);
    auto attrs = ctx->Clone(attributes);
    return ctx->dst->create<BlockStatement>(src, std::move(stmts), std::move(attrs));
}

}  // namespace tint::ast
