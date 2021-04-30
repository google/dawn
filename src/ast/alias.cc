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

#include "src/ast/alias.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Alias);

namespace tint {
namespace ast {

Alias::Alias(ProgramID program_id,
             const Source& source,
             const Symbol& name,
             Type* subtype)
    : Base(program_id, source, name),
      subtype_(subtype),
      type_name_("__alias_" + name.to_str() + subtype->type_name()) {
  TINT_ASSERT(subtype_);
}

Alias::Alias(Alias&&) = default;

Alias::~Alias() = default;

std::string Alias::type_name() const {
  return type_name_;
}

Alias* Alias::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source());
  auto sym = ctx->Clone(name());
  auto* ty = ctx->Clone(type());
  return ctx->dst->create<Alias>(src, sym, ty);
}

}  // namespace ast
}  // namespace tint
