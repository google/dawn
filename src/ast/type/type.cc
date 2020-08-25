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
#include "src/ast/type/sampler_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/texture_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type/void_type.h"

namespace tint {
namespace ast {
namespace type {

Type::Type() = default;

Type::~Type() = default;

Type* Type::UnwrapPtrIfNeeded() {
  if (IsPointer()) {
    return AsPointer()->type();
  }
  return this;
}

Type* Type::UnwrapAliasesIfNeeded() {
  auto* where = this;
  while (where->IsAlias()) {
        where = where->AsAlias()->type();
  }
  return where;
}

Type* Type::UnwrapAliasPtrAlias() {
  return UnwrapAliasesIfNeeded()->UnwrapPtrIfNeeded()->UnwrapAliasesIfNeeded();
}

bool Type::IsAlias() const {
  return false;
}

bool Type::IsArray() const {
  return false;
}

bool Type::IsBool() const {
  return false;
}

bool Type::IsF32() const {
  return false;
}

bool Type::IsI32() const {
  return false;
}

bool Type::IsMatrix() const {
  return false;
}

bool Type::IsPointer() const {
  return false;
}

bool Type::IsSampler() const {
  return false;
}

bool Type::IsStruct() const {
  return false;
}

bool Type::IsTexture() const {
  return false;
}

bool Type::IsU32() const {
  return false;
}

bool Type::IsVector() const {
  return false;
}

bool Type::IsVoid() const {
  return false;
}

bool Type::is_float_scalar() {
  return IsF32();
}

bool Type::is_float_matrix() {
  return IsMatrix() && AsMatrix()->type()->is_float_scalar();
}

bool Type::is_float_vector() {
  return IsVector() && AsVector()->type()->is_float_scalar();
}

bool Type::is_float_scalar_or_vector() {
  return is_float_scalar() || is_float_vector();
}

bool Type::is_integer_scalar() {
  return IsU32() || IsI32();
}

bool Type::is_unsigned_integer_vector() {
  return IsVector() && AsVector()->type()->IsU32();
}

bool Type::is_signed_integer_vector() {
  return IsVector() && AsVector()->type()->IsI32();
}

bool Type::is_unsigned_scalar_or_vector() {
  return IsU32() || (IsVector() && AsVector()->type()->IsU32());
}

bool Type::is_signed_scalar_or_vector() {
  return IsI32() || (IsVector() && AsVector()->type()->IsI32());
}

bool Type::is_integer_scalar_or_vector() {
  return is_unsigned_scalar_or_vector() || is_signed_scalar_or_vector();
}

const AliasType* Type::AsAlias() const {
  assert(IsAlias());
  return static_cast<const AliasType*>(this);
}

const ArrayType* Type::AsArray() const {
  assert(IsArray());
  return static_cast<const ArrayType*>(this);
}

const BoolType* Type::AsBool() const {
  assert(IsBool());
  return static_cast<const BoolType*>(this);
}

const F32Type* Type::AsF32() const {
  assert(IsF32());
  return static_cast<const F32Type*>(this);
}

const I32Type* Type::AsI32() const {
  assert(IsI32());
  return static_cast<const I32Type*>(this);
}

const MatrixType* Type::AsMatrix() const {
  assert(IsMatrix());
  return static_cast<const MatrixType*>(this);
}

const PointerType* Type::AsPointer() const {
  assert(IsPointer());
  return static_cast<const PointerType*>(this);
}

const SamplerType* Type::AsSampler() const {
  assert(IsSampler());
  return static_cast<const SamplerType*>(this);
}

const StructType* Type::AsStruct() const {
  assert(IsStruct());
  return static_cast<const StructType*>(this);
}

const TextureType* Type::AsTexture() const {
  assert(IsTexture());
  return static_cast<const TextureType*>(this);
}

const U32Type* Type::AsU32() const {
  assert(IsU32());
  return static_cast<const U32Type*>(this);
}

const VectorType* Type::AsVector() const {
  assert(IsVector());
  return static_cast<const VectorType*>(this);
}

const VoidType* Type::AsVoid() const {
  assert(IsVoid());
  return static_cast<const VoidType*>(this);
}

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

SamplerType* Type::AsSampler() {
  assert(IsSampler());
  return static_cast<SamplerType*>(this);
}

StructType* Type::AsStruct() {
  assert(IsStruct());
  return static_cast<StructType*>(this);
}

TextureType* Type::AsTexture() {
  assert(IsTexture());
  return static_cast<TextureType*>(this);
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
