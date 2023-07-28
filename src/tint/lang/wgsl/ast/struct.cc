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

#include "src/tint/lang/wgsl/ast/struct.h"

#include <string>

#include "src/tint/lang/wgsl/program/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Struct);

namespace tint::ast {

Struct::Struct(GenerationID pid,
               NodeID nid,
               const Source& src,
               const Identifier* n,
               VectorRef<const StructMember*> m,
               VectorRef<const Attribute*> attrs)
    : Base(pid, nid, src, n), members(std::move(m)), attributes(std::move(attrs)) {
    for (auto* mem : members) {
        TINT_ASSERT(mem);
        TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(mem, generation_id);
    }
    for (auto* attr : attributes) {
        TINT_ASSERT(attr);
        TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(attr, generation_id);
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
