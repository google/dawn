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

#include "src/tint/sem/type.h"

#include "src/tint/sem/bool.h"
#include "src/tint/sem/f32.h"
#include "src/tint/sem/i32.h"
#include "src/tint/sem/matrix.h"
#include "src/tint/sem/pointer.h"
#include "src/tint/sem/reference.h"
#include "src/tint/sem/sampler.h"
#include "src/tint/sem/texture.h"
#include "src/tint/sem/u32.h"
#include "src/tint/sem/vector.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Type);

namespace tint::sem {

Type::Type() = default;

Type::Type(Type&&) = default;

Type::~Type() = default;

const Type* Type::UnwrapPtr() const {
  auto* type = this;
  while (auto* ptr = type->As<sem::Pointer>()) {
    type = ptr->StoreType();
  }
  return type;
}

const Type* Type::UnwrapRef() const {
  auto* type = this;
  if (auto* ref = type->As<sem::Reference>()) {
    type = ref->StoreType();
  }
  return type;
}

uint32_t Type::Size() const {
  return 0;
}

uint32_t Type::Align() const {
  return 0;
}

bool Type::IsConstructible() const {
  return false;
}

bool Type::is_scalar() const {
  return IsAnyOf<F32, U32, I32, Bool>();
}

bool Type::is_numeric_scalar() const {
  return IsAnyOf<F32, U32, I32>();
}

bool Type::is_float_scalar() const {
  return Is<F32>();
}

bool Type::is_float_matrix() const {
  return Is([](const Matrix* m) { return m->type()->is_float_scalar(); });
}

bool Type::is_square_float_matrix() const {
  return Is([](const Matrix* m) {
    return m->type()->is_float_scalar() && m->rows() == m->columns();
  });
}

bool Type::is_float_vector() const {
  return Is([](const Vector* v) { return v->type()->is_float_scalar(); });
}

bool Type::is_float_scalar_or_vector() const {
  return is_float_scalar() || is_float_vector();
}

bool Type::is_float_scalar_or_vector_or_matrix() const {
  return is_float_scalar() || is_float_vector() || is_float_matrix();
}

bool Type::is_integer_scalar() const {
  return IsAnyOf<U32, I32>();
}

bool Type::is_signed_integer_scalar() const {
  return Is<I32>();
}

bool Type::is_unsigned_integer_scalar() const {
  return Is<U32>();
}

bool Type::is_signed_integer_vector() const {
  return Is([](const Vector* v) { return v->type()->Is<I32>(); });
}

bool Type::is_unsigned_integer_vector() const {
  return Is([](const Vector* v) { return v->type()->Is<U32>(); });
}

bool Type::is_unsigned_scalar_or_vector() const {
  return Is<U32>() || is_unsigned_integer_vector();
}

bool Type::is_signed_scalar_or_vector() const {
  return Is<I32>() || is_signed_integer_vector();
}

bool Type::is_integer_scalar_or_vector() const {
  return is_unsigned_scalar_or_vector() || is_signed_scalar_or_vector();
}

bool Type::is_bool_vector() const {
  return Is([](const Vector* v) { return v->type()->Is<Bool>(); });
}

bool Type::is_bool_scalar_or_vector() const {
  return Is<Bool>() || is_bool_vector();
}

bool Type::is_numeric_vector() const {
  return Is([](const Vector* v) { return v->type()->is_numeric_scalar(); });
}

bool Type::is_scalar_vector() const {
  return Is([](const Vector* v) { return v->type()->is_scalar(); });
}

bool Type::is_numeric_scalar_or_vector() const {
  return is_numeric_scalar() || is_numeric_vector();
}

bool Type::is_handle() const {
  return IsAnyOf<Sampler, Texture>();
}

}  // namespace tint::sem
