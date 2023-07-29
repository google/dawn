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

#include "src/tint/lang/wgsl/ast/switch_statement.h"

#include <utility>

#include "src/tint/lang/wgsl/ast/builder.h"
#include "src/tint/lang/wgsl/ast/clone_context.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::SwitchStatement);

namespace tint::ast {

SwitchStatement::SwitchStatement(GenerationID pid,
                                 NodeID nid,
                                 const Source& src,
                                 const Expression* cond,
                                 VectorRef<const CaseStatement*> b,
                                 VectorRef<const Attribute*> stmt_attrs,
                                 VectorRef<const Attribute*> body_attrs)
    : Base(pid, nid, src),
      condition(cond),
      body(std::move(b)),
      attributes(std::move(stmt_attrs)),
      body_attributes(std::move(body_attrs)) {
    TINT_ASSERT(condition);
    TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(condition, generation_id);
    for (auto* stmt : body) {
        TINT_ASSERT(stmt);
        TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(stmt, generation_id);
    }
    for (auto* attr : attributes) {
        TINT_ASSERT(attr);
        TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(attr, generation_id);
    }
    for (auto* attr : body_attributes) {
        TINT_ASSERT(attr);
        TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(attr, generation_id);
    }
}

SwitchStatement::~SwitchStatement() = default;

const SwitchStatement* SwitchStatement::Clone(CloneContext& ctx) const {
    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx.Clone(source);
    auto* cond = ctx.Clone(condition);
    auto b = ctx.Clone(body);
    auto attrs = ctx.Clone(attributes);
    auto body_attrs = ctx.Clone(body_attributes);
    return ctx.dst->create<SwitchStatement>(src, cond, std::move(b), std::move(attrs),
                                            std::move(body_attrs));
}

}  // namespace tint::ast
