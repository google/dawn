// Copyright 2023 The Tint Authors.
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

#include "src/tint/ast/diagnostic_rule_name.h"

#include <string>

#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::DiagnosticRuleName);

namespace tint::ast {

DiagnosticRuleName::DiagnosticRuleName(ProgramID pid,
                                       NodeID nid,
                                       const Source& src,
                                       const Identifier* n)
    : Base(pid, nid, src), name(n) {
    TINT_ASSERT(AST, name != nullptr);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, name, program_id);
    if (name) {
        // It is invalid for a diagnostic rule name to be templated
        TINT_ASSERT(AST, !name->Is<TemplatedIdentifier>());
    }
}

DiagnosticRuleName::DiagnosticRuleName(ProgramID pid,
                                       NodeID nid,
                                       const Source& src,
                                       const Identifier* c,
                                       const Identifier* n)
    : Base(pid, nid, src), category(c), name(n) {
    TINT_ASSERT(AST, name != nullptr);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, name, program_id);
    if (name) {
        // It is invalid for a diagnostic rule name to be templated
        TINT_ASSERT(AST, !name->Is<TemplatedIdentifier>());
    }
    if (category) {
        TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, category, program_id);
        // It is invalid for a diagnostic rule category to be templated
        TINT_ASSERT(AST, !category->Is<TemplatedIdentifier>());
    }
}

const DiagnosticRuleName* DiagnosticRuleName::Clone(CloneContext* ctx) const {
    auto src = ctx->Clone(source);
    auto n = ctx->Clone(name);
    if (auto c = ctx->Clone(category)) {
        return ctx->dst->create<DiagnosticRuleName>(src, c, n);
    }
    return ctx->dst->create<DiagnosticRuleName>(src, n);
}

std::string DiagnosticRuleName::String() const {
    if (category) {
        return category->symbol.Name() + "." + name->symbol.Name();
    } else {
        return name->symbol.Name();
    }
}

}  // namespace tint::ast
