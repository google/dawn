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

#include "src/ast/type/type.h"

#include <assert.h>

#include "src/ast/type/access_control_type.h"
#include "src/ast/type/alias_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/sampler_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/texture_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type/void_type.h"

TINT_INSTANTIATE_CLASS_ID(tint::ast::type::Type);

namespace tint {
namespace ast {
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

bool Type::is_scalar() {
  return is_float_scalar() || is_integer_scalar() || Is<Bool>();
}

bool Type::is_float_scalar() {
  return Is<F32>();
}

bool Type::is_float_matrix() {
  return Is<Matrix>() && As<Matrix>()->type()->is_float_scalar();
}

bool Type::is_float_vector() {
  return Is<Vector>() && As<Vector>()->type()->is_float_scalar();
}

bool Type::is_float_scalar_or_vector() {
  return is_float_scalar() || is_float_vector();
}

bool Type::is_integer_scalar() {
  return Is<U32>() || Is<I32>();
}

bool Type::is_unsigned_integer_vector() {
  return Is<Vector>() && As<Vector>()->type()->Is<U32>();
}

bool Type::is_signed_integer_vector() {
  return Is<Vector>() && As<Vector>()->type()->Is<I32>();
}

bool Type::is_unsigned_scalar_or_vector() {
  return Is<U32>() || (Is<Vector>() && As<Vector>()->type()->Is<U32>());
}

bool Type::is_signed_scalar_or_vector() {
  return Is<I32>() || (Is<Vector>() && As<Vector>()->type()->Is<I32>());
}

bool Type::is_integer_scalar_or_vector() {
  return is_unsigned_scalar_or_vector() || is_signed_scalar_or_vector();
}

}  // namespace type
}  // namespace ast
}  // namespace tint
