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

#include "src/ast/type/struct_type.h"

#include <utility>

namespace tint {
namespace ast {
namespace type {

StructType::StructType(std::unique_ptr<Struct> impl)
    : struct_(std::move(impl)) {}

StructType::StructType(StructType&&) = default;

bool StructType::IsStruct() const {
  return true;
}

std::string StructType::type_name() const {
  return "__struct_" + name_;
}

StructType::~StructType() = default;

}  // namespace type
}  // namespace ast
}  // namespace tint
