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

#include "src/type/struct_type.h"

#include <cmath>

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::type::Struct);

namespace tint {
namespace type {

Struct::Struct(const Symbol& sym, ast::Struct* impl)
    : symbol_(sym), struct_(impl) {}

Struct::Struct(Struct&&) = default;

Struct::~Struct() = default;

std::string Struct::type_name() const {
  return "__struct_" + symbol_.to_str();
}

std::string Struct::FriendlyName(const SymbolTable& symbols) const {
  return symbols.NameFor(symbol_);
}

Struct* Struct::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto sym = ctx->Clone(symbol());
  auto* str = ctx->Clone(impl());
  return ctx->dst->create<Struct>(sym, str);
}

}  // namespace type
}  // namespace tint
