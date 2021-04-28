// Copyright 2021 The Tint Authors.
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

#include "src/ast/type_name.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::TypeName);

namespace tint {
namespace ast {

TypeName::TypeName(ProgramID program_id, const Source& source, Symbol name)
    : Base(program_id, source),
      name_(name),
      type_name_("__type_name_" + name.to_str()) {}

TypeName::~TypeName() = default;

TypeName::TypeName(TypeName&&) = default;

std::string TypeName::type_name() const {
  return type_name_;
}

std::string TypeName::FriendlyName(const SymbolTable& symbols) const {
  return symbols.NameFor(name_);
}

TypeName* TypeName::Clone(CloneContext* ctx) const {
  auto src = ctx->Clone(source());
  auto n = ctx->Clone(name());
  return ctx->dst->create<TypeName>(src, n);
}

}  // namespace ast
}  // namespace tint
