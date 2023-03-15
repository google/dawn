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

#include "src/tint/ast/struct.h"

#include <string>

#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Struct);

namespace tint::ast {

Struct::Struct(ProgramID pid,
               NodeID nid,
               const Source& src,
               const Identifier* n,
               utils::VectorRef<const StructMember*> m,
               utils::VectorRef<const Attribute*> attrs)
    : Base(pid, nid, src, n), members(std::move(m)), attributes(std::move(attrs)) {
    for (auto* mem : members) {
        TINT_ASSERT(AST, mem);
        TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, mem, program_id);
    }
    for (auto* attr : attributes) {
        TINT_ASSERT(AST, attr);
        TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, attr, program_id);
    }
}

Struct::~Struct() = default;

const Struct* Struct::Clone(CloneContext* ctx) const {
    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx->Clone(source);
    auto n = ctx->Clone(name);
    auto mem = ctx->Clone(members);
    auto attrs = ctx->Clone(attributes);
    return ctx->dst->create<Struct>(src, n, std::move(mem), std::move(attrs));
}

}  // namespace tint::ast
