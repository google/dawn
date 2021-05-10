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

#include "src/ast/type.h"

#include "src/ast/access_control.h"
#include "src/ast/alias.h"
#include "src/ast/bool.h"
#include "src/ast/f32.h"
#include "src/ast/i32.h"
#include "src/ast/matrix.h"
#include "src/ast/pointer.h"
#include "src/ast/sampler.h"
#include "src/ast/texture.h"
#include "src/ast/u32.h"
#include "src/ast/vector.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Type);

namespace tint {
namespace ast {

Type::Type(ProgramID program_id, const Source& source)
    : Base(program_id, source) {}

Type::Type(Type&&) = default;

Type::~Type() = default;

Type* Type::UnwrapAll() {
  auto* type = this;
  while (true) {
    if (auto* alias = type->As<Alias>()) {
      type = alias->type();
    } else if (auto* access = type->As<AccessControl>()) {
      type = access->type();
    } else if (auto* ptr = type->As<Pointer>()) {
      type = ptr->type();
    } else {
      break;
    }
  }
  return type;
}

bool Type::is_scalar() const {
  return IsAnyOf<F32, U32, I32, Bool>();
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

bool Type::is_unsigned_integer_vector() const {
  return Is<Vector>([](const Vector* v) { return v->type()->Is<U32>(); });
}

bool Type::is_signed_integer_vector() const {
  return Is<Vector>([](const Vector* v) { return v->type()->Is<I32>(); });
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

bool Type::is_handle() const {
  return IsAnyOf<Sampler, Texture>();
}

void Type::to_str(const sem::Info&, std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << type_name();
}

}  // namespace ast
}  // namespace tint
