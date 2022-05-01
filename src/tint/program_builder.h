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

#ifndef SRC_TINT_PROGRAM_BUILDER_H_
#define SRC_TINT_PROGRAM_BUILDER_H_

#include <string>
#include <unordered_set>
#include <utility>

#include "src/tint/ast/alias.h"
#include "src/tint/ast/array.h"
#include "src/tint/ast/assignment_statement.h"
#include "src/tint/ast/atomic.h"
#include "src/tint/ast/binary_expression.h"
#include "src/tint/ast/binding_attribute.h"
#include "src/tint/ast/bitcast_expression.h"
#include "src/tint/ast/bool.h"
#include "src/tint/ast/bool_literal_expression.h"
#include "src/tint/ast/break_statement.h"
#include "src/tint/ast/call_expression.h"
#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/case_statement.h"
#include "src/tint/ast/compound_assignment_statement.h"
#include "src/tint/ast/continue_statement.h"
#include "src/tint/ast/depth_multisampled_texture.h"
#include "src/tint/ast/depth_texture.h"
#include "src/tint/ast/disable_validation_attribute.h"
#include "src/tint/ast/discard_statement.h"
#include "src/tint/ast/enable.h"
#include "src/tint/ast/external_texture.h"
#include "src/tint/ast/f32.h"
#include "src/tint/ast/fallthrough_statement.h"
#include "src/tint/ast/float_literal_expression.h"
#include "src/tint/ast/for_loop_statement.h"
#include "src/tint/ast/i32.h"
#include "src/tint/ast/id_attribute.h"
#include "src/tint/ast/if_statement.h"
#include "src/tint/ast/increment_decrement_statement.h"
#include "src/tint/ast/index_accessor_expression.h"
#include "src/tint/ast/interpolate_attribute.h"
#include "src/tint/ast/invariant_attribute.h"
#include "src/tint/ast/loop_statement.h"
#include "src/tint/ast/matrix.h"
#include "src/tint/ast/member_accessor_expression.h"
#include "src/tint/ast/module.h"
#include "src/tint/ast/multisampled_texture.h"
#include "src/tint/ast/phony_expression.h"
#include "src/tint/ast/pointer.h"
#include "src/tint/ast/return_statement.h"
#include "src/tint/ast/sampled_texture.h"
#include "src/tint/ast/sampler.h"
#include "src/tint/ast/sint_literal_expression.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/ast/storage_texture.h"
#include "src/tint/ast/stride_attribute.h"
#include "src/tint/ast/struct_member_align_attribute.h"
#include "src/tint/ast/struct_member_offset_attribute.h"
#include "src/tint/ast/struct_member_size_attribute.h"
#include "src/tint/ast/switch_statement.h"
#include "src/tint/ast/type_name.h"
#include "src/tint/ast/u32.h"
#include "src/tint/ast/uint_literal_expression.h"
#include "src/tint/ast/unary_op_expression.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/ast/vector.h"
#include "src/tint/ast/void.h"
#include "src/tint/ast/workgroup_attribute.h"
#include "src/tint/program.h"
#include "src/tint/program_id.h"
#include "src/tint/sem/array.h"
#include "src/tint/sem/bool.h"
#include "src/tint/sem/depth_texture.h"
#include "src/tint/sem/external_texture.h"
#include "src/tint/sem/f32.h"
#include "src/tint/sem/i32.h"
#include "src/tint/sem/matrix.h"
#include "src/tint/sem/multisampled_texture.h"
#include "src/tint/sem/pointer.h"
#include "src/tint/sem/sampled_texture.h"
#include "src/tint/sem/storage_texture.h"
#include "src/tint/sem/struct.h"
#include "src/tint/sem/u32.h"
#include "src/tint/sem/vector.h"
#include "src/tint/sem/void.h"

#ifdef INCLUDE_TINT_TINT_H_
#error "internal tint header being #included from tint.h"
#endif

// Forward declarations
namespace tint {
class CloneContext;
}  // namespace tint
namespace tint::ast {
class VariableDeclStatement;
}  // namespace tint::ast

namespace tint {

/// ProgramBuilder is a mutable builder for a Program.
/// To construct a Program, populate the builder and then `std::move` it to a
/// Program.
class ProgramBuilder {
    /// A helper used to disable overloads if the first type in `TYPES` is a
    /// Source. Used to avoid ambiguities in overloads that take a Source as the
    /// first parameter and those that perfectly-forward the first argument.
    template <typename... TYPES>
    using DisableIfSource =
        traits::EnableIfIsNotType<traits::Decay<traits::NthTypeOf<0, TYPES..., void>>, Source>;

    /// VarOptionals is a helper for accepting a number of optional, extra
    /// arguments for Var() and Global().
    struct VarOptionals {
        template <typename... ARGS>
        explicit VarOptionals(ARGS&&... args) {
            Apply(std::forward<ARGS>(args)...);
        }
        ~VarOptionals();

        ast::StorageClass storage = ast::StorageClass::kNone;
        ast::Access access = ast::Access::kUndefined;
        const ast::Expression* constructor = nullptr;
        ast::AttributeList attributes = {};

      private:
        void Set(ast::StorageClass sc) { storage = sc; }
        void Set(ast::Access ac) { access = ac; }
        void Set(const ast::Expression* c) { constructor = c; }
        void Set(const ast::AttributeList& l) { attributes = l; }

        template <typename FIRST, typename... ARGS>
        void Apply(FIRST&& first, ARGS&&... args) {
            Set(std::forward<FIRST>(first));
            Apply(std::forward<ARGS>(args)...);
        }
        void Apply() {}
    };

  public:
    /// ASTNodeAllocator is an alias to BlockAllocator<ast::Node>
    using ASTNodeAllocator = utils::BlockAllocator<ast::Node>;

    /// SemNodeAllocator is an alias to BlockAllocator<sem::Node>
    using SemNodeAllocator = utils::BlockAllocator<sem::Node>;

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

