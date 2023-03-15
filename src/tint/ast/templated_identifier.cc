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

#include "src/tint/ast/templated_identifier.h"

#include <utility>

#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::TemplatedIdentifier);

namespace tint::ast {

TemplatedIdentifier::TemplatedIdentifier(ProgramID pid,
                                         NodeID nid,
                                         const Source& src,
                                         const Symbol& sym,
                                         utils::VectorRef<const Expression*> args,
                                         utils::VectorRef<const Attribute*> attrs)
    : Base(pid, nid, src, sym), arguments(std::move(args)), attributes(std::move(attrs)) {
    TINT_ASSERT(AST, !arguments.IsEmpty());  // Should have been an Identifier if this fires.
    for (auto* arg : arguments) {
        TINT_ASSERT_PROGRAM_IDS_EQUAL(AST, arg, program_id);
    }
    for (auto* attr : attributes) {
        TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, attr, program_id);
    }
}

TemplatedIdentifier::~TemplatedIdentifier() = default;

const TemplatedIdentifier* TemplatedIdentifier::Clone(CloneContext* ctx) const {
    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx->Clone(source);
    auto sym = ctx->Clone(symbol);
    auto args = ctx->Clone(arguments);
    auto attrs = ctx->Clone(attributes);
    return ctx->dst->create<TemplatedIdentifier>(src, sym, std::move(args), std::move(attrs));
}

}  // namespace tint::ast
