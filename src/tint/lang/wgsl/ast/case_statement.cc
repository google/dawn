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

#include "src/tint/lang/wgsl/ast/case_statement.h"

#include <utility>

#include "src/tint/lang/wgsl/ast/builder.h"
#include "src/tint/lang/wgsl/ast/clone_context.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::CaseStatement);

namespace tint::ast {

CaseStatement::CaseStatement(GenerationID pid,
                             NodeID nid,
                             const Source& src,
                             VectorRef<const CaseSelector*> s,
                             const BlockStatement* b)
    : Base(pid, nid, src), selectors(std::move(s)), body(b) {
    TINT_ASSERT(body);
    TINT_ASSERT(!selectors.IsEmpty());
    TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(body, generation_id);
    for (auto* selector : selectors) {
        TINT_ASSERT(selector);
        TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(selector, generation_id);
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

const CaseStatement* CaseStatement::Clone(CloneContext& ctx) const {
    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx.Clone(source);
    auto sel = ctx.Clone(selectors);
    auto* b = ctx.Clone(body);
    return ctx.dst->create<CaseStatement>(src, std::move(sel), b);
}

}  // namespace tint::ast
