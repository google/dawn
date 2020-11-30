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

#include "src/ast/type/multisampled_texture_type.h"

#include <cassert>
#include <sstream>

namespace tint {
namespace ast {
namespace type {

MultisampledTextureType::MultisampledTextureType(TextureDimension dim,
                                                 Type* type)
    : Base(dim), type_(type) {
  assert(type_);
}

MultisampledTextureType::MultisampledTextureType(MultisampledTextureType&&) =
    default;

MultisampledTextureType::~MultisampledTextureType() = default;

bool MultisampledTextureType::IsMultisampled() const {
  return true;
}

std::string MultisampledTextureType::type_name() const {
  std::ostringstream out;
  out << "__multisampled_texture_" << dim() << type_->type_name();
  return out.str();
}

}  // namespace type
}  // namespace ast
}  // namespace tint
