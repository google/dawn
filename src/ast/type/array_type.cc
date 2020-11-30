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

#include <cmath>

#include "src/ast/stride_decoration.h"
#include "src/ast/type/vector_type.h"

namespace tint {
namespace ast {
namespace type {

ArrayType::ArrayType(Type* subtype) : subtype_(subtype) {}

ArrayType::ArrayType(Type* subtype, uint32_t size)
    : subtype_(subtype), size_(size) {}

ArrayType::ArrayType(ArrayType&&) = default;

ArrayType::~ArrayType() = default;

uint64_t ArrayType::MinBufferBindingSize(MemoryLayout mem_layout) const {
  if (!has_array_stride()) {
    // Arrays in buffers are required to have a stride.
    return 0;
  }

  if (IsRuntimeArray()) {
    // WebGPU spec 10.1.2:
    // If the last field of the corresponding structure defined in the shader
    // has an unbounded array type, then the value of minBufferBindingSize must
    // be greater than or equal to the byte offset of that field plus the stride
    // of the unbounded array
    return array_stride();
  } else {
    // Not including the padding for the last element
    return (size_ - 1) * array_stride() +
           subtype_->MinBufferBindingSize(mem_layout);
  }
}

uint64_t ArrayType::BaseAlignment(MemoryLayout mem_layout) const {
  if (mem_layout == MemoryLayout::kUniformBuffer) {
    float aligment = 16;  // for a vec4
    float unaligned = static_cast<float>(subtype_->BaseAlignment(mem_layout));
    return static_cast<uint64_t>(aligment * std::ceil(unaligned / aligment));
  } else if (mem_layout == MemoryLayout::kStorageBuffer) {
    return subtype_->BaseAlignment(mem_layout);
  }
  return 0;
}

uint32_t ArrayType::array_stride() const {
  for (auto* deco : decos_) {
    if (auto* stride = deco->As<StrideDecoration>()) {
      return stride->stride();
    }
  }
  return 0;
}

bool ArrayType::has_array_stride() const {
  for (auto* deco : decos_) {
    if (deco->Is<StrideDecoration>()) {
      return true;
    }
  }
  return false;
}

std::string ArrayType::type_name() const {
  assert(subtype_);

  std::string type_name = "__array" + subtype_->type_name();
  if (!IsRuntimeArray())
    type_name += "_" + std::to_string(size_);
  if (has_array_stride())
    type_name += "_stride_" + std::to_string(array_stride());

  return type_name;
}

}  // namespace type
}  // namespace ast
}  // namespace tint
