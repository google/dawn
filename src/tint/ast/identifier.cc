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

#include "src/tint/ast/identifier.h"

#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Identifier);

namespace tint::ast {

Identifier::Identifier(ProgramID pid, NodeID nid, const Source& src, Symbol sym)
    : Base(pid, nid, src), symbol(sym) {
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, symbol, program_id);
    TINT_ASSERT(AST, symbol.IsValid());
}

Identifier::~Identifier() = default;

const Identifier* Identifier::Clone(CloneContext* ctx) const {
    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx->Clone(source);
    auto sym = ctx->Clone(symbol);
    return ctx->dst->create<Identifier>(src, sym);
}

}  // namespace tint::ast
