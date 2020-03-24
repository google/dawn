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
class StructType;
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
  virtual bool IsAlias() const { return false; }
  /// @returns true if the type is an array type
  virtual bool IsArray() const { return false; }
  /// @returns true if the type is a bool type
  virtual bool IsBool() const { return false; }
  /// @returns true if the type is an f32 type
  virtual bool IsF32() const { return false; }
  /// @returns true if the type is an i32 type
  virtual bool IsI32() const { return false; }
  /// @returns true if the type is a matrix type
  virtual bool IsMatrix() const { return false; }
  /// @returns true if the type is a ptr type
  virtual bool IsPointer() const { return false; }
  /// @returns true if the type is a struct type
  virtual bool IsStruct() const { return false; }
  /// @returns true if the type is a u32 type
  virtual bool IsU32() const { return false; }
  /// @returns true if the type is a vec type
  virtual bool IsVector() const { return false; }
  /// @returns true if the type is a void type
  virtual bool IsVoid() const { return false; }

  /// @returns the name for this type. The |type_name| is unique over all types.
  virtual std::string type_name() const = 0;

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
  /// @returns the type as a struct type
  StructType* AsStruct();
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
