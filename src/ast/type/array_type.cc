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

#include "src/ast/type/array_type.h"

namespace tint {
namespace ast {
namespace type {

ArrayType::ArrayType(Type* subtype) : subtype_(subtype) {}

ArrayType::ArrayType(Type* subtype, uint32_t size)
    : subtype_(subtype), size_(size) {}

ArrayType::ArrayType(ArrayType&&) = default;

ArrayType::~ArrayType() = default;

bool ArrayType::IsArray() const {
  return true;
}

std::string ArrayType::type_name() const {
  assert(subtype_);

  std::string type_name = "__array" + subtype_->type_name();
  if (!IsRuntimeArray())
    type_name += "_" + std::to_string(size_);
  if (has_array_stride())
    type_name += "_stride_" + std::to_string(array_stride_);

  return type_name;
}

}  // namespace type
}  // namespace ast
}  // namespace tint
