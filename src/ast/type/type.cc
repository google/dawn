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

#include "src/ast/type/alias_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type/void_type.h"

namespace tint {
namespace ast {
namespace type {

Type::Type() = default;

Type::~Type() = default;

AliasType* Type::AsAlias() {
  assert(IsAlias());
  return static_cast<AliasType*>(this);
}

ArrayType* Type::AsArray() {
  assert(IsArray());
  return static_cast<ArrayType*>(this);
}

BoolType* Type::AsBool() {
  assert(IsBool());
  return static_cast<BoolType*>(this);
}

F32Type* Type::AsF32() {
  assert(IsF32());
  return static_cast<F32Type*>(this);
}

I32Type* Type::AsI32() {
  assert(IsI32());
  return static_cast<I32Type*>(this);
}

MatrixType* Type::AsMatrix() {
  assert(IsMatrix());
  return static_cast<MatrixType*>(this);
}

PointerType* Type::AsPointer() {
  assert(IsPointer());
  return static_cast<PointerType*>(this);
}

StructType* Type::AsStruct() {
  assert(IsStruct());
  return static_cast<StructType*>(this);
}

U32Type* Type::AsU32() {
  assert(IsU32());
  return static_cast<U32Type*>(this);
}

VectorType* Type::AsVector() {
  assert(IsVector());
  return static_cast<VectorType*>(this);
}

VoidType* Type::AsVoid() {
  assert(IsVoid());
  return static_cast<VoidType*>(this);
}

}  // namespace type
}  // namespace ast
}  // namespace tint
