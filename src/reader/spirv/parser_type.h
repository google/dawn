// Copyright 2021 The Tint Authors.
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

#ifndef SRC_READER_SPIRV_PARSER_TYPE_H_
#define SRC_READER_SPIRV_PARSER_TYPE_H_

#include <memory>
#include <string>
#include <vector>

#include "src/ast/access_control.h"
#include "src/ast/sampler.h"
#include "src/ast/storage_class.h"
#include "src/ast/storage_texture.h"
#include "src/ast/texture.h"
#include "src/block_allocator.h"
#include "src/castable.h"

// Forward declarations
namespace tint {
class ProgramBuilder;
namespace ast {
class Type;
}  // namespace ast
}  // namespace tint

namespace tint {
namespace reader {
namespace spirv {

/// Type is the base class for all types
class Type : public Castable<Type> {
 public:
  /// @param b the ProgramBuilder used to construct the AST types
  /// @returns the constructed ast::Type node for the given type
  virtual ast::Type* Build(ProgramBuilder& b) const = 0;

  /// @returns the inner most store type if this is a pointer, `this` otherwise
  const Type* UnwrapPtr() const;

  /// @returns the inner most aliased type if this is an alias, `this` otherwise
  const Type* UnwrapAlias() const;

  /// @returns the type with all aliasing and access control removed
  const Type* UnwrapAliasAndAccess() const;

  /// @returns the type with all aliasing, access control and pointers removed
  const Type* UnwrapAll() const;

  /// @returns true if this type is a float scalar
  bool IsFloatScalar() const;
  /// @returns true if this type is a float scalar or vector
  bool IsFloatScalarOrVector() const;
  /// @returns true if this type is a float vector
  bool IsFloatVector() const;
  /// @returns true if this type is an integer scalar
  bool IsIntegerScalar() const;
  /// @returns true if this type is an integer scalar or vector
  bool IsIntegerScalarOrVector() const;
  /// @returns true if this type is a scalar
  bool IsScalar() const;
  /// @returns true if this type is a signed integer vector
  bool IsSignedIntegerVector() const;
  /// @returns true if this type is a signed scalar or vector
  bool IsSignedScalarOrVector() const;
  /// @returns true if this type is an unsigned integer vector
  bool IsUnsignedIntegerVector() const;
  /// @returns true if this type is an unsigned scalar or vector
  bool IsUnsignedScalarOrVector() const;

#ifndef NDEBUG
  /// @returns a string representation of the type, for debug purposes only
  virtual std::string String() const = 0;
#endif  // NDEBUG
};

using TypeList = std::vector<const Type*>;

/// `void` type
struct Void : public Castable<Void, Type> {
  /// @param b the ProgramBuilder used to construct the AST types
  /// @returns the constructed ast::Type node for the given type
  ast::Type* Build(ProgramBuilder& b) const override;

#ifndef NDEBUG
  /// @returns a string representation of the type, for debug purposes only
  std::string String() const override;
#endif  // NDEBUG
};

/// `bool` type
struct Bool : public Castable<Bool, Type> {
  /// @param b the ProgramBuilder used to construct the AST types
  /// @returns the constructed ast::Type node for the given type
  ast::Type* Build(ProgramBuilder& b) const override;

#ifndef NDEBUG
  /// @returns a string representation of the type, for debug purposes only
  std::string String() const override;
#endif  // NDEBUG
};

/// `u32` type
struct U32 : public Castable<U32, Type> {
  /// @param b the ProgramBuilder used to construct the AST types
  /// @returns the constructed ast::Type node for the given type
  ast::Type* Build(ProgramBuilder& b) const override;

#ifndef NDEBUG
  /// @returns a string representation of the type, for debug purposes only
  std::string String() const override;
#endif  // NDEBUG
};

/// `f32` type
struct F32 : public Castable<F32, Type> {
  /// @param b the ProgramBuilder used to construct the AST types
  /// @returns the constructed ast::Type node for the given type
  ast::Type* Build(ProgramBuilder& b) const override;

#ifndef NDEBUG
  /// @returns a string representation of the type, for debug purposes only
  std::string String() const override;
#endif  // NDEBUG
};

/// `i32` type
struct I32 : public Castable<I32, Type> {
  /// @param b the ProgramBuilder used to construct the AST types
  /// @returns the constructed ast::Type node for the given type
  ast::Type* Build(ProgramBuilder& b) const override;

#ifndef NDEBUG
  /// @returns a string representation of the type, for debug purposes only
  std::string String() const override;
#endif  // NDEBUG
};

/// `ptr<SC, T>` type
struct Pointer : public Castable<Pointer, Type> {
  /// Constructor
  /// @param ty the store type
  /// @param sc the pointer storage class
  Pointer(const Type* ty, ast::StorageClass sc);

