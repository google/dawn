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

#include "src/writer/append_vector.h"

#include <utility>

#include "src/ast/expression.h"
#include "src/ast/type_constructor_expression.h"
#include "src/type/vector_type.h"

namespace tint {
namespace writer {

namespace {

ast::TypeConstructorExpression* AsVectorConstructor(ast::Expression* expr) {
  if (auto* constructor = expr->As<ast::TypeConstructorExpression>()) {
    if (constructor->type()->Is<type::Vector>()) {
      return constructor;
    }
  }
  return nullptr;
}

}  // namespace

bool AppendVector(
    ast::Expression* vector,
    ast::Expression* scalar,
    std::function<bool(ast::TypeConstructorExpression*)> callback) {
  uint32_t packed_size;
  type::Type* packed_el_ty;  // Currently must be f32.
  if (auto* vec = vector->result_type()->As<type::Vector>()) {
    packed_size = vec->size() + 1;
    packed_el_ty = vec->type();
  } else {
    packed_size = 2;
    packed_el_ty = vector->result_type();
  }

  if (!packed_el_ty) {
    return false;  // missing type info
  }

  // Cast scalar to the vector element type
  ast::TypeConstructorExpression scalar_cast(Source{}, packed_el_ty, {scalar});
  scalar_cast.set_result_type(packed_el_ty);

  type::Vector packed_ty(packed_el_ty, packed_size);

  // If the coordinates are already passed in a vector constructor, extract
  // the elements into the new vector instead of nesting a vector-in-vector.
  ast::ExpressionList packed;
  if (auto* vc = AsVectorConstructor(vector)) {
    packed = vc->values();
  } else {
    packed.emplace_back(vector);
  }
  if (packed_el_ty != scalar->result_type()) {
    packed.emplace_back(&scalar_cast);
  } else {
    packed.emplace_back(scalar);
  }

  ast::TypeConstructorExpression constructor{Source{}, &packed_ty,
                                             std::move(packed)};
  constructor.set_result_type(&packed_ty);

  return callback(&constructor);
}

}  // namespace writer
}  // namespace tint
