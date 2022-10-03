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
         const Symbol& sym,
         const ast::Type* ty,
         AddressSpace address_space,
         Access access,
         const Expression* ctor,
         utils::VectorRef<const Attribute*> attrs)
    : Base(pid, nid, src, sym, ty, ctor, std::move(attrs)),
      declared_address_space(address_space),
      declared_access(access) {}

Var::Var(Var&&) = default;

Var::~Var() = default;

const char* Var::Kind() const {
    return "var";
}

const Var* Var::Clone(CloneContext* ctx) const {
    auto src = ctx->Clone(source);
    auto sym = ctx->Clone(symbol);
    auto* ty = ctx->Clone(type);
    auto* ctor = ctx->Clone(constructor);
    auto attrs = ctx->Clone(attributes);
    return ctx->dst->create<Var>(src, sym, ty, declared_address_space, declared_access, ctor,
                                 std::move(attrs));
}

}  // namespace tint::ast
