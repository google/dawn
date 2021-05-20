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

#ifndef SRC_PROGRAM_BUILDER_H_
#define SRC_PROGRAM_BUILDER_H_

#include <string>
#include <utility>

#include "src/ast/alias.h"
#include "src/ast/array.h"
#include "src/ast/array_accessor_expression.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/binary_expression.h"
#include "src/ast/bool.h"
#include "src/ast/bool_literal.h"
#include "src/ast/call_expression.h"
#include "src/ast/case_statement.h"
#include "src/ast/depth_texture.h"
#include "src/ast/external_texture.h"
#include "src/ast/f32.h"
#include "src/ast/float_literal.h"
#include "src/ast/i32.h"
#include "src/ast/if_statement.h"
#include "src/ast/loop_statement.h"
#include "src/ast/matrix.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/module.h"
#include "src/ast/multisampled_texture.h"
#include "src/ast/override_decoration.h"
#include "src/ast/pointer.h"
#include "src/ast/return_statement.h"
#include "src/ast/sampled_texture.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/storage_texture.h"
#include "src/ast/stride_decoration.h"
#include "src/ast/struct_member_align_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/struct_member_size_decoration.h"
#include "src/ast/switch_statement.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/type_name.h"
#include "src/ast/u32.h"
#include "src/ast/uint_literal.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable_decl_statement.h"
#include "src/ast/vector.h"
#include "src/ast/void.h"
#include "src/ast/workgroup_decoration.h"
#include "src/program.h"
#include "src/program_id.h"
#include "src/sem/array.h"
#include "src/sem/bool_type.h"
#include "src/sem/depth_texture_type.h"
#include "src/sem/external_texture_type.h"
#include "src/sem/f32_type.h"
#include "src/sem/i32_type.h"
#include "src/sem/matrix_type.h"
#include "src/sem/multisampled_texture_type.h"
#include "src/sem/pointer_type.h"
#include "src/sem/sampled_texture_type.h"
#include "src/sem/storage_texture_type.h"
#include "src/sem/struct.h"
#include "src/sem/u32_type.h"
#include "src/sem/vector_type.h"
#include "src/sem/void_type.h"

namespace tint {

// Forward declarations
namespace ast {
class VariableDeclStatement;
}  // namespace ast

class CloneContext;

/// ProgramBuilder is a mutable builder for a Program.
/// To construct a Program, populate the builder and then `std::move` it to a
/// Program.
class ProgramBuilder {
 public:
  /// ASTNodeAllocator is an alias to BlockAllocator<ast::Node>
  using ASTNodeAllocator = BlockAllocator<ast::Node>;

  /// SemNodeAllocator is an alias to BlockAllocator<sem::Node>
  using SemNodeAllocator = BlockAllocator<sem::Node>;

  /// `i32` is a type alias to `int`.
  /// Useful for passing to template methods such as `vec2<i32>()` to imitate
  /// WGSL syntax.
  /// Note: this is intentionally not aliased to uint32_t as we want integer
  /// literals passed to the builder to match WGSL's integer literal types.
  using i32 = decltype(1);
  /// `u32` is a type alias to `unsigned int`.
  /// Useful for passing to template methods such as `vec2<u32>()` to imitate
  /// WGSL syntax.
  /// Note: this is intentionally not aliased to uint32_t as we want integer
  /// literals passed to the builder to match WGSL's integer literal types.
  using u32 = decltype(1u);
  /// `f32` is a type alias to `float`
  /// Useful for passing to template methods such as `vec2<f32>()` to imitate
  /// WGSL syntax.
  using f32 = float;

  /// Constructor
  ProgramBuilder();

  /// Move constructor
  /// @param rhs the builder to move
  ProgramBuilder(ProgramBuilder&& rhs);

  /// Destructor
  virtual ~ProgramBuilder();

  /// Move assignment operator
  /// @param rhs the builder to move
  /// @return this builder
  ProgramBuilder& operator=(ProgramBuilder&& rhs);

  /// Wrap returns a new ProgramBuilder wrapping the Program `program` without
  /// making a deep clone of the Program contents.
  /// ProgramBuilder returned by Wrap() is intended to temporarily extend an
  /// existing immutable program.
  /// As the returned ProgramBuilder wraps `program`, `program` must not be
  /// destructed or assigned while using the returned ProgramBuilder.
  /// TODO(bclayton) - Evaluate whether there are safer alternatives to this
  /// function. See crbug.com/tint/460.
  /// @param program the immutable Program to wrap
  /// @return the ProgramBuilder that wraps `program`
  static ProgramBuilder Wrap(const Program* program);

  /// @returns the unique identifier for this program
  ProgramID ID() const { return id_; }

  /// @returns a reference to the program's types
  sem::Manager& Types() {
    AssertNotMoved();
    return types_;
  }

  /// @returns a reference to the program's types
  const sem::Manager& Types() const {
    AssertNotMoved();
    return types_;
  }

  /// @returns a reference to the program's AST nodes storage
  ASTNodeAllocator& ASTNodes() {
    AssertNotMoved();
    return ast_nodes_;
  }

  /// @returns a reference to the program's AST nodes storage
  const ASTNodeAllocator& ASTNodes() const {
    AssertNotMoved();
    return ast_nodes_;
  }

  /// @returns a reference to the program's semantic nodes storage
  SemNodeAllocator& SemNodes() {
    AssertNotMoved();
    return sem_nodes_;
  }

  /// @returns a reference to the program's semantic nodes storage
  const SemNodeAllocator& SemNodes() const {
    AssertNotMoved();
    return sem_nodes_;
  }

  /// @returns a reference to the program's AST root Module
  ast::Module& AST() {
    AssertNotMoved();
    return *ast_;
  }

  /// @returns a reference to the program's AST root Module
  const ast::Module& AST() const {
    AssertNotMoved();
    return *ast_;
  }

  /// @returns a reference to the program's semantic info
  sem::Info& Sem() {
    AssertNotMoved();
    return sem_;
  }

  /// @returns a reference to the program's semantic info
  const sem::Info& Sem() const {
    AssertNotMoved();
    return sem_;
  }

  /// @returns a reference to the program's SymbolTable
  SymbolTable& Symbols() {
    AssertNotMoved();
    return symbols_;
  }

  /// @returns a reference to the program's SymbolTable
  const SymbolTable& Symbols() const {
    AssertNotMoved();
    return symbols_;
  }

  /// @returns a reference to the program's diagnostics
  diag::List& Diagnostics() {
    AssertNotMoved();
    return diagnostics_;
  }

  /// @returns a reference to the program's diagnostics
  const diag::List& Diagnostics() const {
    AssertNotMoved();
    return diagnostics_;
  }

  /// Controls whether the Resolver will be run on the program when it is built.
  /// @param enable the new flag value (defaults to true)
  void SetResolveOnBuild(bool enable) { resolve_on_build_ = enable; }

  /// @return true if the Resolver will be run on the program when it is
  /// built.
  bool ResolveOnBuild() const { return resolve_on_build_; }

  /// @returns true if the program has no error diagnostics and is not missing
  /// information
  bool IsValid() const;

  /// Writes a representation of the node to the output stream
  /// @note unlike str(), to_str() does not automatically demangle the string.
  /// @param node the AST node
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(const ast::Node* node, std::ostream& out, size_t indent) const {
    node->to_str(Sem(), out, indent);
  }

  /// Returns a demangled, string representation of `node`.
  /// @param node the AST node
  /// @returns a string representation of the node
  std::string str(const ast::Node* node) const;

  /// Creates a new ast::Node owned by the ProgramBuilder. When the
  /// ProgramBuilder is destructed, the ast::Node will also be destructed.
  /// @param source the Source of the node
  /// @param args the arguments to pass to the type constructor
  /// @returns the node pointer
  template <typename T, typename... ARGS>
  traits::EnableIfIsType<T, ast::Node>* create(const Source& source,
                                               ARGS&&... args) {
    AssertNotMoved();
    return ast_nodes_.Create<T>(id_, source, std::forward<ARGS>(args)...);
  }

  /// Creates a new ast::Node owned by the ProgramBuilder, injecting the current
  /// Source as set by the last call to SetSource() as the only argument to the
  /// constructor.
  /// When the ProgramBuilder is destructed, the ast::Node will also be
  /// destructed.
  /// @returns the node pointer
  template <typename T>
  traits::EnableIfIsType<T, ast::Node>* create() {
    AssertNotMoved();
    return ast_nodes_.Create<T>(id_, source_);
  }

  /// Creates a new ast::Node owned by the ProgramBuilder, injecting the current
  /// Source as set by the last call to SetSource() as the first argument to the
  /// constructor.
  /// When the ProgramBuilder is destructed, the ast::Node will also be
  /// destructed.
  /// @param arg0 the first arguments to pass to the type constructor
  /// @param args the remaining arguments to pass to the type constructor
  /// @returns the node pointer
  template <typename T, typename ARG0, typename... ARGS>
  traits::EnableIf</* T is ast::Node and ARG0 is not Source */
                   traits::IsTypeOrDerived<T, ast::Node>::value &&
                       !traits::IsTypeOrDerived<ARG0, Source>::value,
                   T>*
  create(ARG0&& arg0, ARGS&&... args) {
    AssertNotMoved();
    return ast_nodes_.Create<T>(id_, source_, std::forward<ARG0>(arg0),
                                std::forward<ARGS>(args)...);
  }

