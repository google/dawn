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

#include "src/ast/struct.h"

#include <string>

#include "src/ast/struct_block_decoration.h"
#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Struct);

namespace tint {
namespace ast {

Struct::Struct(ProgramID pid,
               const Source& src,
               Symbol n,
               StructMemberList m,
               DecorationList decos)
    : Base(pid, src, n), members(std::move(m)), decorations(std::move(decos)) {
  for (auto* mem : members) {
    TINT_ASSERT(AST, mem);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, mem, program_id);
  }
  for (auto* deco : decorations) {
    TINT_ASSERT(AST, deco);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, deco, program_id);
  }
}

Struct::Struct(Struct&&) = default;

Struct::~Struct() = default;

bool Struct::IsBlockDecorated() const {
  return HasDecoration<StructBlockDecoration>(decorations);
}

const Struct* Struct::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source);
  auto n = ctx->Clone(name);
  auto mem = ctx->Clone(members);
  auto decos = ctx->Clone(decorations);
  return ctx->dst->create<Struct>(src, n, mem, decos);
}

}  // namespace ast
}  // namespace tint