    /// Creates a new ast::Node owned by the ProgramBuilder. When the
    /// ProgramBuilder is destructed, the ast::Node will also be destructed.
    /// @param source the Source of the node
    /// @param args the arguments to pass to the type constructor
    /// @returns the node pointer
    template <typename T, typename... ARGS>
    traits::EnableIfIsType<T, ast::Node>* create(const Source& source, ARGS&&... args) {
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
                     traits::IsTypeOrDerived<T, ast::Node> &&
                         !traits::IsTypeOrDerived<ARG0, Source>,
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
    traits::EnableIf<traits::IsTypeOrDerived<T, sem::Node> &&
                         !traits::IsTypeOrDerived<T, sem::Type>,
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
        static_assert(std::is_base_of<sem::Type, T>::value, "T does not derive from sem::Type");
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
        const ast::Type* Of() const {
            return CToAST<T>::get(this);
        }

        /// @returns a boolean type
        const ast::Bool* bool_() const { return builder->create<ast::Bool>(); }

        /// @param source the Source of the node
        /// @returns a boolean type
        const ast::Bool* bool_(const Source& source) const {
            return builder->create<ast::Bool>(source);
        }

        /// @returns a f32 type
        const ast::F32* f32() const { return builder->create<ast::F32>(); }

        /// @param source the Source of the node
        /// @returns a f32 type
        const ast::F32* f32(const Source& source) const {
            return builder->create<ast::F32>(source);
        }

        /// @returns a i32 type
        const ast::I32* i32() const { return builder->create<ast::I32>(); }

        /// @param source the Source of the node
        /// @returns a i32 type
        const ast::I32* i32(const Source& source) const {
            return builder->create<ast::I32>(source);
        }

        /// @returns a u32 type
        const ast::U32* u32() const { return builder->create<ast::U32>(); }

        /// @param source the Source of the node
        /// @returns a u32 type
        const ast::U32* u32(const Source& source) const {
            return builder->create<ast::U32>(source);
        }

        /// @returns a void type
        const ast::Void* void_() const { return builder->create<ast::Void>(); }

        /// @param source the Source of the node
        /// @returns a void type
        const ast::Void* void_(const Source& source) const {
            return builder->create<ast::Void>(source);
        }

        /// @param type vector subtype
        /// @param n vector width in elements
        /// @return the tint AST type for a `n`-element vector of `type`.
        const ast::Vector* vec(const ast::Type* type, uint32_t n) const {
            return builder->create<ast::Vector>(type, n);
        }

        /// @param source the Source of the node
        /// @param type vector subtype
        /// @param n vector width in elements
        /// @return the tint AST type for a `n`-element vector of `type`.
        const ast::Vector* vec(const Source& source, const ast::Type* type, uint32_t n) const {
            return builder->create<ast::Vector>(source, type, n);
        }

        /// @param type vector subtype
        /// @return the tint AST type for a 2-element vector of `type`.
        const ast::Vector* vec2(const ast::Type* type) const { return vec(type, 2u); }

        /// @param type vector subtype
        /// @return the tint AST type for a 3-element vector of `type`.
        const ast::Vector* vec3(const ast::Type* type) const { return vec(type, 3u); }

        /// @param type vector subtype
        /// @return the tint AST type for a 4-element vector of `type`.
        const ast::Vector* vec4(const ast::Type* type) const { return vec(type, 4u); }

        /// @param n vector width in elements
        /// @return the tint AST type for a `n`-element vector of `type`.
        template <typename T>
        const ast::Vector* vec(uint32_t n) const {
            return vec(Of<T>(), n);
        }

        /// @return the tint AST type for a 2-element vector of the C type `T`.
        template <typename T>
        const ast::Vector* vec2() const {
            return vec2(Of<T>());
        }

        /// @return the tint AST type for a 3-element vector of the C type `T`.
        template <typename T>
        const ast::Vector* vec3() const {
            return vec3(Of<T>());
        }

        /// @return the tint AST type for a 4-element vector of the C type `T`.
        template <typename T>
        const ast::Vector* vec4() const {
            return vec4(Of<T>());
        }

        /// @param type matrix subtype
        /// @param columns number of columns for the matrix
        /// @param rows number of rows for the matrix
        /// @return the tint AST type for a matrix of `type`
        const ast::Matrix* mat(const ast::Type* type, uint32_t columns, uint32_t rows) const {
            return builder->create<ast::Matrix>(type, rows, columns);
        }

        /// @param source the Source of the node
        /// @param type matrix subtype
        /// @param columns number of columns for the matrix
        /// @param rows number of rows for the matrix
        /// @return the tint AST type for a matrix of `type`
        const ast::Matrix* mat(const Source& source,
                               const ast::Type* type,
                               uint32_t columns,
                               uint32_t rows) const {
            return builder->create<ast::Matrix>(source, type, rows, columns);
        }

        /// @param type matrix subtype
        /// @return the tint AST type for a 2x3 matrix of `type`.
        const ast::Matrix* mat2x2(const ast::Type* type) const { return mat(type, 2u, 2u); }

        /// @param type matrix subtype
        /// @return the tint AST type for a 2x3 matrix of `type`.
        const ast::Matrix* mat2x3(const ast::Type* type) const { return mat(type, 2u, 3u); }

        /// @param type matrix subtype
        /// @return the tint AST type for a 2x4 matrix of `type`.
        const ast::Matrix* mat2x4(const ast::Type* type) const { return mat(type, 2u, 4u); }

        /// @param type matrix subtype
        /// @return the tint AST type for a 3x2 matrix of `type`.
        const ast::Matrix* mat3x2(const ast::Type* type) const { return mat(type, 3u, 2u); }

        /// @param type matrix subtype
        /// @return the tint AST type for a 3x3 matrix of `type`.
        const ast::Matrix* mat3x3(const ast::Type* type) const { return mat(type, 3u, 3u); }

        /// @param type matrix subtype
        /// @return the tint AST type for a 3x4 matrix of `type`.
        const ast::Matrix* mat3x4(const ast::Type* type) const { return mat(type, 3u, 4u); }

        /// @param type matrix subtype
        /// @return the tint AST type for a 4x2 matrix of `type`.
        const ast::Matrix* mat4x2(const ast::Type* type) const { return mat(type, 4u, 2u); }

        /// @param type matrix subtype
        /// @return the tint AST type for a 4x3 matrix of `type`.
        const ast::Matrix* mat4x3(const ast::Type* type) const { return mat(type, 4u, 3u); }

        /// @param type matrix subtype
        /// @return the tint AST type for a 4x4 matrix of `type`.
        const ast::Matrix* mat4x4(const ast::Type* type) const { return mat(type, 4u, 4u); }

        /// @param columns number of columns for the matrix
        /// @param rows number of rows for the matrix
        /// @return the tint AST type for a matrix of `type`
        template <typename T>
        const ast::Matrix* mat(uint32_t columns, uint32_t rows) const {
            return mat(Of<T>(), columns, rows);
        }

        /// @return the tint AST type for a 2x3 matrix of the C type `T`.
        template <typename T>
        const ast::Matrix* mat2x2() const {
            return mat2x2(Of<T>());
        }

        /// @return the tint AST type for a 2x3 matrix of the C type `T`.
        template <typename T>
        const ast::Matrix* mat2x3() const {
            return mat2x3(Of<T>());
        }

        /// @return the tint AST type for a 2x4 matrix of the C type `T`.
        template <typename T>
        const ast::Matrix* mat2x4() const {
            return mat2x4(Of<T>());
        }

        /// @return the tint AST type for a 3x2 matrix of the C type `T`.
        template <typename T>
        const ast::Matrix* mat3x2() const {
            return mat3x2(Of<T>());
        }

        /// @return the tint AST type for a 3x3 matrix of the C type `T`.
        template <typename T>
        const ast::Matrix* mat3x3() const {
            return mat3x3(Of<T>());
        }

        /// @return the tint AST type for a 3x4 matrix of the C type `T`.
        template <typename T>
        const ast::Matrix* mat3x4() const {
            return mat3x4(Of<T>());
        }

        /// @return the tint AST type for a 4x2 matrix of the C type `T`.
        template <typename T>
        const ast::Matrix* mat4x2() const {
            return mat4x2(Of<T>());
        }

        /// @return the tint AST type for a 4x3 matrix of the C type `T`.
        template <typename T>
        const ast::Matrix* mat4x3() const {
            return mat4x3(Of<T>());
        }

        /// @return the tint AST type for a 4x4 matrix of the C type `T`.
        template <typename T>
        const ast::Matrix* mat4x4() const {
            return mat4x4(Of<T>());
        }

        /// @param subtype the array element type
        /// @param n the array size. nullptr represents a runtime-array
        /// @param attrs the optional attributes for the array
        /// @return the tint AST type for a array of size `n` of type `T`
        template <typename EXPR = ast::Expression*>
        const ast::Array* array(const ast::Type* subtype,
                                EXPR&& n = nullptr,
                                ast::AttributeList attrs = {}) const {
            return builder->create<ast::Array>(subtype, builder->Expr(std::forward<EXPR>(n)),
                                               attrs);
        }

        /// @param source the Source of the node
        /// @param subtype the array element type
        /// @param n the array size. nullptr represents a runtime-array
        /// @param attrs the optional attributes for the array
        /// @return the tint AST type for a array of size `n` of type `T`
        template <typename EXPR = ast::Expression*>
        const ast::Array* array(const Source& source,
                                const ast::Type* subtype,
                                EXPR&& n = nullptr,
                                ast::AttributeList attrs = {}) const {
            return builder->create<ast::Array>(source, subtype,
                                               builder->Expr(std::forward<EXPR>(n)), attrs);
        }

        /// @param subtype the array element type
        /// @param n the array size. nullptr represents a runtime-array
        /// @param stride the array stride. 0 represents implicit stride
        /// @return the tint AST type for a array of size `n` of type `T`
        template <typename EXPR>
        const ast::Array* array(const ast::Type* subtype, EXPR&& n, uint32_t stride) const {
            ast::AttributeList attrs;
            if (stride) {
                attrs.emplace_back(builder->create<ast::StrideAttribute>(stride));
            }
            return array(subtype, std::forward<EXPR>(n), std::move(attrs));
        }

        /// @param source the Source of the node
        /// @param subtype the array element type
        /// @param n the array size. nullptr represents a runtime-array
        /// @param stride the array stride. 0 represents implicit stride
        /// @return the tint AST type for a array of size `n` of type `T`
        template <typename EXPR>
        const ast::Array* array(const Source& source,
                                const ast::Type* subtype,
                                EXPR&& n,
                                uint32_t stride) const {
            ast::AttributeList attrs;
            if (stride) {
                attrs.emplace_back(builder->create<ast::StrideAttribute>(stride));
            }
            return array(source, subtype, std::forward<EXPR>(n), std::move(attrs));
        }

        /// @return the tint AST type for a runtime-sized array of type `T`
        template <typename T>
        const ast::Array* array() const {
            return array(Of<T>(), nullptr);
        }

        /// @return the tint AST type for an array of size `N` of type `T`
        template <typename T, int N>
        const ast::Array* array() const {
            return array(Of<T>(), builder->Expr(N));
        }

        /// @param stride the array stride
        /// @return the tint AST type for a runtime-sized array of type `T`
        template <typename T>
        const ast::Array* array(uint32_t stride) const {
            return array(Of<T>(), nullptr, stride);
        }

        /// @param stride the array stride
        /// @return the tint AST type for an array of size `N` of type `T`
        template <typename T, int N>
        const ast::Array* array(uint32_t stride) const {
            return array(Of<T>(), builder->Expr(N), stride);
        }

        /// Creates a type name
        /// @param name the name
        /// @returns the type name
        template <typename NAME>
        const ast::TypeName* type_name(NAME&& name) const {
            return builder->create<ast::TypeName>(builder->Sym(std::forward<NAME>(name)));
        }

        /// Creates a type name
        /// @param source the Source of the node
        /// @param name the name
        /// @returns the type name
        template <typename NAME>
        const ast::TypeName* type_name(const Source& source, NAME&& name) const {
            return builder->create<ast::TypeName>(source, builder->Sym(std::forward<NAME>(name)));
        }

        /// Creates an alias type
        /// @param name the alias name
        /// @param type the alias type
        /// @returns the alias pointer
        template <typename NAME>
        const ast::Alias* alias(NAME&& name, const ast::Type* type) const {
            auto sym = builder->Sym(std::forward<NAME>(name));
            return builder->create<ast::Alias>(sym, type);
        }

        /// Creates an alias type
        /// @param source the Source of the node
        /// @param name the alias name
        /// @param type the alias type
        /// @returns the alias pointer
        template <typename NAME>
        const ast::Alias* alias(const Source& source, NAME&& name, const ast::Type* type) const {
            auto sym = builder->Sym(std::forward<NAME>(name));
            return builder->create<ast::Alias>(source, sym, type);
        }

        /// @param type the type of the pointer
        /// @param storage_class the storage class of the pointer
        /// @param access the optional access control of the pointer
        /// @return the pointer to `type` with the given ast::StorageClass
        const ast::Pointer* pointer(const ast::Type* type,
                                    ast::StorageClass storage_class,
                                    ast::Access access = ast::Access::kUndefined) const {
            return builder->create<ast::Pointer>(type, storage_class, access);
        }

        /// @param source the Source of the node
        /// @param type the type of the pointer
        /// @param storage_class the storage class of the pointer
        /// @param access the optional access control of the pointer
        /// @return the pointer to `type` with the given ast::StorageClass
        const ast::Pointer* pointer(const Source& source,
                                    const ast::Type* type,
                                    ast::StorageClass storage_class,
                                    ast::Access access = ast::Access::kUndefined) const {
            return builder->create<ast::Pointer>(source, type, storage_class, access);
        }

        /// @param storage_class the storage class of the pointer
        /// @param access the optional access control of the pointer
        /// @return the pointer to type `T` with the given ast::StorageClass.
        template <typename T>
        const ast::Pointer* pointer(ast::StorageClass storage_class,
                                    ast::Access access = ast::Access::kUndefined) const {
            return pointer(Of<T>(), storage_class, access);
        }

        /// @param source the Source of the node
        /// @param type the type of the atomic
        /// @return the atomic to `type`
        const ast::Atomic* atomic(const Source& source, const ast::Type* type) const {
            return builder->create<ast::Atomic>(source, type);
        }

        /// @param type the type of the atomic
        /// @return the atomic to `type`
        const ast::Atomic* atomic(const ast::Type* type) const {
            return builder->create<ast::Atomic>(type);
        }

        /// @return the atomic to type `T`
        template <typename T>
        const ast::Atomic* atomic() const {
            return atomic(Of<T>());
        }

        /// @param kind the kind of sampler
        /// @returns the sampler
        const ast::Sampler* sampler(ast::SamplerKind kind) const {
            return builder->create<ast::Sampler>(kind);
        }

        /// @param source the Source of the node
        /// @param kind the kind of sampler
        /// @returns the sampler
        const ast::Sampler* sampler(const Source& source, ast::SamplerKind kind) const {
            return builder->create<ast::Sampler>(source, kind);
        }

        /// @param dims the dimensionality of the texture
        /// @returns the depth texture
        const ast::DepthTexture* depth_texture(ast::TextureDimension dims) const {
            return builder->create<ast::DepthTexture>(dims);
        }

        /// @param source the Source of the node
        /// @param dims the dimensionality of the texture
        /// @returns the depth texture
        const ast::DepthTexture* depth_texture(const Source& source,
                                               ast::TextureDimension dims) const {
            return builder->create<ast::DepthTexture>(source, dims);
        }

        /// @param dims the dimensionality of the texture
        /// @returns the multisampled depth texture
        const ast::DepthMultisampledTexture* depth_multisampled_texture(
            ast::TextureDimension dims) const {
            return builder->create<ast::DepthMultisampledTexture>(dims);
        }

        /// @param source the Source of the node
        /// @param dims the dimensionality of the texture
        /// @returns the multisampled depth texture
        const ast::DepthMultisampledTexture* depth_multisampled_texture(
            const Source& source,
            ast::TextureDimension dims) const {
            return builder->create<ast::DepthMultisampledTexture>(source, dims);
        }

        /// @param dims the dimensionality of the texture
        /// @param subtype the texture subtype.
        /// @returns the sampled texture
        const ast::SampledTexture* sampled_texture(ast::TextureDimension dims,
                                                   const ast::Type* subtype) const {
            return builder->create<ast::SampledTexture>(dims, subtype);
        }

        /// @param source the Source of the node
        /// @param dims the dimensionality of the texture
        /// @param subtype the texture subtype.
        /// @returns the sampled texture
        const ast::SampledTexture* sampled_texture(const Source& source,
                                                   ast::TextureDimension dims,
                                                   const ast::Type* subtype) const {
            return builder->create<ast::SampledTexture>(source, dims, subtype);
        }

        /// @param dims the dimensionality of the texture
        /// @param subtype the texture subtype.
        /// @returns the multisampled texture
        const ast::MultisampledTexture* multisampled_texture(ast::TextureDimension dims,
                                                             const ast::Type* subtype) const {
            return builder->create<ast::MultisampledTexture>(dims, subtype);
        }

        /// @param source the Source of the node
        /// @param dims the dimensionality of the texture
        /// @param subtype the texture subtype.
        /// @returns the multisampled texture
        const ast::MultisampledTexture* multisampled_texture(const Source& source,
                                                             ast::TextureDimension dims,
                                                             const ast::Type* subtype) const {
            return builder->create<ast::MultisampledTexture>(source, dims, subtype);
        }

        /// @param dims the dimensionality of the texture
        /// @param format the texel format of the texture
        /// @param access the access control of the texture
        /// @returns the storage texture
        const ast::StorageTexture* storage_texture(ast::TextureDimension dims,
                                                   ast::TexelFormat format,
                                                   ast::Access access) const {
            auto* subtype = ast::StorageTexture::SubtypeFor(format, *builder);
            return builder->create<ast::StorageTexture>(dims, format, subtype, access);
        }

        /// @param source the Source of the node
        /// @param dims the dimensionality of the texture
        /// @param format the texel format of the texture
        /// @param access the access control of the texture
        /// @returns the storage texture
        const ast::StorageTexture* storage_texture(const Source& source,
                                                   ast::TextureDimension dims,
                                                   ast::TexelFormat format,
                                                   ast::Access access) const {
            auto* subtype = ast::StorageTexture::SubtypeFor(format, *builder);
            return builder->create<ast::StorageTexture>(source, dims, format, subtype, access);
        }

        /// @returns the external texture
        const ast::ExternalTexture* external_texture() const {
            return builder->create<ast::ExternalTexture>();
        }

        /// @param source the Source of the node
        /// @returns the external texture
        const ast::ExternalTexture* external_texture(const Source& source) const {
            return builder->create<ast::ExternalTexture>(source);
        }

        /// Constructs a TypeName for the type declaration.
        /// @param type the type
        /// @return either type or a pointer to a new ast::TypeName
        const ast::TypeName* Of(const ast::TypeDecl* type) const;

        /// The ProgramBuilder
        ProgramBuilder* const builder;

      private:
        /// CToAST<T> is specialized for various `T` types and each specialization
        /// contains a single static `get()` method for obtaining the corresponding
        /// AST type for the C type `T`.
        /// `get()` has the signature:
        ///    `static const ast::Type* get(Types* t)`
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
    const ast::IdentifierExpression* Expr(std::nullptr_t) { return nullptr; }

    /// @param source the source information
    /// @param symbol the identifier symbol
    /// @return an ast::IdentifierExpression with the given symbol
    const ast::IdentifierExpression* Expr(const Source& source, Symbol symbol) {
        return create<ast::IdentifierExpression>(source, symbol);
    }

    /// @param symbol the identifier symbol
    /// @return an ast::IdentifierExpression with the given symbol
    const ast::IdentifierExpression* Expr(Symbol symbol) {
        return create<ast::IdentifierExpression>(symbol);
    }

    /// @param source the source information
    /// @param variable the AST variable
    /// @return an ast::IdentifierExpression with the variable's symbol
    const ast::IdentifierExpression* Expr(const Source& source, const ast::Variable* variable) {
        return create<ast::IdentifierExpression>(source, variable->symbol);
    }

    /// @param variable the AST variable
    /// @return an ast::IdentifierExpression with the variable's symbol
    const ast::IdentifierExpression* Expr(const ast::Variable* variable) {
        return create<ast::IdentifierExpression>(variable->symbol);
    }

    /// @param source the source information
    /// @param name the identifier name
    /// @return an ast::IdentifierExpression with the given name
    const ast::IdentifierExpression* Expr(const Source& source, const char* name) {
        return create<ast::IdentifierExpression>(source, Symbols().Register(name));
    }

    /// @param name the identifier name
    /// @return an ast::IdentifierExpression with the given name
    const ast::IdentifierExpression* Expr(const char* name) {
        return create<ast::IdentifierExpression>(Symbols().Register(name));
    }

    /// @param source the source information
    /// @param name the identifier name
    /// @return an ast::IdentifierExpression with the given name
    const ast::IdentifierExpression* Expr(const Source& source, const std::string& name) {
        return create<ast::IdentifierExpression>(source, Symbols().Register(name));
    }

    /// @param name the identifier name
    /// @return an ast::IdentifierExpression with the given name
    const ast::IdentifierExpression* Expr(const std::string& name) {
        return create<ast::IdentifierExpression>(Symbols().Register(name));
    }

    /// @param source the source information
    /// @param value the boolean value
    /// @return a Scalar constructor for the given value
    const ast::BoolLiteralExpression* Expr(const Source& source, bool value) {
        return create<ast::BoolLiteralExpression>(source, value);
    }

    /// @param value the boolean value
    /// @return a Scalar constructor for the given value
    const ast::BoolLiteralExpression* Expr(bool value) {
        return create<ast::BoolLiteralExpression>(value);
    }

    /// @param source the source information
    /// @param value the float value
    /// @return a Scalar constructor for the given value
    const ast::FloatLiteralExpression* Expr(const Source& source, f32 value) {
        return create<ast::FloatLiteralExpression>(source, value);
    }

    /// @param value the float value
    /// @return a Scalar constructor for the given value
    const ast::FloatLiteralExpression* Expr(f32 value) {
        return create<ast::FloatLiteralExpression>(value);
    }

    /// @param source the source information
    /// @param value the integer value
    /// @return a Scalar constructor for the given value
    const ast::SintLiteralExpression* Expr(const Source& source, i32 value) {
        return create<ast::SintLiteralExpression>(source, value);
    }

    /// @param value the integer value
    /// @return a Scalar constructor for the given value
    const ast::SintLiteralExpression* Expr(i32 value) {
        return create<ast::SintLiteralExpression>(value);
    }

    /// @param source the source information
    /// @param value the unsigned int value
    /// @return a Scalar constructor for the given value
    const ast::UintLiteralExpression* Expr(const Source& source, u32 value) {
        return create<ast::UintLiteralExpression>(source, value);
    }

    /// @param value the unsigned int value
    /// @return a Scalar constructor for the given value
    const ast::UintLiteralExpression* Expr(u32 value) {
        return create<ast::UintLiteralExpression>(value);
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

    /// @param args the arguments for the type constructor
    /// @return an `ast::CallExpression` of type `ty`, with the values
    /// of `args` converted to `ast::Expression`s using `Expr()`
    template <typename T, typename... ARGS>
    const ast::CallExpression* Construct(ARGS&&... args) {
        return Construct(ty.Of<T>(), std::forward<ARGS>(args)...);
    }

    /// @param type the type to construct
    /// @param args the arguments for the constructor
    /// @return an `ast::CallExpression` of `type` constructed with the
    /// values `args`.
    template <typename... ARGS>
    const ast::CallExpression* Construct(const ast::Type* type, ARGS&&... args) {
        return Construct(source_, type, std::forward<ARGS>(args)...);
    }

    /// @param source the source information
    /// @param type the type to construct
    /// @param args the arguments for the constructor
    /// @return an `ast::CallExpression` of `type` constructed with the
    /// values `args`.
    template <typename... ARGS>
    const ast::CallExpression* Construct(const Source& source,
                                         const ast::Type* type,
                                         ARGS&&... args) {
        return create<ast::CallExpression>(source, type, ExprList(std::forward<ARGS>(args)...));
    }

    /// @param expr the expression for the bitcast
    /// @return an `ast::BitcastExpression` of type `ty`, with the values of
    /// `expr` converted to `ast::Expression`s using `Expr()`
    template <typename T, typename EXPR>
    const ast::BitcastExpression* Bitcast(EXPR&& expr) {
        return Bitcast(ty.Of<T>(), std::forward<EXPR>(expr));
    }

    /// @param type the type to cast to
    /// @param expr the expression for the bitcast
    /// @return an `ast::BitcastExpression` of `type` constructed with the values
    /// `expr`.
    template <typename EXPR>
    const ast::BitcastExpression* Bitcast(const ast::Type* type, EXPR&& expr) {
        return create<ast::BitcastExpression>(type, Expr(std::forward<EXPR>(expr)));
    }

    /// @param source the source information
    /// @param type the type to cast to
    /// @param expr the expression for the bitcast
    /// @return an `ast::BitcastExpression` of `type` constructed with the values
    /// `expr`.
    template <typename EXPR>
    const ast::BitcastExpression* Bitcast(const Source& source,
                                          const ast::Type* type,
                                          EXPR&& expr) {
        return create<ast::BitcastExpression>(source, type, Expr(std::forward<EXPR>(expr)));
    }

    /// @param args the arguments for the vector constructor
    /// @param type the vector type
    /// @param size the vector size
    /// @return an `ast::CallExpression` of a `size`-element vector of
    /// type `type`, constructed with the values `args`.
    template <typename... ARGS>
    const ast::CallExpression* vec(const ast::Type* type, uint32_t size, ARGS&&... args) {
        return Construct(ty.vec(type, size), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the vector constructor
    /// @return an `ast::CallExpression` of a 2-element vector of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS>
    const ast::CallExpression* vec2(ARGS&&... args) {
        return Construct(ty.vec2<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the vector constructor
    /// @return an `ast::CallExpression` of a 3-element vector of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS>
    const ast::CallExpression* vec3(ARGS&&... args) {
        return Construct(ty.vec3<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the vector constructor
    /// @return an `ast::CallExpression` of a 4-element vector of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS>
    const ast::CallExpression* vec4(ARGS&&... args) {
        return Construct(ty.vec4<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the matrix constructor
    /// @return an `ast::CallExpression` of a 2x2 matrix of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS>
    const ast::CallExpression* mat2x2(ARGS&&... args) {
        return Construct(ty.mat2x2<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the matrix constructor
    /// @return an `ast::CallExpression` of a 2x3 matrix of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS>
    const ast::CallExpression* mat2x3(ARGS&&... args) {
        return Construct(ty.mat2x3<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the matrix constructor
    /// @return an `ast::CallExpression` of a 2x4 matrix of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS>
    const ast::CallExpression* mat2x4(ARGS&&... args) {
        return Construct(ty.mat2x4<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the matrix constructor
    /// @return an `ast::CallExpression` of a 3x2 matrix of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS>
    const ast::CallExpression* mat3x2(ARGS&&... args) {
        return Construct(ty.mat3x2<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the matrix constructor
    /// @return an `ast::CallExpression` of a 3x3 matrix of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS>
    const ast::CallExpression* mat3x3(ARGS&&... args) {
        return Construct(ty.mat3x3<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the matrix constructor
    /// @return an `ast::CallExpression` of a 3x4 matrix of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS>
    const ast::CallExpression* mat3x4(ARGS&&... args) {
        return Construct(ty.mat3x4<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the matrix constructor
    /// @return an `ast::CallExpression` of a 4x2 matrix of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS>
    const ast::CallExpression* mat4x2(ARGS&&... args) {
        return Construct(ty.mat4x2<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the matrix constructor
    /// @return an `ast::CallExpression` of a 4x3 matrix of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS>
    const ast::CallExpression* mat4x3(ARGS&&... args) {
        return Construct(ty.mat4x3<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the matrix constructor
    /// @return an `ast::CallExpression` of a 4x4 matrix of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS>
    const ast::CallExpression* mat4x4(ARGS&&... args) {
        return Construct(ty.mat4x4<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the array constructor
    /// @return an `ast::CallExpression` of an array with element type
    /// `T` and size `N`, constructed with the values `args`.
    template <typename T, int N, typename... ARGS>
    const ast::CallExpression* array(ARGS&&... args) {
        return Construct(ty.array<T, N>(), std::forward<ARGS>(args)...);
    }

    /// @param subtype the array element type
    /// @param n the array size. nullptr represents a runtime-array.
    /// @param args the arguments for the array constructor
    /// @return an `ast::CallExpression` of an array with element type
    /// `subtype`, constructed with the values `args`.
    template <typename EXPR, typename... ARGS>
    const ast::CallExpression* array(const ast::Type* subtype, EXPR&& n, ARGS&&... args) {
        return Construct(ty.array(subtype, std::forward<EXPR>(n)), std::forward<ARGS>(args)...);
    }

    /// @param name the variable name
    /// @param type the variable type
    /// @param optional the optional variable settings.
    /// Can be any of the following, in any order:
    ///   * ast::StorageClass   - specifies the variable storage class
    ///   * ast::Access         - specifies the variable's access control
    ///   * ast::Expression*    - specifies the variable's initializer expression
    ///   * ast::AttributeList - specifies the variable's attributes
    /// Note that repeated arguments of the same type will use the last argument's
    /// value.
    /// @returns a `ast::Variable` with the given name, type and additional
    /// options
    template <typename NAME, typename... OPTIONAL>
    const ast::Variable* Var(NAME&& name, const ast::Type* type, OPTIONAL&&... optional) {
        VarOptionals opts(std::forward<OPTIONAL>(optional)...);
        return create<ast::Variable>(Sym(std::forward<NAME>(name)), opts.storage, opts.access, type,
                                     false /* is_const */, false /* is_overridable */,
                                     opts.constructor, std::move(opts.attributes));
    }

    /// @param source the variable source
    /// @param name the variable name
    /// @param type the variable type
    /// @param optional the optional variable settings.
    /// Can be any of the following, in any order:
    ///   * ast::StorageClass   - specifies the variable storage class
    ///   * ast::Access         - specifies the variable's access control
    ///   * ast::Expression*    - specifies the variable's initializer expression
    ///   * ast::AttributeList - specifies the variable's attributes
    /// Note that repeated arguments of the same type will use the last argument's
    /// value.
    /// @returns a `ast::Variable` with the given name, storage and type
    template <typename NAME, typename... OPTIONAL>
    const ast::Variable* Var(const Source& source,
                             NAME&& name,
                             const ast::Type* type,
                             OPTIONAL&&... optional) {
        VarOptionals opts(std::forward<OPTIONAL>(optional)...);
        return create<ast::Variable>(source, Sym(std::forward<NAME>(name)), opts.storage,
                                     opts.access, type, false /* is_const */,
                                     false /* is_overridable */, opts.constructor,
                                     std::move(opts.attributes));
    }

    /// @param name the variable name
    /// @param type the variable type
    /// @param constructor constructor expression
    /// @param attributes optional variable attributes
    /// @returns an immutable `ast::Variable` with the given name and type
    template <typename NAME>
    const ast::Variable* Let(NAME&& name,
                             const ast::Type* type,
                             const ast::Expression* constructor,
                             ast::AttributeList attributes = {}) {
        return create<ast::Variable>(Sym(std::forward<NAME>(name)), ast::StorageClass::kNone,
                                     ast::Access::kUndefined, type, true /* is_const */,
                                     false /* is_overridable */, constructor, attributes);
    }

    /// @param source the variable source
    /// @param name the variable name
    /// @param type the variable type
    /// @param constructor constructor expression
    /// @param attributes optional variable attributes
    /// @returns an immutable `ast::Variable` with the given name and type
    template <typename NAME>
    const ast::Variable* Let(const Source& source,
                             NAME&& name,
                             const ast::Type* type,
                             const ast::Expression* constructor,
                             ast::AttributeList attributes = {}) {
        return create<ast::Variable>(source, Sym(std::forward<NAME>(name)),
                                     ast::StorageClass::kNone, ast::Access::kUndefined, type,
                                     true /* is_const */, false /* is_overridable */, constructor,
                                     attributes);
    }

    /// @param name the parameter name
    /// @param type the parameter type
    /// @param attributes optional parameter attributes
    /// @returns an immutable `ast::Variable` with the given name and type
    template <typename NAME>
    const ast::Variable* Param(NAME&& name,
                               const ast::Type* type,
                               ast::AttributeList attributes = {}) {
        return create<ast::Variable>(Sym(std::forward<NAME>(name)), ast::StorageClass::kNone,
                                     ast::Access::kUndefined, type, true /* is_const */,
                                     false /* is_overridable */, nullptr, attributes);
    }

    /// @param source the parameter source
    /// @param name the parameter name
    /// @param type the parameter type
    /// @param attributes optional parameter attributes
    /// @returns an immutable `ast::Variable` with the given name and type
    template <typename NAME>
    const ast::Variable* Param(const Source& source,
                               NAME&& name,
                               const ast::Type* type,
                               ast::AttributeList attributes = {}) {
        return create<ast::Variable>(source, Sym(std::forward<NAME>(name)),
                                     ast::StorageClass::kNone, ast::Access::kUndefined, type,
                                     true /* is_const */, false /* is_overridable */, nullptr,
                                     attributes);
    }

    /// @param name the variable name
    /// @param type the variable type
    /// @param optional the optional variable settings.
    /// Can be any of the following, in any order:
    ///   * ast::StorageClass   - specifies the variable storage class
    ///   * ast::Access         - specifies the variable's access control
    ///   * ast::Expression*    - specifies the variable's initializer expression
    ///   * ast::AttributeList - specifies the variable's attributes
    /// Note that repeated arguments of the same type will use the last argument's
    /// value.
    /// @returns a new `ast::Variable`, which is automatically registered as a
    /// global variable with the ast::Module.
    template <typename NAME, typename... OPTIONAL, typename = DisableIfSource<NAME>>
    const ast::Variable* Global(NAME&& name, const ast::Type* type, OPTIONAL&&... optional) {
        auto* var = Var(std::forward<NAME>(name), type, std::forward<OPTIONAL>(optional)...);
        AST().AddGlobalVariable(var);
        return var;
    }

    /// @param source the variable source
    /// @param name the variable name
    /// @param type the variable type
    /// @param optional the optional variable settings.
    /// Can be any of the following, in any order:
    ///   * ast::StorageClass   - specifies the variable storage class
    ///   * ast::Access         - specifies the variable's access control
    ///   * ast::Expression*    - specifies the variable's initializer expression
    ///   * ast::AttributeList - specifies the variable's attributes
    /// Note that repeated arguments of the same type will use the last argument's
    /// value.
    /// @returns a new `ast::Variable`, which is automatically registered as a
    /// global variable with the ast::Module.
    template <typename NAME, typename... OPTIONAL>
    const ast::Variable* Global(const Source& source,
                                NAME&& name,
                                const ast::Type* type,
                                OPTIONAL&&... optional) {
        auto* var =
            Var(source, std::forward<NAME>(name), type, std::forward<OPTIONAL>(optional)...);
        AST().AddGlobalVariable(var);
        return var;
    }

    /// @param name the variable name
    /// @param type the variable type
    /// @param constructor constructor expression
    /// @param attributes optional variable attributes
    /// @returns a const `ast::Variable` constructed by calling Var() with the
    /// arguments of `args`, which is automatically registered as a global
    /// variable with the ast::Module.
    template <typename NAME>
    const ast::Variable* GlobalConst(NAME&& name,
                                     const ast::Type* type,
                                     const ast::Expression* constructor,
                                     ast::AttributeList attributes = {}) {
        auto* var = Let(std::forward<NAME>(name), type, constructor, std::move(attributes));
        AST().AddGlobalVariable(var);
        return var;
    }

    /// @param source the variable source
    /// @param name the variable name
    /// @param type the variable type
    /// @param constructor constructor expression
    /// @param attributes optional variable attributes
    /// @returns a const `ast::Variable` constructed by calling Var() with the
    /// arguments of `args`, which is automatically registered as a global
    /// variable with the ast::Module.
    template <typename NAME>
    const ast::Variable* GlobalConst(const Source& source,
                                     NAME&& name,
                                     const ast::Type* type,
                                     const ast::Expression* constructor,
                                     ast::AttributeList attributes = {}) {
        auto* var = Let(source, std::forward<NAME>(name), type, constructor, std::move(attributes));
        AST().AddGlobalVariable(var);
        return var;
    }

    /// @param name the variable name
    /// @param type the variable type
    /// @param constructor optional constructor expression
    /// @param attributes optional variable attributes
    /// @returns an overridable const `ast::Variable` which is automatically
    /// registered as a global variable with the ast::Module.
    template <typename NAME>
    const ast::Variable* Override(NAME&& name,
                                  const ast::Type* type,
                                  const ast::Expression* constructor,
                                  ast::AttributeList attributes = {}) {
        auto* var =
            create<ast::Variable>(source_, Sym(std::forward<NAME>(name)), ast::StorageClass::kNone,
                                  ast::Access::kUndefined, type, true /* is_const */,
                                  true /* is_overridable */, constructor, std::move(attributes));
        AST().AddGlobalVariable(var);
        return var;
    }

    /// @param source the variable source
    /// @param name the variable name
    /// @param type the variable type
    /// @param constructor constructor expression
    /// @param attributes optional variable attributes
    /// @returns a const `ast::Variable` constructed by calling Var() with the
    /// arguments of `args`, which is automatically registered as a global
    /// variable with the ast::Module.
    template <typename NAME>
    const ast::Variable* Override(const Source& source,
                                  NAME&& name,
                                  const ast::Type* type,
                                  const ast::Expression* constructor,
                                  ast::AttributeList attributes = {}) {
        auto* var =
            create<ast::Variable>(source, Sym(std::forward<NAME>(name)), ast::StorageClass::kNone,
                                  ast::Access::kUndefined, type, true /* is_const */,
                                  true /* is_overridable */, constructor, std::move(attributes));
        AST().AddGlobalVariable(var);
        return var;
    }

    /// @param source the source information
    /// @param expr the expression to take the address of
    /// @return an ast::UnaryOpExpression that takes the address of `expr`
    template <typename EXPR>
    const ast::UnaryOpExpression* AddressOf(const Source& source, EXPR&& expr) {
        return create<ast::UnaryOpExpression>(source, ast::UnaryOp::kAddressOf,
                                              Expr(std::forward<EXPR>(expr)));
    }

    /// @param expr the expression to take the address of
    /// @return an ast::UnaryOpExpression that takes the address of `expr`
    template <typename EXPR>
    const ast::UnaryOpExpression* AddressOf(EXPR&& expr) {
        return create<ast::UnaryOpExpression>(ast::UnaryOp::kAddressOf,
                                              Expr(std::forward<EXPR>(expr)));
    }

    /// @param source the source information
    /// @param expr the expression to perform an indirection on
    /// @return an ast::UnaryOpExpression that dereferences the pointer `expr`
    template <typename EXPR>
    const ast::UnaryOpExpression* Deref(const Source& source, EXPR&& expr) {
        return create<ast::UnaryOpExpression>(source, ast::UnaryOp::kIndirection,
                                              Expr(std::forward<EXPR>(expr)));
    }

    /// @param expr the expression to perform an indirection on
    /// @return an ast::UnaryOpExpression that dereferences the pointer `expr`
    template <typename EXPR>
    const ast::UnaryOpExpression* Deref(EXPR&& expr) {
        return create<ast::UnaryOpExpression>(ast::UnaryOp::kIndirection,
                                              Expr(std::forward<EXPR>(expr)));
    }

    /// @param expr the expression to perform a unary not on
    /// @return an ast::UnaryOpExpression that is the unary not of the input
    /// expression
    template <typename EXPR>
    const ast::UnaryOpExpression* Not(EXPR&& expr) {
        return create<ast::UnaryOpExpression>(ast::UnaryOp::kNot, Expr(std::forward<EXPR>(expr)));
    }

    /// @param expr the expression to perform a unary complement on
    /// @return an ast::UnaryOpExpression that is the unary complement of the
    /// input expression
    template <typename EXPR>
    const ast::UnaryOpExpression* Complement(EXPR&& expr) {
        return create<ast::UnaryOpExpression>(ast::UnaryOp::kComplement,
                                              Expr(std::forward<EXPR>(expr)));
    }

    /// @param source the source information
    /// @param func the function name
    /// @param args the function call arguments
    /// @returns a `ast::CallExpression` to the function `func`, with the
    /// arguments of `args` converted to `ast::Expression`s using `Expr()`.
    template <typename NAME, typename... ARGS>
    const ast::CallExpression* Call(const Source& source, NAME&& func, ARGS&&... args) {
        return create<ast::CallExpression>(source, Expr(func),
                                           ExprList(std::forward<ARGS>(args)...));
    }

    /// @param func the function name
    /// @param args the function call arguments
    /// @returns a `ast::CallExpression` to the function `func`, with the
    /// arguments of `args` converted to `ast::Expression`s using `Expr()`.
    template <typename NAME, typename... ARGS, typename = DisableIfSource<NAME>>
    const ast::CallExpression* Call(NAME&& func, ARGS&&... args) {
        return create<ast::CallExpression>(Expr(func), ExprList(std::forward<ARGS>(args)...));
    }

    /// @param source the source information
    /// @param call the call expression to wrap in a call statement
    /// @returns a `ast::CallStatement` for the given call expression
    const ast::CallStatement* CallStmt(const Source& source, const ast::CallExpression* call) {
        return create<ast::CallStatement>(source, call);
    }

    /// @param call the call expression to wrap in a call statement
    /// @returns a `ast::CallStatement` for the given call expression
    const ast::CallStatement* CallStmt(const ast::CallExpression* call) {
        return create<ast::CallStatement>(call);
    }

    /// @param source the source information
    /// @returns a `ast::PhonyExpression`
    const ast::PhonyExpression* Phony(const Source& source) {
        return create<ast::PhonyExpression>(source);
    }

    /// @returns a `ast::PhonyExpression`
    const ast::PhonyExpression* Phony() { return create<ast::PhonyExpression>(); }

    /// @param expr the expression to ignore
    /// @returns a `ast::AssignmentStatement` that assigns 'expr' to the phony
    /// (underscore) variable.
    template <typename EXPR>
    const ast::AssignmentStatement* Ignore(EXPR&& expr) {
        return create<ast::AssignmentStatement>(Phony(), Expr(expr));
    }

    /// @param lhs the left hand argument to the addition operation
    /// @param rhs the right hand argument to the addition operation
    /// @returns a `ast::BinaryExpression` summing the arguments `lhs` and `rhs`
    template <typename LHS, typename RHS>
    const ast::BinaryExpression* Add(LHS&& lhs, RHS&& rhs) {
        return create<ast::BinaryExpression>(ast::BinaryOp::kAdd, Expr(std::forward<LHS>(lhs)),
                                             Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the and operation
    /// @param rhs the right hand argument to the and operation
    /// @returns a `ast::BinaryExpression` bitwise anding `lhs` and `rhs`
    template <typename LHS, typename RHS>
    const ast::BinaryExpression* And(LHS&& lhs, RHS&& rhs) {
        return create<ast::BinaryExpression>(ast::BinaryOp::kAnd, Expr(std::forward<LHS>(lhs)),
                                             Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the or operation
    /// @param rhs the right hand argument to the or operation
    /// @returns a `ast::BinaryExpression` bitwise or-ing `lhs` and `rhs`
    template <typename LHS, typename RHS>
    const ast::BinaryExpression* Or(LHS&& lhs, RHS&& rhs) {
        return create<ast::BinaryExpression>(ast::BinaryOp::kOr, Expr(std::forward<LHS>(lhs)),
                                             Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the subtraction operation
    /// @param rhs the right hand argument to the subtraction operation
    /// @returns a `ast::BinaryExpression` subtracting `rhs` from `lhs`
    template <typename LHS, typename RHS>
    const ast::BinaryExpression* Sub(LHS&& lhs, RHS&& rhs) {
        return create<ast::BinaryExpression>(ast::BinaryOp::kSubtract, Expr(std::forward<LHS>(lhs)),
                                             Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the multiplication operation
    /// @param rhs the right hand argument to the multiplication operation
    /// @returns a `ast::BinaryExpression` multiplying `rhs` from `lhs`
    template <typename LHS, typename RHS>
    const ast::BinaryExpression* Mul(LHS&& lhs, RHS&& rhs) {
        return create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, Expr(std::forward<LHS>(lhs)),
                                             Expr(std::forward<RHS>(rhs)));
    }

    /// @param source the source information
    /// @param lhs the left hand argument to the multiplication operation
    /// @param rhs the right hand argument to the multiplication operation
    /// @returns a `ast::BinaryExpression` multiplying `rhs` from `lhs`
    template <typename LHS, typename RHS>
    const ast::BinaryExpression* Mul(const Source& source, LHS&& lhs, RHS&& rhs) {
        return create<ast::BinaryExpression>(source, ast::BinaryOp::kMultiply,
                                             Expr(std::forward<LHS>(lhs)),
                                             Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the division operation
    /// @param rhs the right hand argument to the division operation
    /// @returns a `ast::BinaryExpression` dividing `lhs` by `rhs`
    template <typename LHS, typename RHS>
    const ast::BinaryExpression* Div(LHS&& lhs, RHS&& rhs) {
        return create<ast::BinaryExpression>(ast::BinaryOp::kDivide, Expr(std::forward<LHS>(lhs)),
                                             Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the modulo operation
    /// @param rhs the right hand argument to the modulo operation
    /// @returns a `ast::BinaryExpression` applying modulo of `lhs` by `rhs`
    template <typename LHS, typename RHS>
    const ast::BinaryExpression* Mod(LHS&& lhs, RHS&& rhs) {
        return create<ast::BinaryExpression>(ast::BinaryOp::kModulo, Expr(std::forward<LHS>(lhs)),
                                             Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the bit shift right operation
    /// @param rhs the right hand argument to the bit shift right operation
    /// @returns a `ast::BinaryExpression` bit shifting right `lhs` by `rhs`
    template <typename LHS, typename RHS>
    const ast::BinaryExpression* Shr(LHS&& lhs, RHS&& rhs) {
        return create<ast::BinaryExpression>(
            ast::BinaryOp::kShiftRight, Expr(std::forward<LHS>(lhs)), Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the bit shift left operation
    /// @param rhs the right hand argument to the bit shift left operation
    /// @returns a `ast::BinaryExpression` bit shifting left `lhs` by `rhs`
    template <typename LHS, typename RHS>
    const ast::BinaryExpression* Shl(LHS&& lhs, RHS&& rhs) {
        return create<ast::BinaryExpression>(
            ast::BinaryOp::kShiftLeft, Expr(std::forward<LHS>(lhs)), Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the xor operation
    /// @param rhs the right hand argument to the xor operation
    /// @returns a `ast::BinaryExpression` bitwise xor-ing `lhs` and `rhs`
    template <typename LHS, typename RHS>
    const ast::BinaryExpression* Xor(LHS&& lhs, RHS&& rhs) {
        return create<ast::BinaryExpression>(ast::BinaryOp::kXor, Expr(std::forward<LHS>(lhs)),
                                             Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the logical and operation
    /// @param rhs the right hand argument to the logical and operation
    /// @returns a `ast::BinaryExpression` of `lhs` && `rhs`
    template <typename LHS, typename RHS>
    const ast::BinaryExpression* LogicalAnd(LHS&& lhs, RHS&& rhs) {
        return create<ast::BinaryExpression>(
            ast::BinaryOp::kLogicalAnd, Expr(std::forward<LHS>(lhs)), Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the logical or operation
    /// @param rhs the right hand argument to the logical or operation
    /// @returns a `ast::BinaryExpression` of `lhs` || `rhs`
    template <typename LHS, typename RHS>
    const ast::BinaryExpression* LogicalOr(LHS&& lhs, RHS&& rhs) {
        return create<ast::BinaryExpression>(
            ast::BinaryOp::kLogicalOr, Expr(std::forward<LHS>(lhs)), Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the greater than operation
    /// @param rhs the right hand argument to the greater than operation
    /// @returns a `ast::BinaryExpression` of `lhs` > `rhs`
    template <typename LHS, typename RHS>
    const ast::BinaryExpression* GreaterThan(LHS&& lhs, RHS&& rhs) {
        return create<ast::BinaryExpression>(ast::BinaryOp::kGreaterThan,
                                             Expr(std::forward<LHS>(lhs)),
                                             Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the greater than or equal operation
    /// @param rhs the right hand argument to the greater than or equal operation
    /// @returns a `ast::BinaryExpression` of `lhs` >= `rhs`
    template <typename LHS, typename RHS>
    const ast::BinaryExpression* GreaterThanEqual(LHS&& lhs, RHS&& rhs) {
        return create<ast::BinaryExpression>(ast::BinaryOp::kGreaterThanEqual,
                                             Expr(std::forward<LHS>(lhs)),
                                             Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the less than operation
    /// @param rhs the right hand argument to the less than operation
    /// @returns a `ast::BinaryExpression` of `lhs` < `rhs`
    template <typename LHS, typename RHS>
    const ast::BinaryExpression* LessThan(LHS&& lhs, RHS&& rhs) {
        return create<ast::BinaryExpression>(ast::BinaryOp::kLessThan, Expr(std::forward<LHS>(lhs)),
                                             Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the less than or equal operation
    /// @param rhs the right hand argument to the less than or equal operation
    /// @returns a `ast::BinaryExpression` of `lhs` <= `rhs`
    template <typename LHS, typename RHS>
    const ast::BinaryExpression* LessThanEqual(LHS&& lhs, RHS&& rhs) {
        return create<ast::BinaryExpression>(ast::BinaryOp::kLessThanEqual,
                                             Expr(std::forward<LHS>(lhs)),
                                             Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the equal expression
    /// @param rhs the right hand argument to the equal expression
    /// @returns a `ast::BinaryExpression` comparing `lhs` equal to `rhs`
    template <typename LHS, typename RHS>
    const ast::BinaryExpression* Equal(LHS&& lhs, RHS&& rhs) {
        return create<ast::BinaryExpression>(ast::BinaryOp::kEqual, Expr(std::forward<LHS>(lhs)),
                                             Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the not-equal expression
    /// @param rhs the right hand argument to the not-equal expression
    /// @returns a `ast::BinaryExpression` comparing `lhs` equal to `rhs` for
    ///          disequality
    template <typename LHS, typename RHS>
    const ast::BinaryExpression* NotEqual(LHS&& lhs, RHS&& rhs) {
        return create<ast::BinaryExpression>(ast::BinaryOp::kNotEqual, Expr(std::forward<LHS>(lhs)),
                                             Expr(std::forward<RHS>(rhs)));
    }

    /// @param source the source information
    /// @param obj the object for the index accessor expression
    /// @param idx the index argument for the index accessor expression
    /// @returns a `ast::IndexAccessorExpression` that indexes `arr` with `idx`
    template <typename OBJ, typename IDX>
    const ast::IndexAccessorExpression* IndexAccessor(const Source& source, OBJ&& obj, IDX&& idx) {
        return create<ast::IndexAccessorExpression>(source, Expr(std::forward<OBJ>(obj)),
                                                    Expr(std::forward<IDX>(idx)));
    }

    /// @param obj the object for the index accessor expression
    /// @param idx the index argument for the index accessor expression
    /// @returns a `ast::IndexAccessorExpression` that indexes `arr` with `idx`
    template <typename OBJ, typename IDX>
    const ast::IndexAccessorExpression* IndexAccessor(OBJ&& obj, IDX&& idx) {
        return create<ast::IndexAccessorExpression>(Expr(std::forward<OBJ>(obj)),
                                                    Expr(std::forward<IDX>(idx)));
    }

    /// @param source the source information
    /// @param obj the object for the member accessor expression
    /// @param idx the index argument for the member accessor expression
    /// @returns a `ast::MemberAccessorExpression` that indexes `obj` with `idx`
    template <typename OBJ, typename IDX>
    const ast::MemberAccessorExpression* MemberAccessor(const Source& source,
                                                        OBJ&& obj,
                                                        IDX&& idx) {
        return create<ast::MemberAccessorExpression>(source, Expr(std::forward<OBJ>(obj)),
                                                     Expr(std::forward<IDX>(idx)));
    }

    /// @param obj the object for the member accessor expression
    /// @param idx the index argument for the member accessor expression
    /// @returns a `ast::MemberAccessorExpression` that indexes `obj` with `idx`
    template <typename OBJ, typename IDX>
    const ast::MemberAccessorExpression* MemberAccessor(OBJ&& obj, IDX&& idx) {
        return create<ast::MemberAccessorExpression>(Expr(std::forward<OBJ>(obj)),
                                                     Expr(std::forward<IDX>(idx)));
    }

    /// Creates a ast::StructMemberOffsetAttribute
    /// @param val the offset value
    /// @returns the offset attribute pointer
    const ast::StructMemberOffsetAttribute* MemberOffset(uint32_t val) {
        return create<ast::StructMemberOffsetAttribute>(source_, val);
    }

    /// Creates a ast::StructMemberSizeAttribute
    /// @param source the source information
    /// @param val the size value
    /// @returns the size attribute pointer
    const ast::StructMemberSizeAttribute* MemberSize(const Source& source, uint32_t val) {
        return create<ast::StructMemberSizeAttribute>(source, val);
    }

    /// Creates a ast::StructMemberSizeAttribute
    /// @param val the size value
    /// @returns the size attribute pointer
    const ast::StructMemberSizeAttribute* MemberSize(uint32_t val) {
        return create<ast::StructMemberSizeAttribute>(source_, val);
    }

    /// Creates a ast::StructMemberAlignAttribute
    /// @param source the source information
    /// @param val the align value
    /// @returns the align attribute pointer
    const ast::StructMemberAlignAttribute* MemberAlign(const Source& source, uint32_t val) {
        return create<ast::StructMemberAlignAttribute>(source, val);
    }

    /// Creates a ast::StructMemberAlignAttribute
    /// @param val the align value
    /// @returns the align attribute pointer
    const ast::StructMemberAlignAttribute* MemberAlign(uint32_t val) {
        return create<ast::StructMemberAlignAttribute>(source_, val);
    }

    /// Creates the ast::GroupAttribute
    /// @param value group attribute index
    /// @returns the group attribute pointer
    const ast::GroupAttribute* Group(uint32_t value) { return create<ast::GroupAttribute>(value); }

    /// Creates the ast::BindingAttribute
    /// @param value the binding index
    /// @returns the binding deocration pointer
    const ast::BindingAttribute* Binding(uint32_t value) {
        return create<ast::BindingAttribute>(value);
    }

    /// Convenience function to create both a ast::GroupAttribute and
    /// ast::BindingAttribute
    /// @param group the group index
    /// @param binding the binding index
    /// @returns a attribute list with both the group and binding attributes
    ast::AttributeList GroupAndBinding(uint32_t group, uint32_t binding) {
        return {Group(group), Binding(binding)};
    }

    /// Creates an ast::Function and registers it with the ast::Module.
    /// @param source the source information
    /// @param name the function name
    /// @param params the function parameters
    /// @param type the function return type
    /// @param body the function body
    /// @param attributes the optional function attributes
    /// @param return_type_attributes the optional function return type
    /// attributes
    /// @returns the function pointer
    template <typename NAME>
    const ast::Function* Func(const Source& source,
                              NAME&& name,
                              ast::VariableList params,
                              const ast::Type* type,
                              ast::StatementList body,
                              ast::AttributeList attributes = {},
                              ast::AttributeList return_type_attributes = {}) {
        auto* func = create<ast::Function>(source, Sym(std::forward<NAME>(name)), params, type,
                                           create<ast::BlockStatement>(body), attributes,
                                           return_type_attributes);
        AST().AddFunction(func);
        return func;
    }

    /// Creates an ast::Function and registers it with the ast::Module.
    /// @param name the function name
    /// @param params the function parameters
    /// @param type the function return type
    /// @param body the function body
    /// @param attributes the optional function attributes
    /// @param return_type_attributes the optional function return type
    /// attributes
    /// @returns the function pointer
    template <typename NAME>
    const ast::Function* Func(NAME&& name,
                              ast::VariableList params,
                              const ast::Type* type,
                              ast::StatementList body,
                              ast::AttributeList attributes = {},
                              ast::AttributeList return_type_attributes = {}) {
        auto* func = create<ast::Function>(Sym(std::forward<NAME>(name)), params, type,
                                           create<ast::BlockStatement>(body), attributes,
                                           return_type_attributes);
        AST().AddFunction(func);
        return func;
    }

    /// Creates an ast::BreakStatement
    /// @param source the source information
    /// @returns the break statement pointer
    const ast::BreakStatement* Break(const Source& source) {
        return create<ast::BreakStatement>(source);
    }

    /// Creates an ast::BreakStatement
    /// @returns the break statement pointer
    const ast::BreakStatement* Break() { return create<ast::BreakStatement>(); }

    /// Creates an ast::ContinueStatement
    /// @param source the source information
    /// @returns the continue statement pointer
    const ast::ContinueStatement* Continue(const Source& source) {
        return create<ast::ContinueStatement>(source);
    }

    /// Creates an ast::ContinueStatement
    /// @returns the continue statement pointer
    const ast::ContinueStatement* Continue() { return create<ast::ContinueStatement>(); }

    /// Creates an ast::ReturnStatement with no return value
    /// @param source the source information
    /// @returns the return statement pointer
    const ast::ReturnStatement* Return(const Source& source) {
        return create<ast::ReturnStatement>(source);
    }

    /// Creates an ast::ReturnStatement with no return value
    /// @returns the return statement pointer
    const ast::ReturnStatement* Return() { return create<ast::ReturnStatement>(); }

    /// Creates an ast::ReturnStatement with the given return value
    /// @param source the source information
    /// @param val the return value
    /// @returns the return statement pointer
    template <typename EXPR>
    const ast::ReturnStatement* Return(const Source& source, EXPR&& val) {
        return create<ast::ReturnStatement>(source, Expr(std::forward<EXPR>(val)));
    }

    /// Creates an ast::ReturnStatement with the given return value
    /// @param val the return value
    /// @returns the return statement pointer
    template <typename EXPR, typename = DisableIfSource<EXPR>>
    const ast::ReturnStatement* Return(EXPR&& val) {
        return create<ast::ReturnStatement>(Expr(std::forward<EXPR>(val)));
    }

    /// Creates an ast::DiscardStatement
    /// @param source the source information
    /// @returns the discard statement pointer
    const ast::DiscardStatement* Discard(const Source& source) {
        return create<ast::DiscardStatement>(source);
    }

    /// Creates an ast::DiscardStatement
    /// @returns the discard statement pointer
    const ast::DiscardStatement* Discard() { return create<ast::DiscardStatement>(); }

    /// Creates a ast::Alias registering it with the AST().TypeDecls().
    /// @param source the source information
    /// @param name the alias name
    /// @param type the alias target type
    /// @returns the alias type
    template <typename NAME>
    const ast::Alias* Alias(const Source& source, NAME&& name, const ast::Type* type) {
        auto* out = ty.alias(source, std::forward<NAME>(name), type);
        AST().AddTypeDecl(out);
        return out;
    }

    /// Creates a ast::Alias registering it with the AST().TypeDecls().
    /// @param name the alias name
    /// @param type the alias target type
    /// @returns the alias type
    template <typename NAME>
    const ast::Alias* Alias(NAME&& name, const ast::Type* type) {
        auto* out = ty.alias(std::forward<NAME>(name), type);
        AST().AddTypeDecl(out);
        return out;
    }

    /// Creates a ast::Struct registering it with the AST().TypeDecls().
    /// @param source the source information
    /// @param name the struct name
    /// @param members the struct members
    /// @returns the struct type
    template <typename NAME>
    const ast::Struct* Structure(const Source& source, NAME&& name, ast::StructMemberList members) {
        auto sym = Sym(std::forward<NAME>(name));
        auto* type = create<ast::Struct>(source, sym, std::move(members), ast::AttributeList{});
        AST().AddTypeDecl(type);
        return type;
    }

    /// Creates a ast::Struct registering it with the AST().TypeDecls().
    /// @param name the struct name
    /// @param members the struct members
    /// @returns the struct type
    template <typename NAME>
    const ast::Struct* Structure(NAME&& name, ast::StructMemberList members) {
        auto sym = Sym(std::forward<NAME>(name));
        auto* type = create<ast::Struct>(sym, std::move(members), ast::AttributeList{});
        AST().AddTypeDecl(type);
        return type;
    }

    /// Creates a ast::StructMember
    /// @param source the source information
    /// @param name the struct member name
    /// @param type the struct member type
    /// @param attributes the optional struct member attributes
    /// @returns the struct member pointer
    template <typename NAME>
    const ast::StructMember* Member(const Source& source,
                                    NAME&& name,
                                    const ast::Type* type,
                                    ast::AttributeList attributes = {}) {
        return create<ast::StructMember>(source, Sym(std::forward<NAME>(name)), type,
                                         std::move(attributes));
    }

    /// Creates a ast::StructMember
    /// @param name the struct member name
    /// @param type the struct member type
    /// @param attributes the optional struct member attributes
    /// @returns the struct member pointer
    template <typename NAME>
    const ast::StructMember* Member(NAME&& name,
                                    const ast::Type* type,
                                    ast::AttributeList attributes = {}) {
        return create<ast::StructMember>(source_, Sym(std::forward<NAME>(name)), type,
                                         std::move(attributes));
    }

    /// Creates a ast::StructMember with the given byte offset
    /// @param offset the offset to use in the StructMemberOffsetattribute
    /// @param name the struct member name
    /// @param type the struct member type
    /// @returns the struct member pointer
    template <typename NAME>
    const ast::StructMember* Member(uint32_t offset, NAME&& name, const ast::Type* type) {
        return create<ast::StructMember>(source_, Sym(std::forward<NAME>(name)), type,
                                         ast::AttributeList{
                                             create<ast::StructMemberOffsetAttribute>(offset),
                                         });
    }

    /// Creates a ast::BlockStatement with input statements
    /// @param source the source information for the block
    /// @param statements statements of block
    /// @returns the block statement pointer
    template <typename... Statements>
    const ast::BlockStatement* Block(const Source& source, Statements&&... statements) {
        return create<ast::BlockStatement>(
            source, ast::StatementList{std::forward<Statements>(statements)...});
    }

    /// Creates a ast::BlockStatement with input statements
    /// @param statements statements of block
    /// @returns the block statement pointer
    template <typename... STATEMENTS, typename = DisableIfSource<STATEMENTS...>>
    const ast::BlockStatement* Block(STATEMENTS&&... statements) {
        return create<ast::BlockStatement>(
            ast::StatementList{std::forward<STATEMENTS>(statements)...});
    }

    /// Creates a ast::IfStatement with input condition, body, and optional
    /// else statement
    /// @param source the source information for the if statement
    /// @param condition the if statement condition expression
    /// @param body the if statement body
    /// @param else_stmt optional else statement
    /// @returns the if statement pointer
    template <typename CONDITION>
    const ast::IfStatement* If(const Source& source,
                               CONDITION&& condition,
                               const ast::BlockStatement* body,
                               const ast::Statement* else_stmt = nullptr) {
        return create<ast::IfStatement>(source, Expr(std::forward<CONDITION>(condition)), body,
                                        else_stmt);
    }

    /// Creates a ast::IfStatement with input condition, body, and optional
    /// else statement
    /// @param condition the if statement condition expression
    /// @param body the if statement body
    /// @param else_stmt optional else statement
    /// @returns the if statement pointer
    template <typename CONDITION>
    const ast::IfStatement* If(CONDITION&& condition,
                               const ast::BlockStatement* body,
                               const ast::Statement* else_stmt = nullptr) {
        return create<ast::IfStatement>(Expr(std::forward<CONDITION>(condition)), body, else_stmt);
    }

    /// Creates a ast::AssignmentStatement with input lhs and rhs expressions
    /// @param source the source information
    /// @param lhs the left hand side expression initializer
    /// @param rhs the right hand side expression initializer
    /// @returns the assignment statement pointer
    template <typename LhsExpressionInit, typename RhsExpressionInit>
    const ast::AssignmentStatement* Assign(const Source& source,
                                           LhsExpressionInit&& lhs,
                                           RhsExpressionInit&& rhs) {
        return create<ast::AssignmentStatement>(source, Expr(std::forward<LhsExpressionInit>(lhs)),
                                                Expr(std::forward<RhsExpressionInit>(rhs)));
    }

    /// Creates a ast::AssignmentStatement with input lhs and rhs expressions
    /// @param lhs the left hand side expression initializer
    /// @param rhs the right hand side expression initializer
    /// @returns the assignment statement pointer
    template <typename LhsExpressionInit, typename RhsExpressionInit>
    const ast::AssignmentStatement* Assign(LhsExpressionInit&& lhs, RhsExpressionInit&& rhs) {
        return create<ast::AssignmentStatement>(Expr(std::forward<LhsExpressionInit>(lhs)),
                                                Expr(std::forward<RhsExpressionInit>(rhs)));
    }

    /// Creates a ast::CompoundAssignmentStatement with input lhs and rhs
    /// expressions, and a binary operator.
    /// @param source the source information
    /// @param lhs the left hand side expression initializer
    /// @param rhs the right hand side expression initializer
    /// @param op the binary operator
    /// @returns the compound assignment statement pointer
    template <typename LhsExpressionInit, typename RhsExpressionInit>
    const ast::CompoundAssignmentStatement* CompoundAssign(const Source& source,
                                                           LhsExpressionInit&& lhs,
                                                           RhsExpressionInit&& rhs,
                                                           ast::BinaryOp op) {
        return create<ast::CompoundAssignmentStatement>(
            source, Expr(std::forward<LhsExpressionInit>(lhs)),
            Expr(std::forward<RhsExpressionInit>(rhs)), op);
    }

    /// Creates a ast::CompoundAssignmentStatement with input lhs and rhs
    /// expressions, and a binary operator.
    /// @param lhs the left hand side expression initializer
    /// @param rhs the right hand side expression initializer
    /// @param op the binary operator
    /// @returns the compound assignment statement pointer
    template <typename LhsExpressionInit, typename RhsExpressionInit>
    const ast::CompoundAssignmentStatement* CompoundAssign(LhsExpressionInit&& lhs,
                                                           RhsExpressionInit&& rhs,
                                                           ast::BinaryOp op) {
        return create<ast::CompoundAssignmentStatement>(Expr(std::forward<LhsExpressionInit>(lhs)),
                                                        Expr(std::forward<RhsExpressionInit>(rhs)),
                                                        op);
    }

    /// Creates an ast::IncrementDecrementStatement with input lhs.
    /// @param source the source information
    /// @param lhs the left hand side expression initializer
    /// @returns the increment decrement statement pointer
    template <typename LhsExpressionInit>
    const ast::IncrementDecrementStatement* Increment(const Source& source,
                                                      LhsExpressionInit&& lhs) {
        return create<ast::IncrementDecrementStatement>(
            source, Expr(std::forward<LhsExpressionInit>(lhs)), true);
    }

    /// Creates a ast::IncrementDecrementStatement with input lhs.
    /// @param lhs the left hand side expression initializer
    /// @returns the increment decrement statement pointer
    template <typename LhsExpressionInit>
    const ast::IncrementDecrementStatement* Increment(LhsExpressionInit&& lhs) {
        return create<ast::IncrementDecrementStatement>(Expr(std::forward<LhsExpressionInit>(lhs)),
                                                        true);
    }

    /// Creates an ast::IncrementDecrementStatement with input lhs.
    /// @param source the source information
    /// @param lhs the left hand side expression initializer
    /// @returns the increment decrement statement pointer
    template <typename LhsExpressionInit>
    const ast::IncrementDecrementStatement* Decrement(const Source& source,
                                                      LhsExpressionInit&& lhs) {
        return create<ast::IncrementDecrementStatement>(
            source, Expr(std::forward<LhsExpressionInit>(lhs)), false);
    }

    /// Creates a ast::IncrementDecrementStatement with input lhs.
    /// @param lhs the left hand side expression initializer
    /// @returns the increment decrement statement pointer
    template <typename LhsExpressionInit>
    const ast::IncrementDecrementStatement* Decrement(LhsExpressionInit&& lhs) {
        return create<ast::IncrementDecrementStatement>(Expr(std::forward<LhsExpressionInit>(lhs)),
                                                        false);
    }

    /// Creates a ast::LoopStatement with input body and optional continuing
    /// @param source the source information
    /// @param body the loop body
    /// @param continuing the optional continuing block
    /// @returns the loop statement pointer
    const ast::LoopStatement* Loop(const Source& source,
                                   const ast::BlockStatement* body,
                                   const ast::BlockStatement* continuing = nullptr) {
        return create<ast::LoopStatement>(source, body, continuing);
    }

    /// Creates a ast::LoopStatement with input body and optional continuing
    /// @param body the loop body
    /// @param continuing the optional continuing block
    /// @returns the loop statement pointer
    const ast::LoopStatement* Loop(const ast::BlockStatement* body,
                                   const ast::BlockStatement* continuing = nullptr) {
        return create<ast::LoopStatement>(body, continuing);
    }

    /// Creates a ast::ForLoopStatement with input body and optional initializer,
    /// condition and continuing.
    /// @param source the source information
    /// @param init the optional loop initializer
    /// @param cond the optional loop condition
    /// @param cont the optional loop continuing
    /// @param body the loop body
    /// @returns the for loop statement pointer
    template <typename COND>
    const ast::ForLoopStatement* For(const Source& source,
                                     const ast::Statement* init,
                                     COND&& cond,
                                     const ast::Statement* cont,
                                     const ast::BlockStatement* body) {
        return create<ast::ForLoopStatement>(source, init, Expr(std::forward<COND>(cond)), cont,
                                             body);
    }

    /// Creates a ast::ForLoopStatement with input body and optional initializer,
    /// condition and continuing.
    /// @param init the optional loop initializer
    /// @param cond the optional loop condition
    /// @param cont the optional loop continuing
    /// @param body the loop body
    /// @returns the for loop statement pointer
    template <typename COND>
    const ast::ForLoopStatement* For(const ast::Statement* init,
                                     COND&& cond,
                                     const ast::Statement* cont,
                                     const ast::BlockStatement* body) {
        return create<ast::ForLoopStatement>(init, Expr(std::forward<COND>(cond)), cont, body);
    }

    /// Creates a ast::VariableDeclStatement for the input variable
    /// @param source the source information
    /// @param var the variable to wrap in a decl statement
    /// @returns the variable decl statement pointer
    const ast::VariableDeclStatement* Decl(const Source& source, const ast::Variable* var) {
        return create<ast::VariableDeclStatement>(source, var);
    }

    /// Creates a ast::VariableDeclStatement for the input variable
    /// @param var the variable to wrap in a decl statement
    /// @returns the variable decl statement pointer
    const ast::VariableDeclStatement* Decl(const ast::Variable* var) {
        return create<ast::VariableDeclStatement>(var);
    }

    /// Creates a ast::SwitchStatement with input expression and cases
    /// @param source the source information
    /// @param condition the condition expression initializer
    /// @param cases case statements
    /// @returns the switch statement pointer
    template <typename ExpressionInit, typename... Cases>
    const ast::SwitchStatement* Switch(const Source& source,
                                       ExpressionInit&& condition,
                                       Cases&&... cases) {
        return create<ast::SwitchStatement>(source, Expr(std::forward<ExpressionInit>(condition)),
                                            ast::CaseStatementList{std::forward<Cases>(cases)...});
    }

    /// Creates a ast::SwitchStatement with input expression and cases
    /// @param condition the condition expression initializer
    /// @param cases case statements
    /// @returns the switch statement pointer
    template <typename ExpressionInit,
              typename... Cases,
              typename = DisableIfSource<ExpressionInit>>
    const ast::SwitchStatement* Switch(ExpressionInit&& condition, Cases&&... cases) {
        return create<ast::SwitchStatement>(Expr(std::forward<ExpressionInit>(condition)),
                                            ast::CaseStatementList{std::forward<Cases>(cases)...});
    }

    /// Creates a ast::CaseStatement with input list of selectors, and body
    /// @param source the source information
    /// @param selectors list of selectors
    /// @param body the case body
    /// @returns the case statement pointer
    const ast::CaseStatement* Case(const Source& source,
                                   ast::CaseSelectorList selectors,
                                   const ast::BlockStatement* body = nullptr) {
        return create<ast::CaseStatement>(source, std::move(selectors), body ? body : Block());
    }

    /// Creates a ast::CaseStatement with input list of selectors, and body
    /// @param selectors list of selectors
    /// @param body the case body
    /// @returns the case statement pointer
    const ast::CaseStatement* Case(ast::CaseSelectorList selectors,
                                   const ast::BlockStatement* body = nullptr) {
        return create<ast::CaseStatement>(std::move(selectors), body ? body : Block());
    }

    /// Convenient overload that takes a single selector
    /// @param selector a single case selector
    /// @param body the case body
    /// @returns the case statement pointer
    const ast::CaseStatement* Case(const ast::IntLiteralExpression* selector,
                                   const ast::BlockStatement* body = nullptr) {
        return Case(ast::CaseSelectorList{selector}, body);
    }

    /// Convenience function that creates a 'default' ast::CaseStatement
    /// @param source the source information
    /// @param body the case body
    /// @returns the case statement pointer
    const ast::CaseStatement* DefaultCase(const Source& source,
                                          const ast::BlockStatement* body = nullptr) {
        return Case(source, ast::CaseSelectorList{}, body);
    }

    /// Convenience function that creates a 'default' ast::CaseStatement
    /// @param body the case body
    /// @returns the case statement pointer
    const ast::CaseStatement* DefaultCase(const ast::BlockStatement* body = nullptr) {
        return Case(ast::CaseSelectorList{}, body);
    }

    /// Creates an ast::FallthroughStatement
    /// @param source the source information
    /// @returns the fallthrough statement pointer
    const ast::FallthroughStatement* Fallthrough(const Source& source) {
        return create<ast::FallthroughStatement>(source);
    }

    /// Creates an ast::FallthroughStatement
    /// @returns the fallthrough statement pointer
    const ast::FallthroughStatement* Fallthrough() { return create<ast::FallthroughStatement>(); }

    /// Creates an ast::BuiltinAttribute
    /// @param source the source information
    /// @param builtin the builtin value
    /// @returns the builtin attribute pointer
    const ast::BuiltinAttribute* Builtin(const Source& source, ast::Builtin builtin) {
        return create<ast::BuiltinAttribute>(source, builtin);
    }

    /// Creates an ast::BuiltinAttribute
    /// @param builtin the builtin value
    /// @returns the builtin attribute pointer
    const ast::BuiltinAttribute* Builtin(ast::Builtin builtin) {
        return create<ast::BuiltinAttribute>(source_, builtin);
    }

    /// Creates an ast::InterpolateAttribute
    /// @param source the source information
    /// @param type the interpolation type
    /// @param sampling the interpolation sampling
    /// @returns the interpolate attribute pointer
    const ast::InterpolateAttribute* Interpolate(
        const Source& source,
        ast::InterpolationType type,
        ast::InterpolationSampling sampling = ast::InterpolationSampling::kNone) {
        return create<ast::InterpolateAttribute>(source, type, sampling);
    }

    /// Creates an ast::InterpolateAttribute
    /// @param type the interpolation type
    /// @param sampling the interpolation sampling
    /// @returns the interpolate attribute pointer
    const ast::InterpolateAttribute* Interpolate(
        ast::InterpolationType type,
        ast::InterpolationSampling sampling = ast::InterpolationSampling::kNone) {
        return create<ast::InterpolateAttribute>(source_, type, sampling);
    }

    /// Creates an ast::InterpolateAttribute using flat interpolation
    /// @param source the source information
    /// @returns the interpolate attribute pointer
    const ast::InterpolateAttribute* Flat(const Source& source) {
        return Interpolate(source, ast::InterpolationType::kFlat);
    }

    /// Creates an ast::InterpolateAttribute using flat interpolation
    /// @returns the interpolate attribute pointer
    const ast::InterpolateAttribute* Flat() { return Interpolate(ast::InterpolationType::kFlat); }

    /// Creates an ast::InvariantAttribute
    /// @param source the source information
    /// @returns the invariant attribute pointer
    const ast::InvariantAttribute* Invariant(const Source& source) {
        return create<ast::InvariantAttribute>(source);
    }

    /// Creates an ast::InvariantAttribute
    /// @returns the invariant attribute pointer
    const ast::InvariantAttribute* Invariant() { return create<ast::InvariantAttribute>(source_); }

    /// Creates an ast::LocationAttribute
    /// @param source the source information
    /// @param location the location value
    /// @returns the location attribute pointer
    const ast::LocationAttribute* Location(const Source& source, uint32_t location) {
        return create<ast::LocationAttribute>(source, location);
    }

    /// Creates an ast::LocationAttribute
    /// @param location the location value
    /// @returns the location attribute pointer
    const ast::LocationAttribute* Location(uint32_t location) {
        return create<ast::LocationAttribute>(source_, location);
    }

    /// Creates an ast::IdAttribute
    /// @param source the source information
    /// @param id the id value
    /// @returns the override attribute pointer
    const ast::IdAttribute* Id(const Source& source, uint32_t id) {
        return create<ast::IdAttribute>(source, id);
    }

    /// Creates an ast::IdAttribute with a constant ID
    /// @param id the optional id value
    /// @returns the override attribute pointer
    const ast::IdAttribute* Id(uint32_t id) { return Id(source_, id); }

    /// Creates an ast::StageAttribute
    /// @param source the source information
    /// @param stage the pipeline stage
    /// @returns the stage attribute pointer
    const ast::StageAttribute* Stage(const Source& source, ast::PipelineStage stage) {
        return create<ast::StageAttribute>(source, stage);
    }

    /// Creates an ast::StageAttribute
    /// @param stage the pipeline stage
    /// @returns the stage attribute pointer
    const ast::StageAttribute* Stage(ast::PipelineStage stage) {
        return create<ast::StageAttribute>(source_, stage);
    }

    /// Creates an ast::WorkgroupAttribute
    /// @param x the x dimension expression
    /// @returns the workgroup attribute pointer
    template <typename EXPR_X>
    const ast::WorkgroupAttribute* WorkgroupSize(EXPR_X&& x) {
        return WorkgroupSize(std::forward<EXPR_X>(x), nullptr, nullptr);
    }

    /// Creates an ast::WorkgroupAttribute
    /// @param x the x dimension expression
    /// @param y the y dimension expression
    /// @returns the workgroup attribute pointer
    template <typename EXPR_X, typename EXPR_Y>
    const ast::WorkgroupAttribute* WorkgroupSize(EXPR_X&& x, EXPR_Y&& y) {
        return WorkgroupSize(std::forward<EXPR_X>(x), std::forward<EXPR_Y>(y), nullptr);
    }

    /// Creates an ast::WorkgroupAttribute
    /// @param source the source information
    /// @param x the x dimension expression
    /// @param y the y dimension expression
    /// @param z the z dimension expression
    /// @returns the workgroup attribute pointer
    template <typename EXPR_X, typename EXPR_Y, typename EXPR_Z>
    const ast::WorkgroupAttribute* WorkgroupSize(const Source& source,
                                                 EXPR_X&& x,
                                                 EXPR_Y&& y,
                                                 EXPR_Z&& z) {
        return create<ast::WorkgroupAttribute>(source, Expr(std::forward<EXPR_X>(x)),
                                               Expr(std::forward<EXPR_Y>(y)),
                                               Expr(std::forward<EXPR_Z>(z)));
    }

    /// Creates an ast::WorkgroupAttribute
    /// @param x the x dimension expression
    /// @param y the y dimension expression
    /// @param z the z dimension expression
    /// @returns the workgroup attribute pointer
    template <typename EXPR_X, typename EXPR_Y, typename EXPR_Z>
    const ast::WorkgroupAttribute* WorkgroupSize(EXPR_X&& x, EXPR_Y&& y, EXPR_Z&& z) {
        return create<ast::WorkgroupAttribute>(source_, Expr(std::forward<EXPR_X>(x)),
                                               Expr(std::forward<EXPR_Y>(y)),
                                               Expr(std::forward<EXPR_Z>(z)));
    }

    /// Creates an ast::DisableValidationAttribute
    /// @param validation the validation to disable
    /// @returns the disable validation attribute pointer
    const ast::DisableValidationAttribute* Disable(ast::DisabledValidation validation) {
        return ASTNodes().Create<ast::DisableValidationAttribute>(ID(), validation);
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
    const sem::Type* TypeOf(const ast::Expression* expr) const;

    /// Helper for returning the resolved semantic type of the variable `var`.
    /// @note As the Resolver is run when the Program is built, this will only be
    /// useful for the Resolver itself and tests that use their own Resolver.
    /// @param var the AST variable
    /// @return the resolved semantic type for the variable, or nullptr if the
    /// variable has no resolved type.
    const sem::Type* TypeOf(const ast::Variable* var) const;

    /// Helper for returning the resolved semantic type of the AST type `type`.
    /// @note As the Resolver is run when the Program is built, this will only be
    /// useful for the Resolver itself and tests that use their own Resolver.
    /// @param type the AST type
    /// @return the resolved semantic type for the type, or nullptr if the type
    /// has no resolved type.
    const sem::Type* TypeOf(const ast::Type* type) const;

    /// Helper for returning the resolved semantic type of the AST type
    /// declaration `type_decl`.
    /// @note As the Resolver is run when the Program is built, this will only be
    /// useful for the Resolver itself and tests that use their own Resolver.
    /// @param type_decl the AST type declaration
    /// @return the resolved semantic type for the type declaration, or nullptr if
    /// the type declaration has no resolved type.
    const sem::Type* TypeOf(const ast::TypeDecl* type_decl) const;

    /// Wraps the ast::Expression in a statement. This is used by tests that
    /// construct a partial AST and require the Resolver to reach these
    /// nodes.
    /// @param expr the ast::Expression to be wrapped by an ast::Statement
    /// @return the ast::Statement that wraps the ast::Expression
    const ast::Statement* WrapInStatement(const ast::Expression* expr);
    /// Wraps the ast::Variable in a ast::VariableDeclStatement. This is used by
    /// tests that construct a partial AST and require the Resolver to reach
    /// these nodes.
    /// @param v the ast::Variable to be wrapped by an ast::VariableDeclStatement
    /// @return the ast::VariableDeclStatement that wraps the ast::Variable
    const ast::VariableDeclStatement* WrapInStatement(const ast::Variable* v);
    /// Returns the statement argument. Used as a passthrough-overload by
    /// WrapInFunction().
    /// @param stmt the ast::Statement
    /// @return `stmt`
    const ast::Statement* WrapInStatement(const ast::Statement* stmt);
    /// Wraps the list of arguments in a simple function so that each is reachable
    /// by the Resolver.
    /// @param args a mix of ast::Expression, ast::Statement, ast::Variables.
    /// @returns the function
    template <typename... ARGS>
    const ast::Function* WrapInFunction(ARGS&&... args) {
        ast::StatementList stmts{WrapInStatement(std::forward<ARGS>(args))...};
        return WrapInFunction(std::move(stmts));
    }
    /// @param stmts a list of ast::Statement that will be wrapped by a function,
    /// so that each statement is reachable by the Resolver.
    /// @returns the function
    const ast::Function* WrapInFunction(ast::StatementList stmts);

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
    static const ast::Type* get(const ProgramBuilder::TypesBuilder* t) { return t->i32(); }
};
template <>
struct ProgramBuilder::TypesBuilder::CToAST<ProgramBuilder::u32> {
    static const ast::Type* get(const ProgramBuilder::TypesBuilder* t) { return t->u32(); }
};
template <>
struct ProgramBuilder::TypesBuilder::CToAST<ProgramBuilder::f32> {
    static const ast::Type* get(const ProgramBuilder::TypesBuilder* t) { return t->f32(); }
};
template <>
struct ProgramBuilder::TypesBuilder::CToAST<bool> {
    static const ast::Type* get(const ProgramBuilder::TypesBuilder* t) { return t->bool_(); }
};
template <>
struct ProgramBuilder::TypesBuilder::CToAST<void> {
    static const ast::Type* get(const ProgramBuilder::TypesBuilder* t) { return t->void_(); }
};
//! @endcond

/// @param builder the ProgramBuilder
/// @returns the ProgramID of the ProgramBuilder
inline ProgramID ProgramIDOf(const ProgramBuilder* builder) {
    return builder->ID();
}

}  // namespace tint

#endif  // SRC_TINT_PROGRAM_BUILDER_H_
