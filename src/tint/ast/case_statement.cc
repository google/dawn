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

#include "src/tint/ast/case_statement.h"

#include <utility>

#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::CaseStatement);

namespace tint::ast {

CaseStatement::CaseStatement(ProgramID pid,
                             NodeID nid,
                             const Source& src,
                             utils::VectorRef<const CaseSelector*> s,
                             const BlockStatement* b)
    : Base(pid, nid, src), selectors(std::move(s)), body(b) {
    TINT_ASSERT(AST, body);
    TINT_ASSERT(AST, !selectors.IsEmpty());
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, body, program_id);
    for (auto* selector : selectors) {
        TINT_ASSERT(AST, selector);
        TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, selector, program_id);
    }
}

CaseStatement::~CaseStatement() = default;

bool CaseStatement::ContainsDefault() const {
    for (const auto* sel : selectors) {
        if (sel->IsDefault()) {
            return true;
        }
    }
    return false;
}

const CaseStatement* CaseStatement::Clone(CloneContext* ctx) const {
    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx->Clone(source);
    auto sel = ctx->Clone(selectors);
    auto* b = ctx->Clone(body);
    return ctx->dst->create<CaseStatement>(src, std::move(sel), b);
}

}  // namespace tint::ast