  /// Creates a new sem::Node owned by the ProgramBuilder.
  /// When the ProgramBuilder is destructed, the sem::Node will also be
  /// destructed.
  /// @param args the arguments to pass to the type constructor
  /// @returns the node pointer
  template <typename T, typename... ARGS>
  traits::EnableIf<traits::IsTypeOrDerived<T, sem::Node>::value &&
                       !traits::IsTypeOrDerived<T, sem::Type>::value,
                   T>*
  create(ARGS&&... args) {
    AssertNotMoved();
    return sem_nodes_.Create<T>(std::forward<ARGS>(args)...);
  }

  /// Creates a new sem::Type owned by the ProgramBuilder.
  /// When the ProgramBuilder is destructed, owned ProgramBuilder and the
  /// returned`Type` will also be destructed.
  /// Types are unique (de-aliased), and so calling create() for the same `T`
  /// and arguments will return the same pointer.
  /// @warning Use this method to acquire a type only if all of its type
  /// information is provided in the constructor arguments `args`.<br>
  /// If the type requires additional configuration after construction that
  /// affect its fundamental type, build the type with `std::make_unique`, make
  /// any necessary alterations and then call unique_type() instead.
  /// @param args the arguments to pass to the type constructor
  /// @returns the de-aliased type pointer
  template <typename T, typename... ARGS>
  traits::EnableIfIsType<T, sem::Type>* create(ARGS&&... args) {
    static_assert(std::is_base_of<sem::Type, T>::value,
                  "T does not derive from sem::Type");
    AssertNotMoved();
    return types_.Get<T>(std::forward<ARGS>(args)...);
  }

  /// Marks this builder as moved, preventing any further use of the builder.
  void MarkAsMoved();

  //////////////////////////////////////////////////////////////////////////////
  // TypesBuilder
  //////////////////////////////////////////////////////////////////////////////

  /// TypesBuilder holds basic `tint` types and methods for constructing
  /// complex types.
  class TypesBuilder {
   public:
    /// Constructor
    /// @param builder the program builder
    explicit TypesBuilder(ProgramBuilder* builder);

    /// @return the tint AST type for the C type `T`.
    template <typename T>
    ast::Type* Of() const {
      return CToAST<T>::get(this);
    }

    /// @returns a boolean type
    ast::Bool* bool_() const { return builder->create<ast::Bool>(); }

    /// @param source the Source of the node
    /// @returns a boolean type
    ast::Bool* bool_(const Source& source) const {
      return builder->create<ast::Bool>(source);
    }

    /// @returns a f32 type
    ast::F32* f32() const { return builder->create<ast::F32>(); }

    /// @param source the Source of the node
    /// @returns a f32 type
    ast::F32* f32(const Source& source) const {
      return builder->create<ast::F32>(source);
    }

    /// @returns a i32 type
    ast::I32* i32() const { return builder->create<ast::I32>(); }

    /// @param source the Source of the node
    /// @returns a i32 type
    ast::I32* i32(const Source& source) const {
      return builder->create<ast::I32>(source);
    }

    /// @returns a u32 type
    ast::U32* u32() const { return builder->create<ast::U32>(); }

    /// @param source the Source of the node
    /// @returns a u32 type
    ast::U32* u32(const Source& source) const {
      return builder->create<ast::U32>(source);
    }

    /// @returns a void type
    ast::Void* void_() const { return builder->create<ast::Void>(); }

    /// @param source the Source of the node
    /// @returns a void type
    ast::Void* void_(const Source& source) const {
      return builder->create<ast::Void>(source);
    }

    /// @param type vector subtype
    /// @param n vector width in elements
    /// @return the tint AST type for a `n`-element vector of `type`.
    ast::Vector* vec(ast::Type* type, uint32_t n) const {
      type = MaybeCreateTypename(type);
      return builder->create<ast::Vector>(type, n);
    }

    /// @param source the Source of the node
    /// @param type vector subtype
    /// @param n vector width in elements
    /// @return the tint AST type for a `n`-element vector of `type`.
    ast::Vector* vec(const Source& source, ast::Type* type, uint32_t n) const {
      type = MaybeCreateTypename(type);
      return builder->create<ast::Vector>(source, type, n);
    }

    /// @param type vector subtype
    /// @return the tint AST type for a 2-element vector of `type`.
    ast::Vector* vec2(ast::Type* type) const { return vec(type, 2u); }

    /// @param type vector subtype
    /// @return the tint AST type for a 3-element vector of `type`.
    ast::Vector* vec3(ast::Type* type) const { return vec(type, 3u); }

    /// @param type vector subtype
    /// @return the tint AST type for a 4-element vector of `type`.
    ast::Vector* vec4(ast::Type* type) const { return vec(type, 4u); }

    /// @param n vector width in elements
    /// @return the tint AST type for a `n`-element vector of `type`.
    template <typename T>
    ast::Vector* vec(uint32_t n) const {
      return vec(Of<T>(), n);
    }

    /// @return the tint AST type for a 2-element vector of the C type `T`.
    template <typename T>
    ast::Vector* vec2() const {
      return vec2(Of<T>());
    }

    /// @return the tint AST type for a 3-element vector of the C type `T`.
    template <typename T>
    ast::Vector* vec3() const {
      return vec3(Of<T>());
    }

    /// @return the tint AST type for a 4-element vector of the C type `T`.
    template <typename T>
    ast::Vector* vec4() const {
      return vec4(Of<T>());
    }

    /// @param type matrix subtype
    /// @param columns number of columns for the matrix
    /// @param rows number of rows for the matrix
    /// @return the tint AST type for a matrix of `type`
    ast::Matrix* mat(ast::Type* type, uint32_t columns, uint32_t rows) const {
      type = MaybeCreateTypename(type);
      return builder->create<ast::Matrix>(type, rows, columns);
    }

    /// @param source the Source of the node
    /// @param type matrix subtype
    /// @param columns number of columns for the matrix
    /// @param rows number of rows for the matrix
    /// @return the tint AST type for a matrix of `type`
    ast::Matrix* mat(const Source& source,
                     ast::Type* type,
                     uint32_t columns,
                     uint32_t rows) const {
      return builder->create<ast::Matrix>(source, type, rows, columns);
    }

    /// @param type matrix subtype
    /// @return the tint AST type for a 2x3 matrix of `type`.
    ast::Matrix* mat2x2(ast::Type* type) const { return mat(type, 2u, 2u); }

    /// @param type matrix subtype
    /// @return the tint AST type for a 2x3 matrix of `type`.
    ast::Matrix* mat2x3(ast::Type* type) const { return mat(type, 2u, 3u); }

    /// @param type matrix subtype
    /// @return the tint AST type for a 2x4 matrix of `type`.
    ast::Matrix* mat2x4(ast::Type* type) const { return mat(type, 2u, 4u); }

    /// @param type matrix subtype
    /// @return the tint AST type for a 3x2 matrix of `type`.
    ast::Matrix* mat3x2(ast::Type* type) const { return mat(type, 3u, 2u); }

    /// @param type matrix subtype
    /// @return the tint AST type for a 3x3 matrix of `type`.
    ast::Matrix* mat3x3(ast::Type* type) const { return mat(type, 3u, 3u); }

    /// @param type matrix subtype
    /// @return the tint AST type for a 3x4 matrix of `type`.
    ast::Matrix* mat3x4(ast::Type* type) const { return mat(type, 3u, 4u); }

    /// @param type matrix subtype
    /// @return the tint AST type for a 4x2 matrix of `type`.
    ast::Matrix* mat4x2(ast::Type* type) const { return mat(type, 4u, 2u); }

    /// @param type matrix subtype
    /// @return the tint AST type for a 4x3 matrix of `type`.
    ast::Matrix* mat4x3(ast::Type* type) const { return mat(type, 4u, 3u); }

    /// @param type matrix subtype
    /// @return the tint AST type for a 4x4 matrix of `type`.
    ast::Matrix* mat4x4(ast::Type* type) const { return mat(type, 4u, 4u); }

    /// @param columns number of columns for the matrix
    /// @param rows number of rows for the matrix
    /// @return the tint AST type for a matrix of `type`
    template <typename T>
    ast::Matrix* mat(uint32_t columns, uint32_t rows) const {
      return mat(Of<T>(), columns, rows);
    }

    /// @return the tint AST type for a 2x3 matrix of the C type `T`.
    template <typename T>
    ast::Matrix* mat2x2() const {
      return mat2x2(Of<T>());
    }

    /// @return the tint AST type for a 2x3 matrix of the C type `T`.
    template <typename T>
    ast::Matrix* mat2x3() const {
      return mat2x3(Of<T>());
    }

    /// @return the tint AST type for a 2x4 matrix of the C type `T`.
    template <typename T>
    ast::Matrix* mat2x4() const {
      return mat2x4(Of<T>());
    }

    /// @return the tint AST type for a 3x2 matrix of the C type `T`.
    template <typename T>
    ast::Matrix* mat3x2() const {
      return mat3x2(Of<T>());
    }

    /// @return the tint AST type for a 3x3 matrix of the C type `T`.
    template <typename T>
    ast::Matrix* mat3x3() const {
      return mat3x3(Of<T>());
    }

    /// @return the tint AST type for a 3x4 matrix of the C type `T`.
    template <typename T>
    ast::Matrix* mat3x4() const {
      return mat3x4(Of<T>());
    }

    /// @return the tint AST type for a 4x2 matrix of the C type `T`.
    template <typename T>
    ast::Matrix* mat4x2() const {
      return mat4x2(Of<T>());
    }