  /// Copy constructor
  /// @param other the other type to copy
  Pointer(const Pointer& other);

  /// @param b the ProgramBuilder used to construct the AST types
  /// @returns the constructed ast::Type node for the given type
  ast::Type* Build(ProgramBuilder& b) const override;

#ifndef NDEBUG
  /// @returns a string representation of the type, for debug purposes only
  std::string String() const override;
#endif  // NDEBUG

  /// the store type
  Type const* const type;
  /// the pointer storage class
  ast::StorageClass const storage_class;
};

/// `ref<SC, T>` type
/// Note this has no AST representation, but is used for type tracking in the
/// reader.
struct Reference : public Castable<Reference, Type> {
  /// Constructor
  /// @param ty the referenced type
  /// @param sc the reference storage class
  Reference(const Type* ty, ast::StorageClass sc);

  /// Copy constructor
  /// @param other the other type to copy
  Reference(const Reference& other);

  /// @param b the ProgramBuilder used to construct the AST types
  /// @returns the constructed ast::Type node for the given type
  ast::Type* Build(ProgramBuilder& b) const override;

#ifndef NDEBUG
  /// @returns a string representation of the type, for debug purposes only
  std::string String() const override;
#endif  // NDEBUG

  /// the store type
  Type const* const type;
  /// the pointer storage class
  ast::StorageClass const storage_class;
};

/// `vecN<T>` type
struct Vector : public Castable<Vector, Type> {
  /// Constructor
  /// @param ty the element type
  /// @param sz the number of elements in the vector
  Vector(const Type* ty, uint32_t sz);

  /// Copy constructor
  /// @param other the other type to copy
  Vector(const Vector& other);

  /// @param b the ProgramBuilder used to construct the AST types
  /// @returns the constructed ast::Type node for the given type
  ast::Type* Build(ProgramBuilder& b) const override;

#ifndef NDEBUG
  /// @returns a string representation of the type, for debug purposes only
  std::string String() const override;
#endif  // NDEBUG

  /// the element type
  Type const* const type;
  /// the number of elements in the vector
  uint32_t const size;
};

/// `matNxM<T>` type
struct Matrix : public Castable<Matrix, Type> {
  /// Constructor
  /// @param ty the matrix element type
  /// @param c the number of columns in the matrix
  /// @param r the number of rows in the matrix
  Matrix(const Type* ty, uint32_t c, uint32_t r);

  /// Copy constructor
  /// @param other the other type to copy
  Matrix(const Matrix& other);

  /// @param b the ProgramBuilder used to construct the AST types
  /// @returns the constructed ast::Type node for the given type
  ast::Type* Build(ProgramBuilder& b) const override;

#ifndef NDEBUG
  /// @returns a string representation of the type, for debug purposes only
  std::string String() const override;
#endif  // NDEBUG

  /// the matrix element type
  Type const* const type;
  /// the number of columns in the matrix
  uint32_t const columns;
  /// the number of rows in the matrix
  uint32_t const rows;
};

/// `array<T, N>` type
struct Array : public Castable<Array, Type> {
  /// Constructor
  /// @param el the element type
  /// @param sz the number of elements in the array. 0 represents runtime-sized
  /// array.
  /// @param st the byte stride of the array
  Array(const Type* el, uint32_t sz, uint32_t st);

  /// Copy constructor
  /// @param other the other type to copy
  Array(const Array& other);

  /// @param b the ProgramBuilder used to construct the AST types
  /// @returns the constructed ast::Type node for the given type
  ast::Type* Build(ProgramBuilder& b) const override;

#ifndef NDEBUG
  /// @returns a string representation of the type, for debug purposes only
  std::string String() const override;
#endif  // NDEBUG

  /// the element type
  Type const* const type;
  /// the number of elements in the array. 0 represents runtime-sized array.
  uint32_t const size;
  /// the byte stride of the array
  uint32_t const stride;
};

/// `[[access]]` type
struct AccessControl : public Castable<AccessControl, Type> {
  /// Constructor
  /// @param ty the inner type
  /// @param ac the access control
  AccessControl(const Type* ty, ast::AccessControl::Access ac);

  /// Copy constructor
  /// @param other the other type to copy
  AccessControl(const AccessControl& other);

  /// @return the
  /// @param b the ProgramBuilder used to construct the AST types
  /// @returns the constructed ast::Type node for the given type
  ast::Type* Build(ProgramBuilder& b) const override;

#ifndef NDEBUG
  /// @returns a string representation of the type, for debug purposes only
  std::string String() const override;
#endif  // NDEBUG

  /// the inner type
  Type const* const type;
  /// the access control
  ast::AccessControl::Access const access;
};

/// `sampler` type
struct Sampler : public Castable<Sampler, Type> {
  /// Constructor
  /// @param k the sampler kind
  explicit Sampler(ast::SamplerKind k);

