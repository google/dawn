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

#include "src/tint/ast/let.h"

#include <utility>

#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Let);

namespace tint::ast {

Let::Let(ProgramID pid,
         NodeID nid,
         const Source& src,
         const Identifier* n,
         Type ty,
         const Expression* init,
         utils::VectorRef<const Attribute*> attrs)
    : Base(pid, nid, src, n, ty, init, std::move(attrs)) {
    TINT_ASSERT(AST, init != nullptr);
}

Let::~Let() = default;

const char* Let::Kind() const {
    return "let";
}

const Let* Let::Clone(CloneContext* ctx) const {
    auto src = ctx->Clone(source);
    auto* n = ctx->Clone(name);
    auto ty = ctx->Clone(type);
    auto* init = ctx->Clone(initializer);
    auto attrs = ctx->Clone(attributes);
    return ctx->dst->create<Let>(src, n, ty, init, std::move(attrs));
}

}  // namespace tint::ast
