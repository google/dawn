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

#include "src/type/array_type.h"

#include <cmath>
#include <memory>

#include "src/ast/stride_decoration.h"
#include "src/clone_context.h"
#include "src/program_builder.h"
#include "src/type/vector_type.h"

TINT_INSTANTIATE_TYPEINFO(tint::type::Array);

namespace tint {
namespace type {

Array::Array(Type* subtype, uint32_t size, ast::ArrayDecorationList decorations)
    : subtype_(subtype), size_(size), decos_(decorations) {}

Array::Array(Array&&) = default;

Array::~Array() = default;

uint64_t Array::MinBufferBindingSize(MemoryLayout mem_layout) const {
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

uint64_t Array::BaseAlignment(MemoryLayout mem_layout) const {
  if (mem_layout == MemoryLayout::kUniformBuffer) {
    float aligment = 16;  // for a vec4
    float unaligned = static_cast<float>(subtype_->BaseAlignment(mem_layout));
    return static_cast<uint64_t>(aligment * std::ceil(unaligned / aligment));
  } else if (mem_layout == MemoryLayout::kStorageBuffer) {
    return subtype_->BaseAlignment(mem_layout);
  }
  return 0;
}

uint32_t Array::array_stride() const {
  for (auto* deco : decos_) {
    if (auto* stride = deco->As<ast::StrideDecoration>()) {
      return stride->stride();
    }
  }
  return 0;
}

bool Array::has_array_stride() const {
  for (auto* deco : decos_) {
    if (deco->Is<ast::StrideDecoration>()) {
      return true;
    }
  }
  return false;
}

std::string Array::type_name() const {
  assert(subtype_);

  std::string type_name = "__array" + subtype_->type_name();
  if (!IsRuntimeArray())
    type_name += "_" + std::to_string(size_);
  if (has_array_stride())
    type_name += "_stride_" + std::to_string(array_stride());

  return type_name;
}

std::string Array::FriendlyName(const SymbolTable& symbols) const {
  std::ostringstream out;
  if (has_array_stride()) {
    out << "[[stride(" << array_stride() << ")]] ";
  }
  out << "array<" << subtype_->FriendlyName(symbols);
  if (!IsRuntimeArray()) {
    out << ", " << size_;
  }
  out << ">";
  return out.str();
}

Array* Array::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto* ty = ctx->Clone(type());
  auto decos = ctx->Clone(decorations());
  return ctx->dst->create<Array>(ty, size_, decos);
}

}  // namespace type
}  // namespace tint
