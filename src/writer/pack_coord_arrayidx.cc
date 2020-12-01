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

#include "src/writer/pack_coord_arrayidx.h"

#include <utility>

#include "src/ast/expression.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type_constructor_expression.h"

namespace tint {
namespace writer {

namespace {

ast::TypeConstructorExpression* AsVectorConstructor(ast::Expression* expr) {
  if (auto* constructor = expr->As<ast::TypeConstructorExpression>()) {
    if (constructor->type()->Is<ast::type::Vector>()) {
      return constructor;
    }
  }
  return nullptr;
}

}  // namespace

bool PackCoordAndArrayIndex(
    ast::Expression* coords,
    ast::Expression* array_idx,
    std::function<bool(ast::TypeConstructorExpression*)> callback) {
  uint32_t packed_size;
  ast::type::Type* packed_el_ty;  // Currenly must be f32.
  if (auto* vec = coords->result_type()->As<ast::type::Vector>()) {
    packed_size = vec->size() + 1;
    packed_el_ty = vec->type();
  } else {
    packed_size = 2;
    packed_el_ty = coords->result_type();
  }

  if (!packed_el_ty) {
    return false;  // missing type info
  }

  // Cast array_idx to the vector element type
  ast::TypeConstructorExpression array_index_cast(packed_el_ty, {array_idx});
  array_index_cast.set_result_type(packed_el_ty);

  ast::type::Vector packed_ty(packed_el_ty, packed_size);

  // If the coordinates are already passed in a vector constructor, extract
  // the elements into the new vector instead of nesting a vector-in-vector.
  ast::ExpressionList packed;
  if (auto* vc = AsVectorConstructor(coords)) {
    packed = vc->values();
  } else {
    packed.emplace_back(coords);
  }
  packed.emplace_back(&array_index_cast);

  ast::TypeConstructorExpression constructor{&packed_ty, std::move(packed)};

  return callback(&constructor);
}

}  // namespace writer
}  // namespace tint
