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

TINT_INSTANTIATE_TYPEINFO(tint::type::StructType);

namespace tint {
namespace type {

StructType::StructType(const Symbol& sym, ast::Struct* impl)
    : symbol_(sym), struct_(impl) {}

StructType::StructType(StructType&&) = default;

StructType::~StructType() = default;

std::string StructType::type_name() const {
  return "__struct_" + symbol_.to_str();
}

std::string StructType::FriendlyName(const SymbolTable& symbols) const {
  return symbols.NameFor(symbol_);
}

StructType* StructType::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto sym = ctx->Clone(symbol());
  auto* str = ctx->Clone(impl());
  return ctx->dst->create<StructType>(sym, str);
}

}  // namespace type
}  // namespace tint
