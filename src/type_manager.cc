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

#include "src/type_manager.h"

#include <utility>

namespace tint {

TypeManager::TypeManager() = default;

TypeManager::~TypeManager() = default;

void TypeManager::Reset() {
  types_.clear();
}

ast::type::Type* TypeManager::Get(std::unique_ptr<ast::type::Type> type) {
  auto name = type->type_name();

  if (types_.find(name) == types_.end()) {
    types_[name] = std::move(type);
  }
  return types_.find(name)->second.get();
}

}  // namespace tint