  /// Copy constructor
  /// @param other the other type to copy
  Sampler(const Sampler& other);

  /// @param b the ProgramBuilder used to construct the AST types
  /// @returns the constructed ast::Type node for the given type
  ast::Type* Build(ProgramBuilder& b) const override;

#ifndef NDEBUG
  /// @returns a string representation of the type, for debug purposes only
  std::string String() const override;
#endif  // NDEBUG

  /// the sampler kind
  ast::SamplerKind const kind;
};

/// Base class for texture types
struct Texture : public Castable<Texture, Type> {
  /// Constructor
  /// @param d the texture dimensions
  explicit Texture(ast::TextureDimension d);

  /// Copy constructor
  /// @param other the other type to copy
  Texture(const Texture& other);

  /// the texture dimensions
  ast::TextureDimension const dims;
};

/// `texture_depth_D` type
struct DepthTexture : public Castable<DepthTexture, Texture> {
  /// Constructor
  /// @param d the texture dimensions
  explicit DepthTexture(ast::TextureDimension d);

  /// Copy constructor
  /// @param other the other type to copy
  DepthTexture(const DepthTexture& other);

  /// @param b the ProgramBuilder used to construct the AST types
  /// @returns the constructed ast::Type node for the given type
  ast::Type* Build(ProgramBuilder& b) const override;

#ifndef NDEBUG
  /// @returns a string representation of the type, for debug purposes only
  std::string String() const override;
#endif  // NDEBUG
};

/// `texture_multisampled_D<T>` type
struct MultisampledTexture : public Castable<MultisampledTexture, Texture> {
  /// Constructor
  /// @param d the texture dimensions
  /// @param t the multisampled texture type
  MultisampledTexture(ast::TextureDimension d, const Type* t);

  /// Copy constructor
  /// @param other the other type to copy
  MultisampledTexture(const MultisampledTexture& other);

  /// @param b the ProgramBuilder used to construct the AST types
  /// @returns the constructed ast::Type node for the given type
  ast::Type* Build(ProgramBuilder& b) const override;

#ifndef NDEBUG
  /// @returns a string representation of the type, for debug purposes only
  std::string String() const override;
#endif  // NDEBUG

  /// the multisampled texture type
  Type const* const type;
};

/// `texture_D<T>` type
struct SampledTexture : public Castable<SampledTexture, Texture> {
  /// Constructor
  /// @param d the texture dimensions
  /// @param t the sampled texture type
  SampledTexture(ast::TextureDimension d, const Type* t);

  /// Copy constructor
  /// @param other the other type to copy
  SampledTexture(const SampledTexture& other);

  /// @param b the ProgramBuilder used to construct the AST types
  /// @returns the constructed ast::Type node for the given type
  ast::Type* Build(ProgramBuilder& b) const override;

#ifndef NDEBUG
  /// @returns a string representation of the type, for debug purposes only
  std::string String() const override;
#endif  // NDEBUG

  /// the sampled texture type
  Type const* const type;
};

/// `texture_storage_D<F>` type
struct StorageTexture : public Castable<StorageTexture, Texture> {
  /// Constructor
  /// @param d the texture dimensions
  /// @param f the storage image format
  StorageTexture(ast::TextureDimension d, ast::ImageFormat f);

  /// Copy constructor
  /// @param other the other type to copy
  StorageTexture(const StorageTexture& other);

  /// @param b the ProgramBuilder used to construct the AST types
  /// @returns the constructed ast::Type node for the given type
  ast::Type* Build(ProgramBuilder& b) const override;

#ifndef NDEBUG
  /// @returns a string representation of the type, for debug purposes only
  std::string String() const override;
#endif  // NDEBUG

  /// the storage image format
  ast::ImageFormat const format;
};

/// Base class for named types
struct Named : public Castable<Named, Type> {
  /// Constructor
  /// @param n the type name
  explicit Named(Symbol n);

  /// Copy constructor
  /// @param other the other type to copy
  Named(const Named& other);

  /// Destructor
  ~Named() override;

#ifndef NDEBUG
  /// @returns a string representation of the type, for debug purposes only
  std::string String() const override;
#endif  // NDEBUG

  /// the type name
  Symbol const name;
};

/// `type T = N` type
struct Alias : public Castable<Alias, Named> {
  /// Constructor
  /// @param n the alias name
  /// @param t the aliased type
  Alias(Symbol n, const Type* t);

  /// Copy constructor
  /// @param other the other type to copy
  Alias(const Alias& other);

  /// @param b the ProgramBuilder used to construct the AST types
  /// @returns the constructed ast::Type node for the given type
  ast::Type* Build(ProgramBuilder& b) const override;