    /// @return the tint AST type for a 4x3 matrix of the C type `T`.
    template <typename T>
    ast::Matrix* mat4x3() const {
      return mat4x3(Of<T>());
    }

    /// @return the tint AST type for a 4x4 matrix of the C type `T`.
    template <typename T>
    ast::Matrix* mat4x4() const {
      return mat4x4(Of<T>());
    }

    /// @param subtype the array element type
    /// @param n the array size. 0 represents a runtime-array
    /// @param decos the optional decorations for the array
    /// @return the tint AST type for a array of size `n` of type `T`
    ast::Array* array(ast::Type* subtype,
                      uint32_t n = 0,
                      ast::DecorationList decos = {}) const {
      subtype = MaybeCreateTypename(subtype);
      return builder->create<ast::Array>(subtype, n, decos);
    }

    /// @param source the Source of the node
    /// @param subtype the array element type
    /// @param n the array size. 0 represents a runtime-array
    /// @param decos the optional decorations for the array
    /// @return the tint AST type for a array of size `n` of type `T`
    ast::Array* array(const Source& source,
                      ast::Type* subtype,
                      uint32_t n = 0,
                      ast::DecorationList decos = {}) const {
      subtype = MaybeCreateTypename(subtype);
      return builder->create<ast::Array>(source, subtype, n, decos);
    }

    /// @param subtype the array element type
    /// @param n the array size. 0 represents a runtime-array
    /// @param stride the array stride. 0 represents implicit stride
    /// @return the tint AST type for a array of size `n` of type `T`
    ast::Array* array(ast::Type* subtype, uint32_t n, uint32_t stride) const {
      subtype = MaybeCreateTypename(subtype);
      ast::DecorationList decos;
      if (stride) {
        decos.emplace_back(builder->create<ast::StrideDecoration>(stride));
      }
      return array(subtype, n, std::move(decos));
    }

    /// @param source the Source of the node
    /// @param subtype the array element type
    /// @param n the array size. 0 represents a runtime-array
    /// @param stride the array stride. 0 represents implicit stride
    /// @return the tint AST type for a array of size `n` of type `T`
    ast::Array* array(const Source& source,
                      ast::Type* subtype,
                      uint32_t n,
                      uint32_t stride) const {
      subtype = MaybeCreateTypename(subtype);
      ast::DecorationList decos;
      if (stride) {
        decos.emplace_back(builder->create<ast::StrideDecoration>(stride));
      }
      return array(source, subtype, n, std::move(decos));
    }

    /// @return the tint AST type for an array of size `N` of type `T`
    template <typename T, int N = 0>
    ast::Array* array() const {
      return array(Of<T>(), N);
    }

    /// @param stride the array stride
    /// @return the tint AST type for an array of size `N` of type `T`
    template <typename T, int N = 0>
    ast::Array* array(uint32_t stride) const {
      return array(Of<T>(), N, stride);
    }

    /// Creates a type name
    /// @param name the name
    /// @returns the type name
    template <typename NAME>
    ast::TypeName* type_name(NAME&& name) const {
      return builder->create<ast::TypeName>(
          builder->Sym(std::forward<NAME>(name)));
    }

    /// Creates a type name
    /// @param source the Source of the node
    /// @param name the name
    /// @returns the type name
    template <typename NAME>
    ast::TypeName* type_name(const Source& source, NAME&& name) const {
      return builder->create<ast::TypeName>(
          source, builder->Sym(std::forward<NAME>(name)));
    }

    /// Creates an alias type
    /// @param name the alias name
    /// @param type the alias type
    /// @returns the alias pointer
    template <typename NAME>
    ast::Alias* alias(NAME&& name, ast::Type* type) const {
      type = MaybeCreateTypename(type);
      auto sym = builder->Sym(std::forward<NAME>(name));
      return builder->create<ast::Alias>(sym, type);
    }

    /// Creates an alias type
    /// @param source the Source of the node
    /// @param name the alias name
    /// @param type the alias type
    /// @returns the alias pointer
    template <typename NAME>
    ast::Alias* alias(const Source& source,
                      NAME&& name,
                      ast::Type* type) const {
      type = MaybeCreateTypename(type);
      auto sym = builder->Sym(std::forward<NAME>(name));
      return builder->create<ast::Alias>(source, sym, type);
    }

    /// Creates an access control qualifier type
    /// @param access the access control
    /// @param type the inner type
    /// @returns the access control qualifier type
    ast::AccessControl* access(ast::AccessControl::Access access,
                               const ast::Type* type) const {
      type = MaybeCreateTypename(type);
      return type ? builder->create<ast::AccessControl>(access, type) : nullptr;
    }

    /// Creates an access control qualifier type
    /// @param source the Source of the node
    /// @param access the access control
    /// @param type the inner type
    /// @returns the access control qualifier type
    ast::AccessControl* access(const Source& source,
                               ast::AccessControl::Access access,
                               const ast::Type* type) const {
      type = MaybeCreateTypename(type);
      return type ? builder->create<ast::AccessControl>(source, access, type)
                  : nullptr;
    }

    /// @param type the type of the pointer
    /// @param storage_class the storage class of the pointer
    /// @return the pointer to `type` with the given ast::StorageClass
    ast::Pointer* pointer(ast::Type* type,
                          ast::StorageClass storage_class) const {
      type = MaybeCreateTypename(type);
      return builder->create<ast::Pointer>(type, storage_class);
    }

    /// @param source the Source of the node
    /// @param type the type of the pointer
    /// @param storage_class the storage class of the pointer
    /// @return the pointer to `type` with the given ast::StorageClass
    ast::Pointer* pointer(const Source& source,
                          ast::Type* type,
                          ast::StorageClass storage_class) const {
      type = MaybeCreateTypename(type);
      return builder->create<ast::Pointer>(source, type, storage_class);
    }

    /// @param storage_class the storage class of the pointer
    /// @return the pointer to type `T` with the given ast::StorageClass.
    template <typename T>
    ast::Pointer* pointer(ast::StorageClass storage_class) const {
      return pointer(Of<T>(), storage_class);
    }

    /// @param kind the kind of sampler
    /// @returns the sampler
    ast::Sampler* sampler(ast::SamplerKind kind) const {
      return builder->create<ast::Sampler>(kind);
    }

    /// @param source the Source of the node
    /// @param kind the kind of sampler
    /// @returns the sampler
    ast::Sampler* sampler(const Source& source, ast::SamplerKind kind) const {
      return builder->create<ast::Sampler>(source, kind);
    }

    /// @param dims the dimensionality of the texture
    /// @returns the depth texture
    ast::DepthTexture* depth_texture(ast::TextureDimension dims) const {
      return builder->create<ast::DepthTexture>(dims);
    }

    /// @param source the Source of the node
    /// @param dims the dimensionality of the texture
    /// @returns the depth texture
    ast::DepthTexture* depth_texture(const Source& source,
                                     ast::TextureDimension dims) const {
      return builder->create<ast::DepthTexture>(source, dims);
    }

    /// @param dims the dimensionality of the texture
    /// @param subtype the texture subtype.
    /// @returns the sampled texture
    ast::SampledTexture* sampled_texture(ast::TextureDimension dims,
                                         ast::Type* subtype) const {
      return builder->create<ast::SampledTexture>(dims, subtype);
    }

    /// @param source the Source of the node
    /// @param dims the dimensionality of the texture
    /// @param subtype the texture subtype.
    /// @returns the sampled texture
    ast::SampledTexture* sampled_texture(const Source& source,
                                         ast::TextureDimension dims,
                                         ast::Type* subtype) const {
      return builder->create<ast::SampledTexture>(source, dims, subtype);
    }

    /// @param dims the dimensionality of the texture
    /// @param subtype the texture subtype.
    /// @returns the multisampled texture
    ast::MultisampledTexture* multisampled_texture(ast::TextureDimension dims,
                                                   ast::Type* subtype) const {
      return builder->create<ast::MultisampledTexture>(dims, subtype);
    }

    /// @param source the Source of the node
    /// @param dims the dimensionality of the texture
    /// @param subtype the texture subtype.
    /// @returns the multisampled texture
    ast::MultisampledTexture* multisampled_texture(const Source& source,
                                                   ast::TextureDimension dims,
                                                   ast::Type* subtype) const {
      return builder->create<ast::MultisampledTexture>(source, dims, subtype);
    }

    /// @param dims the dimensionality of the texture
    /// @param format the image format of the texture
    /// @returns the storage texture
    ast::StorageTexture* storage_texture(ast::TextureDimension dims,
                                         ast::ImageFormat format) const {
      auto* subtype = ast::StorageTexture::SubtypeFor(format, *builder);
      return builder->create<ast::StorageTexture>(dims, format, subtype);
    }

    /// @param source the Source of the node
    /// @param dims the dimensionality of the texture
    /// @param format the image format of the texture
    /// @returns the storage texture
    ast::StorageTexture* storage_texture(const Source& source,
                                         ast::TextureDimension dims,
                                         ast::ImageFormat format) const {
      auto* subtype = ast::StorageTexture::SubtypeFor(format, *builder);
      return builder->create<ast::StorageTexture>(source, dims, format,
                                                  subtype);
    }

    /// @returns the external texture
    ast::ExternalTexture* external_texture() const {
      return builder->create<ast::ExternalTexture>();
    }

    /// @param source the Source of the node
    /// @returns the external texture
    ast::ExternalTexture* external_texture(const Source& source) const {
      return builder->create<ast::ExternalTexture>(source);
    }

