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

#include "src/sem/type.h"

#include "src/debug.h"
#include "src/sem/array.h"
#include "src/sem/atomic_type.h"
#include "src/sem/bool_type.h"
#include "src/sem/f32_type.h"
#include "src/sem/i32_type.h"
#include "src/sem/matrix_type.h"
#include "src/sem/pointer_type.h"
#include "src/sem/reference_type.h"
#include "src/sem/sampler_type.h"
#include "src/sem/struct.h"
#include "src/sem/texture_type.h"
#include "src/sem/u32_type.h"
#include "src/sem/vector_type.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Type);

namespace tint {
namespace sem {

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

void Type::GetDefaultAlignAndSize(uint32_t& align, uint32_t& size) const {
  TINT_ASSERT(Semantic, !As<Reference>());
  TINT_ASSERT(Semantic, !As<Pointer>());

  static constexpr uint32_t vector_size[] = {
      /* padding */ 0,
      /* padding */ 0,
      /*vec2*/ 8,
      /*vec3*/ 12,
      /*vec4*/ 16,
  };
  static constexpr uint32_t vector_align[] = {
      /* padding */ 0,
      /* padding */ 0,
      /*vec2*/ 8,
      /*vec3*/ 16,
      /*vec4*/ 16,
  };

  if (is_scalar()) {
    // Note: Also captures booleans, but these are not host-shareable.
    align = 4;
    size = 4;
    return;
  }
  if (auto* vec = As<Vector>()) {
    TINT_ASSERT(Semantic, vec->Width() >= 2 && vec->Width() <= 4);
    align = vector_align[vec->Width()];
    size = vector_size[vec->Width()];
    return;
  }
  if (auto* mat = As<Matrix>()) {
    TINT_ASSERT(Semantic, mat->columns() >= 2 && mat->columns() <= 4);
    TINT_ASSERT(Semantic, mat->rows() >= 2 && mat->rows() <= 4);
    align = vector_align[mat->rows()];
    size = vector_align[mat->rows()] * mat->columns();
    return;
  }
  if (auto* s = As<Struct>()) {
    align = s->Align();
    size = s->Size();
    return;
  }
  if (auto* a = As<Array>()) {
    align = a->Align();
    size = a->SizeInBytes();
    return;
  }
  if (auto* a = As<Atomic>()) {
    return a->Type()->GetDefaultAlignAndSize(align, size);
  }

  TINT_ASSERT(Semantic, false);
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
  return Is<Matrix>(
      [](const Matrix* m) { return m->type()->is_float_scalar(); });
}

bool Type::is_float_vector() const {
  return Is<Vector>(
      [](const Vector* v) { return v->type()->is_float_scalar(); });
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
  return Is<Vector>([](const Vector* v) { return v->type()->Is<I32>(); });
}

bool Type::is_unsigned_integer_vector() const {
  return Is<Vector>([](const Vector* v) { return v->type()->Is<U32>(); });
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
  return Is<Vector>([](const Vector* v) { return v->type()->Is<Bool>(); });
}

bool Type::is_bool_scalar_or_vector() const {
  return Is<Bool>() || is_bool_vector();
}

bool Type::is_numeric_vector() const {
  return Is<Vector>(
      [](const Vector* v) { return v->type()->is_numeric_scalar(); });
}

bool Type::is_scalar_vector() const {
  return Is<Vector>([](const Vector* v) { return v->type()->is_scalar(); });
}

bool Type::is_numeric_scalar_or_vector() const {
  return is_numeric_scalar() || is_numeric_vector();
}

bool Type::is_handle() const {
  return IsAnyOf<Sampler, Texture>();
}

}  // namespace sem
}  // namespace tint
