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

#ifndef SRC_AST_TYPE_TYPE_H_
#define SRC_AST_TYPE_TYPE_H_

#include <string>

namespace tint {
namespace ast {
namespace type {

class AliasType;
class ArrayType;
class BoolType;
class F32Type;
class I32Type;
class MatrixType;
class PointerType;
class SamplerType;
class StructType;
class TextureType;
class U32Type;
class VectorType;
class VoidType;

/// Base class for a type in the system
class Type {
 public:
  /// Move constructor
  Type(Type&&) = default;
  virtual ~Type();

  /// @returns true if the type is an alias type
  virtual bool IsAlias() const;
  /// @returns true if the type is an array type
  virtual bool IsArray() const;
  /// @returns true if the type is a bool type
  virtual bool IsBool() const;
  /// @returns true if the type is an f32 type
  virtual bool IsF32() const;
  /// @returns true if the type is an i32 type
  virtual bool IsI32() const;
  /// @returns true if the type is a matrix type
  virtual bool IsMatrix() const;
  /// @returns true if the type is a ptr type
  virtual bool IsPointer() const;
  /// @returns true if the type is a sampler
  virtual bool IsSampler() const;
  /// @returns true if the type is a struct type
  virtual bool IsStruct() const;
  /// @returns true if the type is a texture type
  virtual bool IsTexture() const;
  /// @returns true if the type is a u32 type
  virtual bool IsU32() const;
  /// @returns true if the type is a vec type
  virtual bool IsVector() const;
  /// @returns true if the type is a void type
  virtual bool IsVoid() const;

  /// @returns the name for this type. The |type_name| is unique over all types.
  virtual std::string type_name() const = 0;

  /// @returns the pointee type if this is a pointer, |this| otherwise
  Type* UnwrapPtrIfNeeded();

  /// Removes all levels of aliasing, if this is an alias type.  Otherwise
  /// returns |this|.  This is just enough to assist with WGSL translation
  /// in that you want see through one level of pointer to get from an
  /// identifier-like expression as an l-value to its corresponding r-value,
  /// plus see through the aliases on either side.
  /// @returns the completely unaliased type.
  Type* UnwrapAliasesIfNeeded();

  /// Returns the type found after:
  /// - removing all layers of aliasing if they exist, then
  /// - removing the pointer, if it exists, then
  /// - removing all further layers of aliasing, if they exist
  /// @returns the unwrapped type
  Type* UnwrapAliasPtrAlias();

  /// @returns true if this type is a float scalar
  bool is_float_scalar();
  /// @returns true if this type is a float matrix
  bool is_float_matrix();
  /// @returns true if this type is a float vector
  bool is_float_vector();
  /// @returns true if this type is a float scalar or vector
  bool is_float_scalar_or_vector();
  /// @returns ture if this type is an integer scalar
  bool is_integer_scalar();
  /// @returns true if this type is a signed integer vector
  bool is_signed_integer_vector();
  /// @returns true if this type is an unsigned vector
  bool is_unsigned_integer_vector();
  /// @returns true if this type is an unsigned scalar or vector
  bool is_unsigned_scalar_or_vector();
  /// @returns true if this type is a signed scalar or vector
  bool is_signed_scalar_or_vector();
  /// @returns true if this type is an integer scalar or vector
  bool is_integer_scalar_or_vector();

  /// @returns the type as an alias type
  const AliasType* AsAlias() const;
  /// @returns the type as an array type
  const ArrayType* AsArray() const;
  /// @returns the type as a bool type
  const BoolType* AsBool() const;
  /// @returns the type as a f32 type
  const F32Type* AsF32() const;
  /// @returns the type as an i32 type
  const I32Type* AsI32() const;
  /// @returns the type as a matrix type
  const MatrixType* AsMatrix() const;
  /// @returns the type as a pointer type
  const PointerType* AsPointer() const;
  /// @returns the type as a sampler type
  const SamplerType* AsSampler() const;
  /// @returns the type as a struct type
  const StructType* AsStruct() const;
  /// @returns the type as a texture type
  const TextureType* AsTexture() const;
  /// @returns the type as a u32 type
  const U32Type* AsU32() const;
  /// @returns the type as a vector type
  const VectorType* AsVector() const;
  /// @returns the type as a void type
  const VoidType* AsVoid() const;

  /// @returns the type as an alias type
  AliasType* AsAlias();
  /// @returns the type as an array type
  ArrayType* AsArray();
  /// @returns the type as a bool type
  BoolType* AsBool();
  /// @returns the type as a f32 type
  F32Type* AsF32();
  /// @returns the type as an i32 type
  I32Type* AsI32();
  /// @returns the type as a matrix type
  MatrixType* AsMatrix();
  /// @returns the type as a pointer type
  PointerType* AsPointer();
  /// @returns the type as a sampler type
  SamplerType* AsSampler();
  /// @returns the type as a struct type
  StructType* AsStruct();
  /// @returns the type as a texture type
  TextureType* AsTexture();
  /// @returns the type as a u32 type
  U32Type* AsU32();
  /// @returns the type as a vector type
  VectorType* AsVector();
  /// @returns the type as a void type
  VoidType* AsVoid();

 protected:
  Type();
};

}  // namespace type
}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_TYPE_TYPE_H_