    /// [DEPRECATED]: TODO(crbug.com/tint/745): Migrate to const AST pointers.
    /// If ty is a ast::Struct or ast::Alias, the returned type is an
    /// ast::TypeName of the given type's name, otherwise  type is returned.
    /// @param type the type
    /// @return either type or a pointer to a new ast::TypeName
    ast::Type* MaybeCreateTypename(ast::Type* type) const;

    /// If ty is a ast::Struct or ast::Alias, the returned type is an
    /// ast::TypeName of the given type's name, otherwise  type is returned.
    /// @param type the type
    /// @return either type or a pointer to a new ast::TypeName
    const ast::Type* MaybeCreateTypename(const ast::Type* type) const;

    /// The ProgramBuilder
    ProgramBuilder* const builder;

   private:
    /// CToAST<T> is specialized for various `T` types and each specialization
    /// contains a single static `get()` method for obtaining the corresponding
    /// AST type for the C type `T`.
    /// `get()` has the signature:
    ///    `static ast::Type* get(Types* t)`
    template <typename T>
    struct CToAST {};
  };

  //////////////////////////////////////////////////////////////////////////////
  // AST helper methods
  //////////////////////////////////////////////////////////////////////////////

  /// @return a new unnamed symbol
  Symbol Sym() { return Symbols().New(); }

  /// @param name the symbol string
  /// @return a Symbol with the given name
  Symbol Sym(const std::string& name) { return Symbols().Register(name); }

  /// @param sym the symbol
  /// @return `sym`
  Symbol Sym(Symbol sym) { return sym; }

  /// @param expr the expression
  /// @return expr
  template <typename T>
  traits::EnableIfIsType<T, ast::Expression>* Expr(T* expr) {
    return expr;
  }

  /// Passthrough for nullptr
  /// @return nullptr
  ast::IdentifierExpression* Expr(std::nullptr_t) { return nullptr; }

  /// @param source the source information
  /// @param symbol the identifier symbol
  /// @return an ast::IdentifierExpression with the given symbol
  ast::IdentifierExpression* Expr(const Source& source, Symbol symbol) {
    return create<ast::IdentifierExpression>(source, symbol);
  }

  /// @param symbol the identifier symbol
  /// @return an ast::IdentifierExpression with the given symbol
  ast::IdentifierExpression* Expr(Symbol symbol) {
    return create<ast::IdentifierExpression>(symbol);
  }

  /// @param source the source information
  /// @param variable the AST variable
  /// @return an ast::IdentifierExpression with the variable's symbol
  ast::IdentifierExpression* Expr(const Source& source,
                                  ast::Variable* variable) {
    return create<ast::IdentifierExpression>(source, variable->symbol());
  }

  /// @param variable the AST variable
  /// @return an ast::IdentifierExpression with the variable's symbol
  ast::IdentifierExpression* Expr(ast::Variable* variable) {
    return create<ast::IdentifierExpression>(variable->symbol());
  }

  /// @param source the source information
  /// @param name the identifier name
  /// @return an ast::IdentifierExpression with the given name
  ast::IdentifierExpression* Expr(const Source& source, const char* name) {
    return create<ast::IdentifierExpression>(source, Symbols().Register(name));
  }

  /// @param name the identifier name
  /// @return an ast::IdentifierExpression with the given name
  ast::IdentifierExpression* Expr(const char* name) {
    return create<ast::IdentifierExpression>(Symbols().Register(name));
  }

  /// @param source the source information
  /// @param name the identifier name
  /// @return an ast::IdentifierExpression with the given name
  ast::IdentifierExpression* Expr(const Source& source,
                                  const std::string& name) {
    return create<ast::IdentifierExpression>(source, Symbols().Register(name));
  }

  /// @param name the identifier name
  /// @return an ast::IdentifierExpression with the given name
  ast::IdentifierExpression* Expr(const std::string& name) {
    return create<ast::IdentifierExpression>(Symbols().Register(name));
  }

  /// @param source the source information
  /// @param value the boolean value
  /// @return a Scalar constructor for the given value
  ast::ScalarConstructorExpression* Expr(const Source& source, bool value) {
    return create<ast::ScalarConstructorExpression>(source, Literal(value));
  }

  /// @param value the boolean value
  /// @return a Scalar constructor for the given value
  ast::ScalarConstructorExpression* Expr(bool value) {
    return create<ast::ScalarConstructorExpression>(Literal(value));
  }

  /// @param source the source information
  /// @param value the float value
  /// @return a Scalar constructor for the given value
  ast::ScalarConstructorExpression* Expr(const Source& source, f32 value) {
    return create<ast::ScalarConstructorExpression>(source, Literal(value));
  }

  /// @param value the float value
  /// @return a Scalar constructor for the given value
  ast::ScalarConstructorExpression* Expr(f32 value) {
    return create<ast::ScalarConstructorExpression>(Literal(value));
  }

  /// @param source the source information
  /// @param value the integer value
  /// @return a Scalar constructor for the given value
  ast::ScalarConstructorExpression* Expr(const Source& source, i32 value) {
    return create<ast::ScalarConstructorExpression>(source, Literal(value));
  }

  /// @param value the integer value
  /// @return a Scalar constructor for the given value
  ast::ScalarConstructorExpression* Expr(i32 value) {
    return create<ast::ScalarConstructorExpression>(Literal(value));
  }

  /// @param source the source information
  /// @param value the unsigned int value
  /// @return a Scalar constructor for the given value
  ast::ScalarConstructorExpression* Expr(const Source& source, u32 value) {
    return create<ast::ScalarConstructorExpression>(source, Literal(value));
  }

  /// @param value the unsigned int value
  /// @return a Scalar constructor for the given value
  ast::ScalarConstructorExpression* Expr(u32 value) {
    return create<ast::ScalarConstructorExpression>(Literal(value));
  }

  /// Converts `arg` to an `ast::Expression` using `Expr()`, then appends it to
  /// `list`.
  /// @param list the list to append too
  /// @param arg the arg to create
  template <typename ARG>
  void Append(ast::ExpressionList& list, ARG&& arg) {
    list.emplace_back(Expr(std::forward<ARG>(arg)));
  }

  /// Converts `arg0` and `args` to `ast::Expression`s using `Expr()`,
  /// then appends them to `list`.
  /// @param list the list to append too
  /// @param arg0 the first argument
  /// @param args the rest of the arguments
  template <typename ARG0, typename... ARGS>
  void Append(ast::ExpressionList& list, ARG0&& arg0, ARGS&&... args) {
    Append(list, std::forward<ARG0>(arg0));
    Append(list, std::forward<ARGS>(args)...);
  }

  /// @return an empty list of expressions
  ast::ExpressionList ExprList() { return {}; }

  /// @param args the list of expressions
  /// @return the list of expressions converted to `ast::Expression`s using
  /// `Expr()`,
  template <typename... ARGS>
  ast::ExpressionList ExprList(ARGS&&... args) {
    ast::ExpressionList list;
    list.reserve(sizeof...(args));
    Append(list, std::forward<ARGS>(args)...);
    return list;
  }

  /// @param list the list of expressions
  /// @return `list`
  ast::ExpressionList ExprList(ast::ExpressionList list) { return list; }

  /// @param val the boolan value
  /// @return a boolean literal with the given value
  ast::BoolLiteral* Literal(bool val) { return create<ast::BoolLiteral>(val); }

  /// @param val the float value
  /// @return a float literal with the given value
  ast::FloatLiteral* Literal(f32 val) { return create<ast::FloatLiteral>(val); }

  /// @param val the unsigned int value
  /// @return a ast::UintLiteral with the given value
  ast::UintLiteral* Literal(u32 val) { return create<ast::UintLiteral>(val); }

  /// @param val the integer value
  /// @return the ast::SintLiteral with the given value
  ast::SintLiteral* Literal(i32 val) { return create<ast::SintLiteral>(val); }

  /// @param args the arguments for the type constructor
  /// @return an `ast::TypeConstructorExpression` of type `ty`, with the values
  /// of `args` converted to `ast::Expression`s using `Expr()`
  template <typename T, typename... ARGS>
  ast::TypeConstructorExpression* Construct(ARGS&&... args) {
    return Construct(ty.Of<T>(), std::forward<ARGS>(args)...);
  }

  /// @param type the type to construct
  /// @param args the arguments for the constructor
  /// @return an `ast::TypeConstructorExpression` of `type` constructed with the
  /// values `args`.
  template <typename... ARGS>
  ast::TypeConstructorExpression* Construct(ast::Type* type, ARGS&&... args) {
    type = ty.MaybeCreateTypename(type);
    return create<ast::TypeConstructorExpression>(
        type, ExprList(std::forward<ARGS>(args)...));
  }

  /// Creates a constructor expression that constructs an object of
  /// `type` filled with `elem_value`. For example,
  /// ConstructValueFilledWith(ty.mat3x4<float>(), 5) returns a
  /// TypeConstructorExpression for a Mat3x4 filled with 5.0f values.
  /// @param type the type to construct
  /// @param elem_value the initial or element value (for vec and mat) to
  /// construct with
  /// @return the constructor expression
  ast::ConstructorExpression* ConstructValueFilledWith(const ast::Type* type,
                                                       int elem_value = 0);

  /// @param args the arguments for the vector constructor
  /// @return an `ast::TypeConstructorExpression` of a 2-element vector of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  ast::TypeConstructorExpression* vec2(ARGS&&... args) {
    return Construct(ty.vec2<T>(), std::forward<ARGS>(args)...);
  }

