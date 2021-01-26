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

#include "src/type/vector_type.h"

#include <assert.h>
#include <cmath>

#include "src/clone_context.h"
#include "src/program.h"

TINT_INSTANTIATE_CLASS_ID(tint::type::Vector);

namespace tint {
namespace type {

Vector::Vector(Type* subtype, uint32_t size) : subtype_(subtype), size_(size) {
  assert(size_ > 1);
  assert(size_ < 5);
}

Vector::Vector(Vector&&) = default;

Vector::~Vector() = default;

std::string Vector::type_name() const {
  return "__vec_" + std::to_string(size_) + subtype_->type_name();
}

uint64_t Vector::MinBufferBindingSize(MemoryLayout mem_layout) const {
  return size_ * subtype_->MinBufferBindingSize(mem_layout);
}

uint64_t Vector::BaseAlignment(MemoryLayout mem_layout) const {
  if (size_ == 2) {
    return 2 * subtype_->BaseAlignment(mem_layout);
  } else if (size_ == 3 || size_ == 4) {
    return 4 * subtype_->BaseAlignment(mem_layout);
  }

  return 0;  // vectors are only supposed to have 2, 3, or 4 elements.
}

Vector* Vector::Clone(CloneContext* ctx) const {
  return ctx->dst->create<Vector>(ctx->Clone(subtype_), size_);
}

}  // namespace type
}  // namespace tint