  /// the aliased type
  Type const* const type;
};

/// `struct N { ... };` type
struct Struct : public Castable<Struct, Named> {
  /// Constructor
  /// @param n the struct name
  /// @param m the member types
  Struct(Symbol n, TypeList m);

  /// Copy constructor
  /// @param other the other type to copy
  Struct(const Struct& other);

  /// Destructor
  ~Struct() override;

  /// @param b the ProgramBuilder used to construct the AST types
  /// @returns the constructed ast::Type node for the given type
  ast::Type* Build(ProgramBuilder& b) const override;

  /// the member types
  TypeList const members;
};

/// A manager of types
class TypeManager {
 public:
  /// Constructor
  TypeManager();

  /// Destructor
  ~TypeManager();

  /// @return a Void type. Repeated calls will return the same pointer.
  const spirv::Void* Void();
  /// @return a Bool type. Repeated calls will return the same pointer.
  const spirv::Bool* Bool();
  /// @return a U32 type. Repeated calls will return the same pointer.
  const spirv::U32* U32();
  /// @return a F32 type. Repeated calls will return the same pointer.
  const spirv::F32* F32();
  /// @return a I32 type. Repeated calls will return the same pointer.
  const spirv::I32* I32();
  /// @param ty the store type
  /// @param sc the pointer storage class
  /// @return a Pointer type. Repeated calls with the same arguments will return
  /// the same pointer.
  const spirv::Pointer* Pointer(const Type* ty, ast::StorageClass sc);
  /// @param ty the referenced type
  /// @param sc the reference storage class
  /// @return a Reference type. Repeated calls with the same arguments will
  /// return the same pointer.
  const spirv::Reference* Reference(const Type* ty, ast::StorageClass sc);
  /// @param ty the element type
  /// @param sz the number of elements in the vector
  /// @return a Vector type. Repeated calls with the same arguments will return
  /// the same pointer.
  const spirv::Vector* Vector(const Type* ty, uint32_t sz);
  /// @param ty the matrix element type
  /// @param c the number of columns in the matrix
  /// @param r the number of rows in the matrix
  /// @return a Matrix type. Repeated calls with the same arguments will return
  /// the same pointer.
  const spirv::Matrix* Matrix(const Type* ty, uint32_t c, uint32_t r);
  /// @param el the element type
  /// @param sz the number of elements in the array. 0 represents runtime-sized
  /// array.
  /// @param st the byte stride of the array
  /// @return a Array type. Repeated calls with the same arguments will return
  /// the same pointer.
  const spirv::Array* Array(const Type* el, uint32_t sz, uint32_t st);
  /// @param ty the inner type
  /// @param ac the access control
  /// @return a AccessControl type. Repeated calls with the same arguments will
  /// return the same pointer.
  const spirv::AccessControl* AccessControl(const Type* ty,
                                            ast::AccessControl::Access ac);
  /// @param n the alias name
  /// @param t the aliased type
  /// @return a Alias type. Repeated calls with the same arguments will return
  /// the same pointer.
  const spirv::Alias* Alias(Symbol n, const Type* t);
  /// @param n the struct name
  /// @param m the member types
  /// @return a Struct type. Repeated calls with the same arguments will return
  /// the same pointer.
  const spirv::Struct* Struct(Symbol n, TypeList m);
  /// @param k the sampler kind
  /// @return a Sampler type. Repeated calls with the same arguments will return
  /// the same pointer.
  const spirv::Sampler* Sampler(ast::SamplerKind k);
  /// @param d the texture dimensions
  /// @return a DepthTexture type. Repeated calls with the same arguments will
  /// return the same pointer.
  const spirv::DepthTexture* DepthTexture(ast::TextureDimension d);
  /// @param d the texture dimensions
  /// @param t the multisampled texture type
  /// @return a MultisampledTexture type. Repeated calls with the same arguments
  /// will return the same pointer.
  const spirv::MultisampledTexture* MultisampledTexture(ast::TextureDimension d,
                                                        const Type* t);
  /// @param d the texture dimensions
  /// @param t the sampled texture type
  /// @return a SampledTexture type. Repeated calls with the same arguments will
  /// return the same pointer.
  const spirv::SampledTexture* SampledTexture(ast::TextureDimension d,
                                              const Type* t);
  /// @param d the texture dimensions
  /// @param f the storage image format
  /// @return a StorageTexture type. Repeated calls with the same arguments will
  /// return the same pointer.
  const spirv::StorageTexture* StorageTexture(ast::TextureDimension d,
                                              ast::ImageFormat f);

 private:
  struct State;
  std::unique_ptr<State> state;
};

}  // namespace spirv
}  // namespace reader
}  // namespace tint

#endif  // SRC_READER_SPIRV_PARSER_TYPE_H_