  /// @param args the arguments for the vector constructor
  /// @return an `ast::TypeConstructorExpression` of a 3-element vector of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  ast::TypeConstructorExpression* vec3(ARGS&&... args) {
    return Construct(ty.vec3<T>(), std::forward<ARGS>(args)...);
  }

  /// @param args the arguments for the vector constructor
  /// @return an `ast::TypeConstructorExpression` of a 4-element vector of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  ast::TypeConstructorExpression* vec4(ARGS&&... args) {
    return Construct(ty.vec4<T>(), std::forward<ARGS>(args)...);
  }

  /// @param args the arguments for the matrix constructor
  /// @return an `ast::TypeConstructorExpression` of a 2x2 matrix of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  ast::TypeConstructorExpression* mat2x2(ARGS&&... args) {
    return Construct(ty.mat2x2<T>(), std::forward<ARGS>(args)...);
  }

  /// @param args the arguments for the matrix constructor
  /// @return an `ast::TypeConstructorExpression` of a 2x3 matrix of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  ast::TypeConstructorExpression* mat2x3(ARGS&&... args) {
    return Construct(ty.mat2x3<T>(), std::forward<ARGS>(args)...);
  }

  /// @param args the arguments for the matrix constructor
  /// @return an `ast::TypeConstructorExpression` of a 2x4 matrix of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  ast::TypeConstructorExpression* mat2x4(ARGS&&... args) {
    return Construct(ty.mat2x4<T>(), std::forward<ARGS>(args)...);
  }

  /// @param args the arguments for the matrix constructor
  /// @return an `ast::TypeConstructorExpression` of a 3x2 matrix of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  ast::TypeConstructorExpression* mat3x2(ARGS&&... args) {
    return Construct(ty.mat3x2<T>(), std::forward<ARGS>(args)...);
  }

  /// @param args the arguments for the matrix constructor
  /// @return an `ast::TypeConstructorExpression` of a 3x3 matrix of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  ast::TypeConstructorExpression* mat3x3(ARGS&&... args) {
    return Construct(ty.mat3x3<T>(), std::forward<ARGS>(args)...);
  }

  /// @param args the arguments for the matrix constructor
  /// @return an `ast::TypeConstructorExpression` of a 3x4 matrix of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  ast::TypeConstructorExpression* mat3x4(ARGS&&... args) {
    return Construct(ty.mat3x4<T>(), std::forward<ARGS>(args)...);
  }

  /// @param args the arguments for the matrix constructor
  /// @return an `ast::TypeConstructorExpression` of a 4x2 matrix of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  ast::TypeConstructorExpression* mat4x2(ARGS&&... args) {
    return Construct(ty.mat4x2<T>(), std::forward<ARGS>(args)...);
  }

  /// @param args the arguments for the matrix constructor
  /// @return an `ast::TypeConstructorExpression` of a 4x3 matrix of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  ast::TypeConstructorExpression* mat4x3(ARGS&&... args) {
    return Construct(ty.mat4x3<T>(), std::forward<ARGS>(args)...);
  }

  /// @param args the arguments for the matrix constructor
  /// @return an `ast::TypeConstructorExpression` of a 4x4 matrix of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  ast::TypeConstructorExpression* mat4x4(ARGS&&... args) {
    return Construct(ty.mat4x4<T>(), std::forward<ARGS>(args)...);
  }

  /// @param args the arguments for the array constructor
  /// @return an `ast::TypeConstructorExpression` of an array with element type
  /// `T`, constructed with the values `args`.
  template <typename T, int N = 0, typename... ARGS>
  ast::TypeConstructorExpression* array(ARGS&&... args) {
    return Construct(ty.array<T, N>(), std::forward<ARGS>(args)...);
  }

  /// @param subtype the array element type
  /// @param n the array size. 0 represents a runtime-array.
  /// @param args the arguments for the array constructor
  /// @return an `ast::TypeConstructorExpression` of an array with element type
  /// `subtype`, constructed with the values `args`.
  template <typename... ARGS>
  ast::TypeConstructorExpression* array(ast::Type* subtype,
                                        uint32_t n,
                                        ARGS&&... args) {
    return Construct(ty.array(subtype, n), std::forward<ARGS>(args)...);
  }

  /// @param name the variable name
  /// @param type the variable type
  /// @param storage the variable storage class
  /// @param constructor constructor expression
  /// @param decorations variable decorations
  /// @returns a `ast::Variable` with the given name, storage and type
  template <typename NAME>
  ast::Variable* Var(NAME&& name,
                     const ast::Type* type,
                     ast::StorageClass storage = ast::StorageClass::kNone,
                     ast::Expression* constructor = nullptr,
                     ast::DecorationList decorations = {}) {
    type = ty.MaybeCreateTypename(type);
    return create<ast::Variable>(Sym(std::forward<NAME>(name)), storage, type,
                                 false, constructor, decorations);
  }

  /// @param source the variable source
  /// @param name the variable name
  /// @param type the variable type
  /// @param storage the variable storage class
  /// @param constructor constructor expression
  /// @param decorations variable decorations
  /// @returns a `ast::Variable` with the given name, storage and type
  template <typename NAME>
  ast::Variable* Var(const Source& source,
                     NAME&& name,
                     const ast::Type* type,
                     ast::StorageClass storage = ast::StorageClass::kNone,
                     ast::Expression* constructor = nullptr,
                     ast::DecorationList decorations = {}) {
    type = ty.MaybeCreateTypename(type);
    return create<ast::Variable>(source, Sym(std::forward<NAME>(name)), storage,
                                 type, false, constructor, decorations);
  }

  /// @param name the variable name
  /// @param type the variable type
  /// @param constructor constructor expression
  /// @param decorations optional variable decorations
  /// @returns a constant `ast::Variable` with the given name and type
  template <typename NAME>
  ast::Variable* Const(NAME&& name,
                       ast::Type* type,
                       ast::Expression* constructor,
                       ast::DecorationList decorations = {}) {
    type = ty.MaybeCreateTypename(type);
    return create<ast::Variable>(Sym(std::forward<NAME>(name)),
                                 ast::StorageClass::kNone, type, true,
                                 constructor, decorations);
  }

  /// @param source the variable source
  /// @param name the variable name
  /// @param type the variable type
  /// @param constructor constructor expression
  /// @param decorations optional variable decorations
  /// @returns a constant `ast::Variable` with the given name and type
  template <typename NAME>
  ast::Variable* Const(const Source& source,
                       NAME&& name,
                       ast::Type* type,
                       ast::Expression* constructor,
                       ast::DecorationList decorations = {}) {
    type = ty.MaybeCreateTypename(type);
    return create<ast::Variable>(source, Sym(std::forward<NAME>(name)),
                                 ast::StorageClass::kNone, type, true,
                                 constructor, decorations);
  }

  /// @param name the parameter name
  /// @param type the parameter type
  /// @param decorations optional parameter decorations
  /// @returns a constant `ast::Variable` with the given name and type
  template <typename NAME>
  ast::Variable* Param(NAME&& name,
                       ast::Type* type,
                       ast::DecorationList decorations = {}) {
    type = ty.MaybeCreateTypename(type);
    return create<ast::Variable>(Sym(std::forward<NAME>(name)),
                                 ast::StorageClass::kNone, type, true, nullptr,
                                 decorations);
  }

  /// @param source the parameter source
  /// @param name the parameter name
  /// @param type the parameter type
  /// @param decorations optional parameter decorations
  /// @returns a constant `ast::Variable` with the given name and type
  template <typename NAME>
  ast::Variable* Param(const Source& source,
                       NAME&& name,
                       ast::Type* type,
                       ast::DecorationList decorations = {}) {
    type = ty.MaybeCreateTypename(type);
    return create<ast::Variable>(source, Sym(std::forward<NAME>(name)),
                                 ast::StorageClass::kNone, type, true, nullptr,
                                 decorations);
  }

  /// @param name the variable name
  /// @param type the variable type
  /// @param storage the variable storage class
  /// @param constructor constructor expression
  /// @param decorations variable decorations
  /// @returns a new `ast::Variable`, which is automatically registered as a
  /// global variable with the ast::Module.
  template <typename NAME>
  ast::Variable* Global(NAME&& name,
                        const ast::Type* type,
                        ast::StorageClass storage,
                        ast::Expression* constructor = nullptr,
                        ast::DecorationList decorations = {}) {
    auto* var =
        Var(std::forward<NAME>(name), type, storage, constructor, decorations);
    AST().AddGlobalVariable(var);
    return var;
  }

  /// @param source the variable source
  /// @param name the variable name
  /// @param type the variable type
  /// @param storage the variable storage class
  /// @param constructor constructor expression
  /// @param decorations variable decorations
  /// @returns a new `ast::Variable`, which is automatically registered as a
  /// global variable with the ast::Module.
  template <typename NAME>
  ast::Variable* Global(const Source& source,
                        NAME&& name,
                        ast::Type* type,
                        ast::StorageClass storage,
                        ast::Expression* constructor = nullptr,
                        ast::DecorationList decorations = {}) {
    auto* var = Var(source, std::forward<NAME>(name), type, storage,
                    constructor, decorations);
    AST().AddGlobalVariable(var);
    return var;
  }

  /// @param name the variable name
  /// @param type the variable type
  /// @param constructor constructor expression
  /// @param decorations optional variable decorations
  /// @returns a const `ast::Variable` constructed by calling Var() with the
  /// arguments of `args`, which is automatically registered as a global
  /// variable with the ast::Module.
  template <typename NAME>
  ast::Variable* GlobalConst(NAME&& name,
                             ast::Type* type,
                             ast::Expression* constructor,
                             ast::DecorationList decorations = {}) {
    auto* var = Const(std::forward<NAME>(name), type, constructor,
                      std::move(decorations));
    AST().AddGlobalVariable(var);
    return var;
  }

  /// @param source the variable source
  /// @param name the variable name
  /// @param type the variable type
  /// @param constructor constructor expression
  /// @param decorations optional variable decorations
  /// @returns a const `ast::Variable` constructed by calling Var() with the
  /// arguments of `args`, which is automatically registered as a global
  /// variable with the ast::Module.
  template <typename NAME>
  ast::Variable* GlobalConst(const Source& source,
                             NAME&& name,
                             ast::Type* type,
                             ast::Expression* constructor,
                             ast::DecorationList decorations = {}) {
    auto* var = Const(source, std::forward<NAME>(name), type, constructor,
                      std::move(decorations));
    AST().AddGlobalVariable(var);
    return var;
  }

  /// @param source the source information
  /// @param expr the expression to take the address of
  /// @return an ast::UnaryOpExpression that takes the address of `expr`
  template <typename EXPR>
  ast::UnaryOpExpression* AddressOf(const Source& source, EXPR&& expr) {
    return create<ast::UnaryOpExpression>(source, ast::UnaryOp::kAddressOf,
                                          Expr(std::forward<EXPR>(expr)));
  }

  /// @param expr the expression to take the address of
  /// @return an ast::UnaryOpExpression that takes the address of `expr`
  template <typename EXPR>
  ast::UnaryOpExpression* AddressOf(EXPR&& expr) {
    return create<ast::UnaryOpExpression>(ast::UnaryOp::kAddressOf,
                                          Expr(std::forward<EXPR>(expr)));
  }

  /// @param source the source information
  /// @param expr the expression to perform an indirection on
  /// @return an ast::UnaryOpExpression that dereferences the pointer `expr`
  template <typename EXPR>
  ast::UnaryOpExpression* Deref(const Source& source, EXPR&& expr) {
    return create<ast::UnaryOpExpression>(source, ast::UnaryOp::kIndirection,
                                          Expr(std::forward<EXPR>(expr)));
  }

  /// @param expr the expression to perform an indirection on
  /// @return an ast::UnaryOpExpression that dereferences the pointer `expr`
  template <typename EXPR>
  ast::UnaryOpExpression* Deref(EXPR&& expr) {
    return create<ast::UnaryOpExpression>(ast::UnaryOp::kIndirection,
                                          Expr(std::forward<EXPR>(expr)));
  }

  /// @param func the function name
  /// @param args the function call arguments
  /// @returns a `ast::CallExpression` to the function `func`, with the
  /// arguments of `args` converted to `ast::Expression`s using `Expr()`.
  template <typename NAME, typename... ARGS>
  ast::CallExpression* Call(NAME&& func, ARGS&&... args) {
    return create<ast::CallExpression>(Expr(func),
                                       ExprList(std::forward<ARGS>(args)...));
  }

  /// @param lhs the left hand argument to the addition operation
  /// @param rhs the right hand argument to the addition operation
  /// @returns a `ast::BinaryExpression` summing the arguments `lhs` and `rhs`
  template <typename LHS, typename RHS>
  ast::BinaryExpression* Add(LHS&& lhs, RHS&& rhs) {
    return create<ast::BinaryExpression>(ast::BinaryOp::kAdd,
                                         Expr(std::forward<LHS>(lhs)),
                                         Expr(std::forward<RHS>(rhs)));
  }

  /// @param lhs the left hand argument to the subtraction operation
  /// @param rhs the right hand argument to the subtraction operation
  /// @returns a `ast::BinaryExpression` subtracting `rhs` from `lhs`
  template <typename LHS, typename RHS>
  ast::BinaryExpression* Sub(LHS&& lhs, RHS&& rhs) {
    return create<ast::BinaryExpression>(ast::BinaryOp::kSubtract,
                                         Expr(std::forward<LHS>(lhs)),
                                         Expr(std::forward<RHS>(rhs)));
  }

  /// @param lhs the left hand argument to the multiplication operation
  /// @param rhs the right hand argument to the multiplication operation
  /// @returns a `ast::BinaryExpression` multiplying `rhs` from `lhs`
  template <typename LHS, typename RHS>
  ast::BinaryExpression* Mul(LHS&& lhs, RHS&& rhs) {
    return create<ast::BinaryExpression>(ast::BinaryOp::kMultiply,
                                         Expr(std::forward<LHS>(lhs)),
                                         Expr(std::forward<RHS>(rhs)));
  }

  /// @param source the source information
  /// @param lhs the left hand argument to the multiplication operation
  /// @param rhs the right hand argument to the multiplication operation
  /// @returns a `ast::BinaryExpression` multiplying `rhs` from `lhs`
  template <typename LHS, typename RHS>
  ast::BinaryExpression* Mul(const Source& source, LHS&& lhs, RHS&& rhs) {
    return create<ast::BinaryExpression>(source, ast::BinaryOp::kMultiply,
                                         Expr(std::forward<LHS>(lhs)),
                                         Expr(std::forward<RHS>(rhs)));
  }

  /// @param lhs the left hand argument to the division operation
  /// @param rhs the right hand argument to the division operation
  /// @returns a `ast::BinaryExpression` dividing `lhs` by `rhs`
  template <typename LHS, typename RHS>
  ast::Expression* Div(LHS&& lhs, RHS&& rhs) {
    return create<ast::BinaryExpression>(ast::BinaryOp::kDivide,
                                         Expr(std::forward<LHS>(lhs)),
                                         Expr(std::forward<RHS>(rhs)));
  }

  /// @param arr the array argument for the array accessor expression
  /// @param idx the index argument for the array accessor expression
  /// @returns a `ast::ArrayAccessorExpression` that indexes `arr` with `idx`
  template <typename ARR, typename IDX>
  ast::ArrayAccessorExpression* IndexAccessor(ARR&& arr, IDX&& idx) {
    return create<ast::ArrayAccessorExpression>(Expr(std::forward<ARR>(arr)),
                                                Expr(std::forward<IDX>(idx)));
  }

  /// @param obj the object for the member accessor expression
  /// @param idx the index argument for the array accessor expression
  /// @returns a `ast::MemberAccessorExpression` that indexes `obj` with `idx`
  template <typename OBJ, typename IDX>
  ast::MemberAccessorExpression* MemberAccessor(OBJ&& obj, IDX&& idx) {
    return create<ast::MemberAccessorExpression>(Expr(std::forward<OBJ>(obj)),
                                                 Expr(std::forward<IDX>(idx)));
  }

  /// Creates a ast::StructMemberOffsetDecoration
  /// @param val the offset value
  /// @returns the offset decoration pointer
  ast::StructMemberOffsetDecoration* MemberOffset(uint32_t val) {
    return create<ast::StructMemberOffsetDecoration>(source_, val);
  }

  /// Creates a ast::StructMemberSizeDecoration
  /// @param source the source information
  /// @param val the size value
  /// @returns the size decoration pointer
  ast::StructMemberSizeDecoration* MemberSize(const Source& source,
                                              uint32_t val) {
    return create<ast::StructMemberSizeDecoration>(source, val);
  }

  /// Creates a ast::StructMemberSizeDecoration
  /// @param val the size value
  /// @returns the size decoration pointer
  ast::StructMemberSizeDecoration* MemberSize(uint32_t val) {
    return create<ast::StructMemberSizeDecoration>(source_, val);
  }

  /// Creates a ast::StructMemberAlignDecoration
  /// @param source the source information
  /// @param val the align value
  /// @returns the align decoration pointer
  ast::StructMemberAlignDecoration* MemberAlign(const Source& source,
                                                uint32_t val) {
    return create<ast::StructMemberAlignDecoration>(source, val);
  }

  /// Creates a ast::StructMemberAlignDecoration
  /// @param val the align value
  /// @returns the align decoration pointer
  ast::StructMemberAlignDecoration* MemberAlign(uint32_t val) {
    return create<ast::StructMemberAlignDecoration>(source_, val);
  }

  /// Creates an ast::Function and registers it with the ast::Module.
  /// @param source the source information
  /// @param name the function name
  /// @param params the function parameters
  /// @param type the function return type
  /// @param body the function body
  /// @param decorations the optional function decorations
  /// @param return_type_decorations the optional function return type
  /// decorations
  /// @returns the function pointer
  template <typename NAME>
  ast::Function* Func(const Source& source,
                      NAME&& name,
                      ast::VariableList params,
                      ast::Type* type,
                      ast::StatementList body,
                      ast::DecorationList decorations = {},
                      ast::DecorationList return_type_decorations = {}) {
    type = ty.MaybeCreateTypename(type);
    auto* func =
        create<ast::Function>(source, Sym(std::forward<NAME>(name)), params,
                              type, create<ast::BlockStatement>(body),
                              decorations, return_type_decorations);
    AST().AddFunction(func);
    return func;
  }

  /// Creates an ast::Function and registers it with the ast::Module.
  /// @param name the function name
  /// @param params the function parameters
  /// @param type the function return type
  /// @param body the function body
  /// @param decorations the optional function decorations
  /// @param return_type_decorations the optional function return type
  /// decorations
  /// @returns the function pointer
  template <typename NAME>
  ast::Function* Func(NAME&& name,
                      ast::VariableList params,
                      ast::Type* type,
                      ast::StatementList body,
                      ast::DecorationList decorations = {},
                      ast::DecorationList return_type_decorations = {}) {
    type = ty.MaybeCreateTypename(type);
    auto* func = create<ast::Function>(Sym(std::forward<NAME>(name)), params,
                                       type, create<ast::BlockStatement>(body),
                                       decorations, return_type_decorations);
    AST().AddFunction(func);
    return func;
  }

  /// Creates an ast::ReturnStatement with no return value
  /// @param source the source information
  /// @returns the return statement pointer
  ast::ReturnStatement* Return(const Source& source) {
    return create<ast::ReturnStatement>(source);
  }

  /// Creates an ast::ReturnStatement with no return value
  /// @returns the return statement pointer
  ast::ReturnStatement* Return() { return create<ast::ReturnStatement>(); }

  /// Creates an ast::ReturnStatement with the given return value
  /// @param source the source information
  /// @param val the return value
  /// @returns the return statement pointer
  template <typename EXPR>
  ast::ReturnStatement* Return(const Source& source, EXPR&& val) {
    return create<ast::ReturnStatement>(source, Expr(std::forward<EXPR>(val)));
  }

  /// Creates an ast::ReturnStatement with the given return value
  /// @param val the return value
  /// @returns the return statement pointer
  template <typename EXPR>
  ast::ReturnStatement* Return(EXPR&& val) {
    return create<ast::ReturnStatement>(Expr(std::forward<EXPR>(val)));
  }

  /// Creates a ast::Struct registering it with the AST().ConstructedTypes().
  /// @param source the source information
  /// @param name the struct name
  /// @param members the struct members
  /// @param decorations the optional struct decorations
  /// @returns the struct type
  template <typename NAME>
  ast::Struct* Structure(const Source& source,
                         NAME&& name,
                         ast::StructMemberList members,
                         ast::DecorationList decorations = {}) {
    auto sym = Sym(std::forward<NAME>(name));
    auto* type = create<ast::Struct>(source, sym, std::move(members),
                                     std::move(decorations));
    AST().AddConstructedType(type);
    return type;
  }

  /// Creates a ast::Struct registering it with the AST().ConstructedTypes().
  /// @param name the struct name
  /// @param members the struct members
  /// @param decorations the optional struct decorations
  /// @returns the struct type
  template <typename NAME>
  ast::Struct* Structure(NAME&& name,
                         ast::StructMemberList members,
                         ast::DecorationList decorations = {}) {
    auto sym = Sym(std::forward<NAME>(name));
    auto* type =
        create<ast::Struct>(sym, std::move(members), std::move(decorations));
    AST().AddConstructedType(type);
    return type;
  }

  /// Creates a ast::StructMember
  /// @param source the source information
  /// @param name the struct member name
  /// @param type the struct member type
  /// @param decorations the optional struct member decorations
  /// @returns the struct member pointer
  template <typename NAME>
  ast::StructMember* Member(const Source& source,
                            NAME&& name,
                            ast::Type* type,
                            ast::DecorationList decorations = {}) {
    type = ty.MaybeCreateTypename(type);
    return create<ast::StructMember>(source, Sym(std::forward<NAME>(name)),
                                     type, std::move(decorations));
  }

  /// Creates a ast::StructMember
  /// @param name the struct member name
  /// @param type the struct member type
  /// @param decorations the optional struct member decorations
  /// @returns the struct member pointer
  template <typename NAME>
  ast::StructMember* Member(NAME&& name,
                            ast::Type* type,
                            ast::DecorationList decorations = {}) {
    type = ty.MaybeCreateTypename(type);
    return create<ast::StructMember>(source_, Sym(std::forward<NAME>(name)),
                                     type, std::move(decorations));
  }

  /// Creates a ast::StructMember with the given byte offset
  /// @param offset the offset to use in the StructMemberOffsetDecoration
  /// @param name the struct member name
  /// @param type the struct member type
  /// @returns the struct member pointer
  template <typename NAME>
  ast::StructMember* Member(uint32_t offset, NAME&& name, ast::Type* type) {
    type = ty.MaybeCreateTypename(type);
    return create<ast::StructMember>(
        source_, Sym(std::forward<NAME>(name)), type,
        ast::DecorationList{
            create<ast::StructMemberOffsetDecoration>(offset),
        });
  }

  /// Creates a ast::BlockStatement with input statements
  /// @param statements statements of block
  /// @returns the block statement pointer
  template <typename... Statements>
  ast::BlockStatement* Block(Statements&&... statements) {
    return create<ast::BlockStatement>(
        ast::StatementList{std::forward<Statements>(statements)...});
  }

  /// Creates a ast::ElseStatement with input condition and body
  /// @param condition the else condition expression
  /// @param body the else body
  /// @returns the else statement pointer
  template <typename CONDITION>
  ast::ElseStatement* Else(CONDITION&& condition, ast::BlockStatement* body) {
    return create<ast::ElseStatement>(Expr(std::forward<CONDITION>(condition)),
                                      body);
  }

  /// Creates a ast::IfStatement with input condition, body, and optional
  /// variadic else statements
  /// @param condition the if statement condition expression
  /// @param body the if statement body
  /// @param elseStatements optional variadic else statements
  /// @returns the if statement pointer
  template <typename CONDITION, typename... ELSE_STATEMENTS>
  ast::IfStatement* If(CONDITION&& condition,
                       ast::BlockStatement* body,
                       ELSE_STATEMENTS&&... elseStatements) {
    return create<ast::IfStatement>(
        Expr(std::forward<CONDITION>(condition)), body,
        ast::ElseStatementList{
            std::forward<ELSE_STATEMENTS>(elseStatements)...});
  }

  /// Creates a ast::AssignmentStatement with input lhs and rhs expressions
  /// @param source the source information
  /// @param lhs the left hand side expression initializer
  /// @param rhs the right hand side expression initializer
  /// @returns the assignment statement pointer
  template <typename LhsExpressionInit, typename RhsExpressionInit>
  ast::AssignmentStatement* Assign(const Source& source,
                                   LhsExpressionInit&& lhs,
                                   RhsExpressionInit&& rhs) {
    return create<ast::AssignmentStatement>(
        source, Expr(std::forward<LhsExpressionInit>(lhs)),
        Expr(std::forward<RhsExpressionInit>(rhs)));
  }

  /// Creates a ast::AssignmentStatement with input lhs and rhs expressions
  /// @param lhs the left hand side expression initializer
  /// @param rhs the right hand side expression initializer
  /// @returns the assignment statement pointer
  template <typename LhsExpressionInit, typename RhsExpressionInit>
  ast::AssignmentStatement* Assign(LhsExpressionInit&& lhs,
                                   RhsExpressionInit&& rhs) {
    return create<ast::AssignmentStatement>(
        Expr(std::forward<LhsExpressionInit>(lhs)),
        Expr(std::forward<RhsExpressionInit>(rhs)));
  }

  /// Creates a ast::LoopStatement with input body and optional continuing
  /// @param body the loop body
  /// @param continuing the optional continuing block
  /// @returns the loop statement pointer
  ast::LoopStatement* Loop(ast::BlockStatement* body,
                           ast::BlockStatement* continuing = nullptr) {
    return create<ast::LoopStatement>(body, continuing);
  }

  /// Creates a ast::VariableDeclStatement for the input variable
  /// @param source the source information
  /// @param var the variable to wrap in a decl statement
  /// @returns the variable decl statement pointer
  ast::VariableDeclStatement* Decl(const Source& source, ast::Variable* var) {
    return create<ast::VariableDeclStatement>(source, var);
  }

  /// Creates a ast::VariableDeclStatement for the input variable
  /// @param var the variable to wrap in a decl statement
  /// @returns the variable decl statement pointer
  ast::VariableDeclStatement* Decl(ast::Variable* var) {
    return create<ast::VariableDeclStatement>(var);
  }

  /// Creates a ast::SwitchStatement with input expression and cases
  /// @param condition the condition expression initializer
  /// @param cases case statements
  /// @returns the switch statement pointer
  template <typename ExpressionInit, typename... Cases>
  ast::SwitchStatement* Switch(ExpressionInit&& condition, Cases&&... cases) {
    return create<ast::SwitchStatement>(
        Expr(std::forward<ExpressionInit>(condition)),
        ast::CaseStatementList{std::forward<Cases>(cases)...});
  }

  /// Creates a ast::CaseStatement with input list of selectors, and body
  /// @param selectors list of selectors
  /// @param body the case body
  /// @returns the case statement pointer
  ast::CaseStatement* Case(ast::CaseSelectorList selectors,
                           ast::BlockStatement* body = nullptr) {
    return create<ast::CaseStatement>(std::move(selectors),
                                      body ? body : Block());
  }

  /// Convenient overload that takes a single selector
  /// @param selector a single case selector
  /// @param body the case body
  /// @returns the case statement pointer
  ast::CaseStatement* Case(ast::IntLiteral* selector,
                           ast::BlockStatement* body = nullptr) {
    return Case(ast::CaseSelectorList{selector}, body);
  }

  /// Convenience function that creates a 'default' ast::CaseStatement
  /// @param body the case body
  /// @returns the case statement pointer
  ast::CaseStatement* DefaultCase(ast::BlockStatement* body = nullptr) {
    return Case(ast::CaseSelectorList{}, body);
  }

  /// Creates an ast::BuiltinDecoration
  /// @param source the source information
  /// @param builtin the builtin value
  /// @returns the builtin decoration pointer
  ast::BuiltinDecoration* Builtin(const Source& source, ast::Builtin builtin) {
    return create<ast::BuiltinDecoration>(source, builtin);
  }

  /// Creates an ast::BuiltinDecoration
  /// @param builtin the builtin value
  /// @returns the builtin decoration pointer
  ast::BuiltinDecoration* Builtin(ast::Builtin builtin) {
    return create<ast::BuiltinDecoration>(source_, builtin);
  }

  /// Creates an ast::LocationDecoration
  /// @param source the source information
  /// @param location the location value
  /// @returns the location decoration pointer
  ast::LocationDecoration* Location(const Source& source, uint32_t location) {
    return create<ast::LocationDecoration>(source, location);
  }

  /// Creates an ast::LocationDecoration
  /// @param location the location value
  /// @returns the location decoration pointer
  ast::LocationDecoration* Location(uint32_t location) {
    return create<ast::LocationDecoration>(source_, location);
  }

  /// Creates an ast::OverrideDecoration with a specific constant ID
  /// @param source the source information
  /// @param id the id value
  /// @returns the override decoration pointer
  ast::OverrideDecoration* Override(const Source& source, uint32_t id) {
    return create<ast::OverrideDecoration>(source, id);
  }

  /// Creates an ast::OverrideDecoration with a specific constant ID
  /// @param id the optional id value
  /// @returns the override decoration pointer
  ast::OverrideDecoration* Override(uint32_t id) {
    return Override(source_, id);
  }

  /// Creates an ast::OverrideDecoration without a constant ID
  /// @param source the source information
  /// @returns the override decoration pointer
  ast::OverrideDecoration* Override(const Source& source) {
    return create<ast::OverrideDecoration>(source);
  }

  /// Creates an ast::OverrideDecoration without a constant ID
  /// @returns the override decoration pointer
  ast::OverrideDecoration* Override() { return Override(source_); }

  /// Creates an ast::StageDecoration
  /// @param source the source information
  /// @param stage the pipeline stage
  /// @returns the stage decoration pointer
  ast::StageDecoration* Stage(const Source& source, ast::PipelineStage stage) {
    return create<ast::StageDecoration>(source, stage);
  }

  /// Creates an ast::StageDecoration
  /// @param stage the pipeline stage
  /// @returns the stage decoration pointer
  ast::StageDecoration* Stage(ast::PipelineStage stage) {
    return create<ast::StageDecoration>(source_, stage);
  }

  /// Creates an ast::WorkgroupDecoration
  /// @param x the x dimension expression
  /// @returns the workgroup decoration pointer
  template <typename EXPR_X>
  ast::WorkgroupDecoration* WorkgroupSize(EXPR_X&& x) {
    return WorkgroupSize(std::forward<EXPR_X>(x), nullptr, nullptr);
  }

  /// Creates an ast::WorkgroupDecoration
  /// @param x the x dimension expression
  /// @param y the y dimension expression
  /// @returns the workgroup decoration pointer
  template <typename EXPR_X, typename EXPR_Y>
  ast::WorkgroupDecoration* WorkgroupSize(EXPR_X&& x, EXPR_Y&& y) {
    return WorkgroupSize(std::forward<EXPR_X>(x), std::forward<EXPR_Y>(y),
                         nullptr);
  }

  /// Creates an ast::WorkgroupDecoration
  /// @param x the x dimension expression
  /// @param y the y dimension expression
  /// @param z the z dimension expression
  /// @returns the workgroup decoration pointer
  template <typename EXPR_X, typename EXPR_Y, typename EXPR_Z>
  ast::WorkgroupDecoration* WorkgroupSize(EXPR_X&& x, EXPR_Y&& y, EXPR_Z&& z) {
    return create<ast::WorkgroupDecoration>(
        source_, Expr(std::forward<EXPR_X>(x)), Expr(std::forward<EXPR_Y>(y)),
        Expr(std::forward<EXPR_Z>(z)));
  }

  /// Sets the current builder source to `src`
  /// @param src the Source used for future create() calls
  void SetSource(const Source& src) {
    AssertNotMoved();
    source_ = src;
  }

  /// Sets the current builder source to `loc`
  /// @param loc the Source used for future create() calls
  void SetSource(const Source::Location& loc) {
    AssertNotMoved();
    source_ = Source(loc);
  }

  /// Helper for returning the resolved semantic type of the expression `expr`.
  /// @note As the Resolver is run when the Program is built, this will only be
  /// useful for the Resolver itself and tests that use their own Resolver.
  /// @param expr the AST expression
  /// @return the resolved semantic type for the expression, or nullptr if the
  /// expression has no resolved type.
  sem::Type* TypeOf(const ast::Expression* expr) const;

  /// Helper for returning the resolved semantic type of the variable `var`.
  /// @note As the Resolver is run when the Program is built, this will only be
  /// useful for the Resolver itself and tests that use their own Resolver.
  /// @param var the AST variable
  /// @return the resolved semantic type for the variable, or nullptr if the
  /// variable has no resolved type.
  sem::Type* TypeOf(const ast::Variable* var) const;

  /// Helper for returning the resolved semantic type of the AST type `type`.
  /// @note As the Resolver is run when the Program is built, this will only be
  /// useful for the Resolver itself and tests that use their own Resolver.
  /// @param expr the AST type
  /// @return the resolved semantic type for the type, or nullptr if the type
  /// has no resolved type.
  const sem::Type* TypeOf(const ast::Type* expr) const;

  /// Wraps the ast::Literal in a statement. This is used by tests that
  /// construct a partial AST and require the Resolver to reach these
  /// nodes.
  /// @param lit the ast::Literal to be wrapped by an ast::Statement
  /// @return the ast::Statement that wraps the ast::Statement
  ast::Statement* WrapInStatement(ast::Literal* lit);
  /// Wraps the ast::Expression in a statement. This is used by tests that
  /// construct a partial AST and require the Resolver to reach these
  /// nodes.
  /// @param expr the ast::Expression to be wrapped by an ast::Statement
  /// @return the ast::Statement that wraps the ast::Expression
  ast::Statement* WrapInStatement(ast::Expression* expr);
  /// Wraps the ast::Variable in a ast::VariableDeclStatement. This is used by
  /// tests that construct a partial AST and require the Resolver to reach
  /// these nodes.
  /// @param v the ast::Variable to be wrapped by an ast::VariableDeclStatement
  /// @return the ast::VariableDeclStatement that wraps the ast::Variable
  ast::VariableDeclStatement* WrapInStatement(ast::Variable* v);
  /// Returns the statement argument. Used as a passthrough-overload by
  /// WrapInFunction().
  /// @param stmt the ast::Statement
  /// @return `stmt`
  ast::Statement* WrapInStatement(ast::Statement* stmt);
  /// Wraps the list of arguments in a simple function so that each is reachable
  /// by the Resolver.
  /// @param args a mix of ast::Expression, ast::Statement, ast::Variables.
  /// @returns the function
  template <typename... ARGS>
  ast::Function* WrapInFunction(ARGS&&... args) {
    ast::StatementList stmts{WrapInStatement(std::forward<ARGS>(args))...};
    return WrapInFunction(std::move(stmts));
  }
  /// @param stmts a list of ast::Statement that will be wrapped by a function,
  /// so that each statement is reachable by the Resolver.
  /// @returns the function
  ast::Function* WrapInFunction(ast::StatementList stmts);

  /// The builder types
  TypesBuilder const ty{this};

 protected:
  /// Asserts that the builder has not been moved.
  void AssertNotMoved() const;

 private:
  ProgramID id_;
  sem::Manager types_;
  ASTNodeAllocator ast_nodes_;
  SemNodeAllocator sem_nodes_;
  ast::Module* ast_;
  sem::Info sem_;
  SymbolTable symbols_{id_};
  diag::List diagnostics_;

  /// The source to use when creating AST nodes without providing a Source as
  /// the first argument.
  Source source_;

  /// Set by SetResolveOnBuild(). If set, the Resolver will be run on the
  /// program when built.
  bool resolve_on_build_ = true;

  /// Set by MarkAsMoved(). Once set, no methods may be called on this builder.
  bool moved_ = false;
};

