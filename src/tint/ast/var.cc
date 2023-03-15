// Copyright 2022 The Tint Authors.
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

#include "src/tint/ast/var.h"

#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Var);

namespace tint::ast {

Var::Var(ProgramID pid,
         NodeID nid,
         const Source& src,
         const Identifier* n,
         Type ty,
         const Expression* address_space,
         const Expression* access,
         const Expression* init,
         utils::VectorRef<const Attribute*> attrs)
    : Base(pid, nid, src, n, ty, init, std::move(attrs)),
      declared_address_space(address_space),
      declared_access(access) {}

Var::~Var() = default;

const char* Var::Kind() const {
    return "var";
}

const Var* Var::Clone(CloneContext* ctx) const {
    auto src = ctx->Clone(source);
    auto* n = ctx->Clone(name);
    auto ty = ctx->Clone(type);
    auto* address_space = ctx->Clone(declared_address_space);
    auto* access = ctx->Clone(declared_access);
    auto* init = ctx->Clone(initializer);
    auto attrs = ctx->Clone(attributes);
    return ctx->dst->create<Var>(src, n, ty, address_space, access, init, std::move(attrs));
}

}  // namespace tint::ast
