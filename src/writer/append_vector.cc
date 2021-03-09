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

#include "src/semantic/expression.h"

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

ast::TypeConstructorExpression* AppendVector(ProgramBuilder* b,
                                             ast::Expression* vector,
                                             ast::Expression* scalar) {
  uint32_t packed_size;
  type::Type* packed_el_ty;  // Currently must be f32.
  auto* vector_sem = b->Sem().Get(vector);
  if (auto* vec = vector_sem->Type()->As<type::Vector>()) {
    packed_size = vec->size() + 1;
    packed_el_ty = vec->type();
  } else {
    packed_size = 2;
    packed_el_ty = vector_sem->Type();
  }

  auto* statement = vector_sem->Stmt();

  // Cast scalar to the vector element type
  auto* scalar_cast = b->Construct(packed_el_ty, scalar);
  b->Sem().Add(scalar_cast, b->create<semantic::Expression>(
                                scalar_cast, packed_el_ty, statement));

  auto* packed_ty = b->create<type::Vector>(packed_el_ty, packed_size);

  // If the coordinates are already passed in a vector constructor, extract
  // the elements into the new vector instead of nesting a vector-in-vector.
  ast::ExpressionList packed;
  if (auto* vc = AsVectorConstructor(vector)) {
    packed = vc->values();
  } else {
    packed.emplace_back(vector);
  }
  if (packed_el_ty != b->Sem().Get(scalar)->Type()) {
    packed.emplace_back(scalar_cast);
  } else {
    packed.emplace_back(scalar);
  }

  auto* constructor = b->Construct(packed_ty, std::move(packed));
  b->Sem().Add(constructor, b->create<semantic::Expression>(
                                constructor, packed_ty, statement));

  return constructor;
}

}  // namespace writer
}  // namespace tint