//! @cond Doxygen_Suppress
// Various template specializations for ProgramBuilder::TypesBuilder::CToAST.
template <>
struct ProgramBuilder::TypesBuilder::CToAST<ProgramBuilder::i32> {
  static ast::Type* get(const ProgramBuilder::TypesBuilder* t) {
    return t->i32();
  }
};
template <>
struct ProgramBuilder::TypesBuilder::CToAST<ProgramBuilder::u32> {
  static ast::Type* get(const ProgramBuilder::TypesBuilder* t) {
    return t->u32();
  }
};
template <>
struct ProgramBuilder::TypesBuilder::CToAST<ProgramBuilder::f32> {
  static ast::Type* get(const ProgramBuilder::TypesBuilder* t) {
    return t->f32();
  }
};
template <>
struct ProgramBuilder::TypesBuilder::CToAST<bool> {
  static ast::Type* get(const ProgramBuilder::TypesBuilder* t) {
    return t->bool_();
  }
};
template <>
struct ProgramBuilder::TypesBuilder::CToAST<void> {
  static ast::Type* get(const ProgramBuilder::TypesBuilder* t) {
    return t->void_();
  }
};
//! @endcond

/// @param builder the ProgramBuilder
/// @returns the ProgramID of the ProgramBuilder
inline ProgramID ProgramIDOf(const ProgramBuilder* builder) {
  return builder->ID();
}

}  // namespace tint

#endif  // SRC_PROGRAM_BUILDER_H_
