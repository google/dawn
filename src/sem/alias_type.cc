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

#include "src/sem/alias_type.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Alias);

namespace tint {
namespace sem {

Alias::Alias(const Symbol& sym, const Type* subtype)
    : symbol_(sym),
      subtype_(subtype),
      type_name_("__alias_" + sym.to_str() + subtype->type_name()) {
  TINT_ASSERT(subtype_);
}

Alias::Alias(Alias&&) = default;

Alias::~Alias() = default;

std::string Alias::type_name() const {
  return type_name_;
}

std::string Alias::FriendlyName(const SymbolTable& symbols) const {
  return symbols.NameFor(symbol_);
}

}  // namespace sem
}  // namespace tint
