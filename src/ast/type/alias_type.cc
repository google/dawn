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

#include "src/ast/type/alias_type.h"

#include <assert.h>

namespace tint {
namespace ast {
namespace type {

AliasType::AliasType(const std::string& name, Type* subtype)
    : name_(name), subtype_(subtype) {
  assert(subtype_);
}

AliasType::~AliasType() = default;

bool AliasType::IsAlias() const {
  return true;
}

std::string AliasType::type_name() const {
  return "__alias_" + name_ + subtype_->type_name();
}

}  // namespace type
}  // namespace ast
}  // namespace tint
