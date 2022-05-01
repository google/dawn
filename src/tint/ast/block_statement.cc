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

#include "src/tint/ast/block_statement.h"

#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::BlockStatement);

namespace tint::ast {

BlockStatement::BlockStatement(ProgramID pid, const Source& src, const StatementList& stmts)
    : Base(pid, src), statements(std::move(stmts)) {
    for (auto* stmt : statements) {
        TINT_ASSERT(AST, stmt);
        TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, stmt, program_id);
    }
}

BlockStatement::BlockStatement(BlockStatement&&) = default;

BlockStatement::~BlockStatement() = default;

const BlockStatement* BlockStatement::Clone(CloneContext* ctx) const {
    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx->Clone(source);
    auto stmts = ctx->Clone(statements);
    return ctx->dst->create<BlockStatement>(src, stmts);
}

}  // namespace tint::ast
