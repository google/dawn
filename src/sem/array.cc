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

#include "src/sem/array.h"

#include <string>

#include "src/debug.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Array);

namespace tint {
namespace sem {

Array::Array(const Type* element,
             uint32_t count,
             uint32_t align,
             uint32_t size,
             uint32_t stride,
             bool stride_implicit)
    : element_(element),
      count_(count),
      align_(align),
      size_(size),
      stride_(stride),
      stride_implicit_(stride_implicit) {
  TINT_ASSERT(element_);
}

std::string Array::type_name() const {
  std::string type_name = "__array" + element_->type_name();
  type_name += "_count_" + std::to_string(count_);
  type_name += "_align_" + std::to_string(align_);
  type_name += "_size_" + std::to_string(size_);
  type_name += "_stride_" + std::to_string(stride_);
  // Note: stride_implicit is not part of the type_name string as this is a
  // property derived from the other fields.
  return type_name;
}

std::string Array::FriendlyName(const SymbolTable& symbols) const {
  std::ostringstream out;
  if (!stride_implicit_) {
    out << "[[stride(" << stride_ << ")]] ";
  }
  out << "array<" << element_->FriendlyName(symbols);
  if (!IsRuntimeSized()) {
    out << ", " << count_;
  }
  out << ">";
  return out.str();
}

}  // namespace sem
}  // namespace tint
