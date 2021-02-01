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

#include "src/type/type.h"

#include <assert.h>

#include "src/type/access_control_type.h"
#include "src/type/alias_type.h"
#include "src/type/array_type.h"
#include "src/type/bool_type.h"
#include "src/type/f32_type.h"
#include "src/type/i32_type.h"
#include "src/type/matrix_type.h"
#include "src/type/pointer_type.h"
#include "src/type/sampler_type.h"
#include "src/type/struct_type.h"
#include "src/type/texture_type.h"
#include "src/type/u32_type.h"
#include "src/type/vector_type.h"
#include "src/type/void_type.h"

TINT_INSTANTIATE_CLASS_ID(tint::type::Type);

namespace tint {
namespace type {

Type::Type() = default;

Type::Type(Type&&) = default;

Type::~Type() = default;

Type* Type::UnwrapPtrIfNeeded() {
  if (auto* ptr = As<type::Pointer>()) {
    return ptr->type();
  }
  return this;
}

Type* Type::UnwrapIfNeeded() {
  auto* where = this;
  while (true) {
    if (auto* alias = where->As<type::Alias>()) {
      where = alias->type();
    } else if (auto* access = where->As<type::AccessControl>()) {
      where = access->type();
    } else {
      break;
    }
  }
  return where;
}

Type* Type::UnwrapAll() {
  return UnwrapIfNeeded()->UnwrapPtrIfNeeded()->UnwrapIfNeeded();
}

uint64_t Type::MinBufferBindingSize(MemoryLayout) const {
  return 0;
}

uint64_t Type::BaseAlignment(MemoryLayout) const {
  return 0;
}

bool Type::is_scalar() const {
  return is_float_scalar() || is_integer_scalar() || Is<Bool>();
}

bool Type::is_float_scalar() const {
  return Is<F32>();
}

bool Type::is_float_matrix() const {
  return Is<Matrix>() && As<Matrix>()->type()->is_float_scalar();
}

bool Type::is_float_vector() const {
  return Is<Vector>() && As<Vector>()->type()->is_float_scalar();
}

bool Type::is_float_scalar_or_vector() const {
  return is_float_scalar() || is_float_vector();
}

bool Type::is_integer_scalar() const {
  return Is<U32>() || Is<I32>();
}

bool Type::is_unsigned_integer_vector() const {
  return Is<Vector>() && As<Vector>()->type()->Is<U32>();
}

bool Type::is_signed_integer_vector() const {
  return Is<Vector>() && As<Vector>()->type()->Is<I32>();
}

bool Type::is_unsigned_scalar_or_vector() const {
  return Is<U32>() || (Is<Vector>() && As<Vector>()->type()->Is<U32>());
}

bool Type::is_signed_scalar_or_vector() const {
  return Is<I32>() || (Is<Vector>() && As<Vector>()->type()->Is<I32>());
}

bool Type::is_integer_scalar_or_vector() const {
  return is_unsigned_scalar_or_vector() || is_signed_scalar_or_vector();
}

bool Type::is_bool_vector() const {
  return Is<Vector>() && As<Vector>()->type()->Is<Bool>();
}

bool Type::is_bool_scalar_or_vector() const {
  return Is<Bool>() || is_bool_vector();
}

}  // namespace type
}  // namespace tint
