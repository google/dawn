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

#include "src/tint/ast/switch_statement.h"

#include <utility>

#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::SwitchStatement);

namespace tint::ast {

SwitchStatement::SwitchStatement(ProgramID pid,
                                 NodeID nid,
                                 const Source& src,
                                 const Expression* cond,
                                 utils::VectorRef<const CaseStatement*> b)
    : Base(pid, nid, src), condition(cond), body(std::move(b)) {
    TINT_ASSERT(AST, condition);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, condition, program_id);
    for (auto* stmt : body) {
        TINT_ASSERT(AST, stmt);
        TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, stmt, program_id);
    }
}

SwitchStatement::SwitchStatement(SwitchStatement&&) = default;

SwitchStatement::~SwitchStatement() = default;

const SwitchStatement* SwitchStatement::Clone(CloneContext* ctx) const {
    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx->Clone(source);
    auto* cond = ctx->Clone(condition);
    auto b = ctx->Clone(body);
    return ctx->dst->create<SwitchStatement>(src, cond, std::move(b));
}

}  // namespace tint::ast
