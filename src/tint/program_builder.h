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

#include "tint/override_id.h"

#include "src/tint/ast/alias.h"
#include "src/tint/ast/array.h"
#include "src/tint/ast/assignment_statement.h"
#include "src/tint/ast/atomic.h"
#include "src/tint/ast/binary_expression.h"
#include "src/tint/ast/binding_attribute.h"
#include "src/tint/ast/bitcast_expression.h"
#include "src/tint/ast/bool.h"
#include "src/tint/ast/bool_literal_expression.h"
#include "src/tint/ast/break_if_statement.h"
#include "src/tint/ast/break_statement.h"
#include "src/tint/ast/call_expression.h"
#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/case_statement.h"
#include "src/tint/ast/compound_assignment_statement.h"
#include "src/tint/ast/const.h"
#include "src/tint/ast/continue_statement.h"
#include "src/tint/ast/depth_multisampled_texture.h"
#include "src/tint/ast/depth_texture.h"
#include "src/tint/ast/disable_validation_attribute.h"
#include "src/tint/ast/discard_statement.h"
#include "src/tint/ast/enable.h"
#include "src/tint/ast/extension.h"
#include "src/tint/ast/external_texture.h"
#include "src/tint/ast/f16.h"
#include "src/tint/ast/f32.h"
#include "src/tint/ast/float_literal_expression.h"
#include "src/tint/ast/for_loop_statement.h"
#include "src/tint/ast/i32.h"
#include "src/tint/ast/id_attribute.h"
#include "src/tint/ast/if_statement.h"
#include "src/tint/ast/increment_decrement_statement.h"
#include "src/tint/ast/index_accessor_expression.h"
#include "src/tint/ast/int_literal_expression.h"
#include "src/tint/ast/interpolate_attribute.h"
#include "src/tint/ast/invariant_attribute.h"
#include "src/tint/ast/let.h"
#include "src/tint/ast/loop_statement.h"
#include "src/tint/ast/matrix.h"
#include "src/tint/ast/member_accessor_expression.h"
#include "src/tint/ast/module.h"
#include "src/tint/ast/multisampled_texture.h"
#include "src/tint/ast/override.h"
#include "src/tint/ast/parameter.h"
#include "src/tint/ast/phony_expression.h"
#include "src/tint/ast/pointer.h"
#include "src/tint/ast/return_statement.h"
#include "src/tint/ast/sampled_texture.h"
#include "src/tint/ast/sampler.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/ast/static_assert.h"
#include "src/tint/ast/storage_texture.h"
#include "src/tint/ast/stride_attribute.h"
#include "src/tint/ast/struct_member_align_attribute.h"
#include "src/tint/ast/struct_member_offset_attribute.h"
#include "src/tint/ast/struct_member_size_attribute.h"
#include "src/tint/ast/switch_statement.h"
#include "src/tint/ast/type_name.h"
#include "src/tint/ast/u32.h"
#include "src/tint/ast/unary_op_expression.h"
#include "src/tint/ast/var.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/ast/vector.h"
#include "src/tint/ast/void.h"
#include "src/tint/ast/while_statement.h"
#include "src/tint/ast/workgroup_attribute.h"
#include "src/tint/number.h"
#include "src/tint/program.h"
#include "src/tint/program_id.h"
#include "src/tint/sem/array.h"
#include "src/tint/sem/bool.h"
#include "src/tint/sem/constant.h"
#include "src/tint/sem/depth_texture.h"
#include "src/tint/sem/external_texture.h"
#include "src/tint/sem/f16.h"
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

#ifdef CURRENTLY_IN_TINT_PUBLIC_HEADER
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

namespace detail {

/// IsVectorLike<T>::value is true if T is a utils::Vector or utils::VectorRef.
template <typename T>
struct IsVectorLike {
    /// Non-specialized form of IsVectorLike defaults to false
    static constexpr bool value = false;
};

/// IsVectorLike specialization for utils::Vector
template <typename T, size_t N>
struct IsVectorLike<utils::Vector<T, N>> {
    /// True for the IsVectorLike specialization of utils::Vector
    static constexpr bool value = true;
};

/// IsVectorLike specialization for utils::VectorRef
template <typename T>
struct IsVectorLike<utils::VectorRef<T>> {
    /// True for the IsVectorLike specialization of utils::VectorRef
    static constexpr bool value = true;
};
}  // namespace detail

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

    /// A helper used to disable overloads if the first type in `TYPES` is a utils::Vector,
    /// utils::VectorRef or utils::VectorRef.
    template <typename... TYPES>
    using DisableIfVectorLike = traits::EnableIf<
        !detail::IsVectorLike<traits::Decay<traits::NthTypeOf<0, TYPES..., void>>>::value>;

    /// VarOptions is a helper for accepting an arbitrary number of order independent options for
    /// constructing an ast::Var.
    struct VarOptions {
        template <typename... ARGS>
        explicit VarOptions(ARGS&&... args) {
            (Set(std::forward<ARGS>(args)), ...);
        }
        ~VarOptions();

        const ast::Type* type = nullptr;
        ast::AddressSpace address_space = ast::AddressSpace::kNone;
        ast::Access access = ast::Access::kUndefined;
        const ast::Expression* initializer = nullptr;
        utils::Vector<const ast::Attribute*, 4> attributes;

      private:
        void Set(const ast::Type* t) { type = t; }
        void Set(ast::AddressSpace addr_space) { address_space = addr_space; }
        void Set(ast::Access ac) { access = ac; }
        void Set(const ast::Expression* c) { initializer = c; }
        void Set(utils::VectorRef<const ast::Attribute*> l) { attributes = std::move(l); }
        void Set(const ast::Attribute* a) { attributes.Push(a); }
    };

    /// LetOptions is a helper for accepting an arbitrary number of order independent options for
    /// constructing an ast::Let.
    struct LetOptions {
        template <typename... ARGS>
        explicit LetOptions(ARGS&&... args) {
            static constexpr bool has_init =
                (traits::IsTypeOrDerived<std::remove_pointer_t<std::remove_reference_t<ARGS>>,
                                         ast::Expression> ||
                 ...);
            static_assert(has_init, "Let() must be constructed with an initializer expression");
            (Set(std::forward<ARGS>(args)), ...);
        }
        ~LetOptions();

        const ast::Type* type = nullptr;
        const ast::Expression* initializer = nullptr;
        utils::Vector<const ast::Attribute*, 4> attributes;

      private:
        void Set(const ast::Type* t) { type = t; }
        void Set(const ast::Expression* c) { initializer = c; }
        void Set(utils::VectorRef<const ast::Attribute*> l) { attributes = std::move(l); }
        void Set(const ast::Attribute* a) { attributes.Push(a); }
    };

    /// ConstOptions is a helper for accepting an arbitrary number of order independent options for
    /// constructing an ast::Const.
    struct ConstOptions {
        template <typename... ARGS>
        explicit ConstOptions(ARGS&&... args) {
            static constexpr bool has_init =
                (traits::IsTypeOrDerived<std::remove_pointer_t<std::remove_reference_t<ARGS>>,
                                         ast::Expression> ||
                 ...);
            static_assert(has_init, "Const() must be constructed with an initializer expression");
            (Set(std::forward<ARGS>(args)), ...);
        }
        ~ConstOptions();

        const ast::Type* type = nullptr;
        const ast::Expression* initializer = nullptr;
        utils::Vector<const ast::Attribute*, 4> attributes;

      private:
        void Set(const ast::Type* t) { type = t; }
        void Set(const ast::Expression* c) { initializer = c; }
        void Set(utils::VectorRef<const ast::Attribute*> l) { attributes = std::move(l); }
        void Set(const ast::Attribute* a) { attributes.Push(a); }
    };

    /// OverrideOptions is a helper for accepting an arbitrary number of order independent options
    /// for constructing an ast::Override.
    struct OverrideOptions {
        template <typename... ARGS>
        explicit OverrideOptions(ARGS&&... args) {
            (Set(std::forward<ARGS>(args)), ...);
        }
        ~OverrideOptions();

        const ast::Type* type = nullptr;
        const ast::Expression* initializer = nullptr;
        utils::Vector<const ast::Attribute*, 4> attributes;

      private:
        void Set(const ast::Type* t) { type = t; }
        void Set(const ast::Expression* c) { initializer = c; }
        void Set(utils::VectorRef<const ast::Attribute*> l) { attributes = std::move(l); }
        void Set(const ast::Attribute* a) { attributes.Push(a); }
    };

  public:
    /// ASTNodeAllocator is an alias to BlockAllocator<ast::Node>
    using ASTNodeAllocator = utils::BlockAllocator<ast::Node>;

    /// SemNodeAllocator is an alias to BlockAllocator<sem::Node>
    using SemNodeAllocator = utils::BlockAllocator<sem::Node>;

    /// ConstantAllocator is an alias to BlockAllocator<sem::Constant>
    using ConstantAllocator = utils::BlockAllocator<sem::Constant>;

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
    sem::TypeManager& Types() {
        AssertNotMoved();
        return types_;
    }

    /// @returns a reference to the program's types
    const sem::TypeManager& Types() const {
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

    /// @returns a reference to the program's semantic constant storage
    ConstantAllocator& ConstantNodes() {
        AssertNotMoved();
        return constant_nodes_;
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

    /// @returns the last allocated (numerically highest) AST node identifier.
    ast::NodeID LastAllocatedNodeID() const { return last_ast_node_id_; }

    /// @returns the next sequentially unique node identifier.
    ast::NodeID AllocateNodeID() {
        auto out = ast::NodeID{last_ast_node_id_.value + 1};
        last_ast_node_id_ = out;
        return out;
    }

    /// Creates a new ast::Node owned by the ProgramBuilder. When the
    /// ProgramBuilder is destructed, the ast::Node will also be destructed.
    /// @param source the Source of the node
    /// @param args the arguments to pass to the type constructor
    /// @returns the node pointer
    template <typename T, typename... ARGS>
    traits::EnableIfIsType<T, ast::Node>* create(const Source& source, ARGS&&... args) {
        AssertNotMoved();
        return ast_nodes_.Create<T>(id_, AllocateNodeID(), source, std::forward<ARGS>(args)...);
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
        return ast_nodes_.Create<T>(id_, AllocateNodeID(), source_);
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
        return ast_nodes_.Create<T>(id_, AllocateNodeID(), source_, std::forward<ARG0>(arg0),
                                    std::forward<ARGS>(args)...);
    }

    /// Creates a new sem::Node owned by the ProgramBuilder.
    /// When the ProgramBuilder is destructed, the sem::Node will also be destructed.
    /// @param args the arguments to pass to the constructor
    /// @returns the node pointer
    template <typename T, typename... ARGS>
    traits::EnableIf<traits::IsTypeOrDerived<T, sem::Node> &&
                         !traits::IsTypeOrDerived<T, sem::Type>,
                     T>*
    create(ARGS&&... args) {
        AssertNotMoved();
        return sem_nodes_.Create<T>(std::forward<ARGS>(args)...);
    }

    /// Creates a new sem::Constant owned by the ProgramBuilder.
    /// When the ProgramBuilder is destructed, the sem::Node will also be destructed.
    /// @param args the arguments to pass to the constructor
    /// @returns the node pointer
    template <typename T, typename... ARGS>
    traits::EnableIf<traits::IsTypeOrDerived<T, sem::Constant>, T>* create(ARGS&&... args) {
        AssertNotMoved();
        return constant_nodes_.Create<T>(std::forward<ARGS>(args)...);
    }

    /// Creates a new sem::Type owned by the ProgramBuilder.
    /// When the ProgramBuilder is destructed, owned ProgramBuilder and the
    /// returned`Type` will also be destructed.
    /// Types are unique (de-aliased), and so calling create() for the same `T`
    /// and arguments will return the same pointer.
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

        /// @returns a f16 type
        const ast::F16* f16() const { return builder->create<ast::F16>(); }

        /// @param source the Source of the node
        /// @returns a f16 type
        const ast::F16* f16(const Source& source) const {
            return builder->create<ast::F16>(source);
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

        /// @param source the vector source
        /// @param type vector subtype
        /// @return the tint AST type for a 2-element vector of `type`.
        const ast::Vector* vec2(const Source& source, const ast::Type* type) const {
            return vec(source, type, 2u);
        }

        /// @param type vector subtype
        /// @return the tint AST type for a 3-element vector of `type`.
        const ast::Vector* vec3(const ast::Type* type) const { return vec(type, 3u); }

        /// @param source the vector source
        /// @param type vector subtype
        /// @return the tint AST type for a 3-element vector of `type`.
        const ast::Vector* vec3(const Source& source, const ast::Type* type) const {
            return vec(source, type, 3u);
        }

        /// @param type vector subtype
        /// @return the tint AST type for a 4-element vector of `type`.
        const ast::Vector* vec4(const ast::Type* type) const { return vec(type, 4u); }

        /// @param source the vector source
        /// @param type vector subtype
        /// @return the tint AST type for a 4-element vector of `type`.
        const ast::Vector* vec4(const Source& source, const ast::Type* type) const {
            return vec(source, type, 4u);
        }

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

        /// @param source the Source of the node
        /// @return the tint AST type for a 2-element vector of the C type `T`.
        template <typename T>
        const ast::Vector* vec2(const Source& source) const {
            return vec2(source, Of<T>());
        }

        /// @return the tint AST type for a 3-element vector of the C type `T`.
        template <typename T>
        const ast::Vector* vec3() const {
            return vec3(Of<T>());
        }

        /// @param source the Source of the node
        /// @return the tint AST type for a 3-element vector of the C type `T`.
        template <typename T>
        const ast::Vector* vec3(const Source& source) const {
            return vec3(source, Of<T>());
        }

        /// @return the tint AST type for a 4-element vector of the C type `T`.
        template <typename T>
        const ast::Vector* vec4() const {
            return vec4(Of<T>());
        }

        /// @param source the Source of the node
        /// @return the tint AST type for a 4-element vector of the C type `T`.
        template <typename T>
        const ast::Vector* vec4(const Source& source) const {
            return vec4(source, Of<T>());
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
        const ast::Array* array(
            const ast::Type* subtype,
            EXPR&& n = nullptr,
            utils::VectorRef<const ast::Attribute*> attrs = utils::Empty) const {
            return builder->create<ast::Array>(subtype, builder->Expr(std::forward<EXPR>(n)),
                                               std::move(attrs));
        }

        /// @param source the Source of the node
        /// @param subtype the array element type
        /// @param n the array size. nullptr represents a runtime-array
        /// @param attrs the optional attributes for the array
        /// @return the tint AST type for a array of size `n` of type `T`
        template <typename EXPR = ast::Expression*>
        const ast::Array* array(
            const Source& source,
            const ast::Type* subtype,
            EXPR&& n = nullptr,
            utils::VectorRef<const ast::Attribute*> attrs = utils::Empty) const {
            return builder->create<ast::Array>(
                source, subtype, builder->Expr(std::forward<EXPR>(n)), std::move(attrs));
        }

        /// @param subtype the array element type
        /// @param n the array size. nullptr represents a runtime-array
        /// @param stride the array stride. 0 represents implicit stride
        /// @return the tint AST type for a array of size `n` of type `T`
        template <typename EXPR>
        const ast::Array* array(const ast::Type* subtype, EXPR&& n, uint32_t stride) const {
            utils::Vector<const ast::Attribute*, 2> attrs;
            if (stride) {
                attrs.Push(builder->create<ast::StrideAttribute>(stride));
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
            utils::Vector<const ast::Attribute*, 2> attrs;
            if (stride) {
                attrs.Push(builder->create<ast::StrideAttribute>(stride));
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
            return array(Of<T>(), builder->Expr(tint::u32(N)));
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
            return array(Of<T>(), builder->Expr(tint::u32(N)), stride);
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
        /// @param address_space the address space of the pointer
        /// @param access the optional access control of the pointer
        /// @return the pointer to `type` with the given ast::AddressSpace
        const ast::Pointer* pointer(const ast::Type* type,
                                    ast::AddressSpace address_space,
                                    ast::Access access = ast::Access::kUndefined) const {
            return builder->create<ast::Pointer>(type, address_space, access);
        }

        /// @param source the Source of the node
        /// @param type the type of the pointer
        /// @param address_space the address space of the pointer
        /// @param access the optional access control of the pointer
        /// @return the pointer to `type` with the given ast::AddressSpace
        const ast::Pointer* pointer(const Source& source,
                                    const ast::Type* type,
                                    ast::AddressSpace address_space,
                                    ast::Access access = ast::Access::kUndefined) const {
            return builder->create<ast::Pointer>(source, type, address_space, access);
        }

        /// @param address_space the address space of the pointer
        /// @param access the optional access control of the pointer
        /// @return the pointer to type `T` with the given ast::AddressSpace.
        template <typename T>
        const ast::Pointer* pointer(ast::AddressSpace address_space,
                                    ast::Access access = ast::Access::kUndefined) const {
            return pointer(Of<T>(), address_space, access);
        }

        /// @param source the Source of the node
        /// @param address_space the address space of the pointer
        /// @param access the optional access control of the pointer
        /// @return the pointer to type `T` with the given ast::AddressSpace.
        template <typename T>
        const ast::Pointer* pointer(const Source& source,
                                    ast::AddressSpace address_space,
                                    ast::Access access = ast::Access::kUndefined) const {
            return pointer(source, Of<T>(), address_space, access);
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
    template <typename BOOL>
    std::enable_if_t<std::is_same_v<BOOL, bool>, const ast::BoolLiteralExpression*> Expr(
        const Source& source,
        BOOL value) {
        return create<ast::BoolLiteralExpression>(source, value);
    }

    /// @param value the boolean value
    /// @return a Scalar constructor for the given value
    template <typename BOOL>
    std::enable_if_t<std::is_same_v<BOOL, bool>, const ast::BoolLiteralExpression*> Expr(
        BOOL value) {
        return create<ast::BoolLiteralExpression>(value);
    }

    /// @param source the source information
    /// @param value the float value
    /// @return a 'f'-suffixed FloatLiteralExpression for the f32 value
    const ast::FloatLiteralExpression* Expr(const Source& source, f32 value) {
        return create<ast::FloatLiteralExpression>(source, static_cast<double>(value.value),
                                                   ast::FloatLiteralExpression::Suffix::kF);
    }

    /// @param value the float value
    /// @return a 'f'-suffixed FloatLiteralExpression for the f32 value
    const ast::FloatLiteralExpression* Expr(f32 value) {
        return create<ast::FloatLiteralExpression>(static_cast<double>(value.value),
                                                   ast::FloatLiteralExpression::Suffix::kF);
    }

    /// @param source the source information
    /// @param value the float value
    /// @return a 'h'-suffixed FloatLiteralExpression for the f16 value
    const ast::FloatLiteralExpression* Expr(const Source& source, f16 value) {
        return create<ast::FloatLiteralExpression>(source, static_cast<double>(value.value),
                                                   ast::FloatLiteralExpression::Suffix::kH);
    }

    /// @param value the float value
    /// @return a 'h'-suffixed FloatLiteralExpression for the f16 value
    const ast::FloatLiteralExpression* Expr(f16 value) {
        return create<ast::FloatLiteralExpression>(static_cast<double>(value.value),
                                                   ast::FloatLiteralExpression::Suffix::kH);
    }

    /// @param source the source information
    /// @param value the integer value
    /// @return an unsuffixed IntLiteralExpression for the AInt value
    const ast::IntLiteralExpression* Expr(const Source& source, AInt value) {
        return create<ast::IntLiteralExpression>(source, value,
                                                 ast::IntLiteralExpression::Suffix::kNone);
    }

    /// @param value the integer value
    /// @return an unsuffixed IntLiteralExpression for the AInt value
    const ast::IntLiteralExpression* Expr(AInt value) {
        return create<ast::IntLiteralExpression>(value, ast::IntLiteralExpression::Suffix::kNone);
    }

    /// @param source the source information
    /// @param value the integer value
    /// @return an unsuffixed FloatLiteralExpression for the AFloat value
    const ast::FloatLiteralExpression* Expr(const Source& source, AFloat value) {
        return create<ast::FloatLiteralExpression>(source, value.value,
                                                   ast::FloatLiteralExpression::Suffix::kNone);
    }

    /// @param value the integer value
    /// @return an unsuffixed FloatLiteralExpression for the AFloat value
    const ast::FloatLiteralExpression* Expr(AFloat value) {
        return create<ast::FloatLiteralExpression>(value.value,
                                                   ast::FloatLiteralExpression::Suffix::kNone);
    }

    /// @param source the source information
    /// @param value the integer value
    /// @return a signed 'i'-suffixed IntLiteralExpression for the i32 value
    const ast::IntLiteralExpression* Expr(const Source& source, i32 value) {
        return create<ast::IntLiteralExpression>(source, value,
                                                 ast::IntLiteralExpression::Suffix::kI);
    }

    /// @param value the integer value
    /// @return a signed 'i'-suffixed IntLiteralExpression for the i32 value
    const ast::IntLiteralExpression* Expr(i32 value) {
        return create<ast::IntLiteralExpression>(value, ast::IntLiteralExpression::Suffix::kI);
    }

    /// @param source the source information
    /// @param value the unsigned int value
    /// @return an unsigned 'u'-suffixed IntLiteralExpression for the u32 value
    const ast::IntLiteralExpression* Expr(const Source& source, u32 value) {
        return create<ast::IntLiteralExpression>(source, value,
                                                 ast::IntLiteralExpression::Suffix::kU);
    }

    /// @param value the unsigned int value
    /// @return an unsigned 'u'-suffixed IntLiteralExpression for the u32 value
    const ast::IntLiteralExpression* Expr(u32 value) {
        return create<ast::IntLiteralExpression>(value, ast::IntLiteralExpression::Suffix::kU);
    }

    /// Converts `arg` to an `ast::Expression` using `Expr()`, then appends it to
    /// `list`.
    /// @param list the list to append too
    /// @param arg the arg to create
    template <size_t N, typename ARG>
    void Append(utils::Vector<const ast::Expression*, N>& list, ARG&& arg) {
        list.Push(Expr(std::forward<ARG>(arg)));
    }

    /// Converts `arg0` and `args` to `ast::Expression`s using `Expr()`,
    /// then appends them to `list`.
    /// @param list the list to append too
    /// @param arg0 the first argument
    /// @param args the rest of the arguments
    template <size_t N, typename ARG0, typename... ARGS>
    void Append(utils::Vector<const ast::Expression*, N>& list, ARG0&& arg0, ARGS&&... args) {
        Append(list, std::forward<ARG0>(arg0));
        Append(list, std::forward<ARGS>(args)...);
    }

    /// @return utils::EmptyType
    utils::EmptyType ExprList() { return utils::Empty; }

    /// @param args the list of expressions
    /// @return the list of expressions converted to `ast::Expression`s using
    /// `Expr()`,
    template <typename... ARGS, typename = DisableIfVectorLike<ARGS...>>
    auto ExprList(ARGS&&... args) {
        utils::Vector<const ast::Expression*, sizeof...(ARGS)> list;
        Append(list, std::forward<ARGS>(args)...);
        return list;
    }

    /// @param list the list of expressions
    /// @return `list`
    template <typename T, size_t N>
    utils::Vector<T, N> ExprList(utils::Vector<T, N>&& list) {
        return std::move(list);
    }

    /// @param list the list of expressions
    /// @return `list`
    utils::VectorRef<const ast::Expression*> ExprList(
        utils::VectorRef<const ast::Expression*> list) {
        return list;
    }

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

    /// @param args the arguments for the vector initializer
    /// @param type the vector type
    /// @param size the vector size
    /// @return an `ast::CallExpression` of a `size`-element vector of
    /// type `type`, constructed with the values `args`.
    template <typename... ARGS>
    const ast::CallExpression* vec(const ast::Type* type, uint32_t size, ARGS&&... args) {
        return Construct(ty.vec(type, size), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the vector initializer
    /// @return an `ast::CallExpression` of a 2-element vector of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS, typename _ = DisableIfSource<ARGS...>>
    const ast::CallExpression* vec2(ARGS&&... args) {
        return Construct(ty.vec2<T>(), std::forward<ARGS>(args)...);
    }

    /// @param source the vector source
    /// @param args the arguments for the vector initializer
    /// @return an `ast::CallExpression` of a 2-element vector of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS>
    const ast::CallExpression* vec2(const Source& source, ARGS&&... args) {
        return Construct(source, ty.vec2<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the vector initializer
    /// @return an `ast::CallExpression` of a 3-element vector of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS, typename _ = DisableIfSource<ARGS...>>
    const ast::CallExpression* vec3(ARGS&&... args) {
        return Construct(ty.vec3<T>(), std::forward<ARGS>(args)...);
    }

    /// @param source the vector source
    /// @param args the arguments for the vector initializer
    /// @return an `ast::CallExpression` of a 3-element vector of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS>
    const ast::CallExpression* vec3(const Source& source, ARGS&&... args) {
        return Construct(source, ty.vec3<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the vector initializer
    /// @return an `ast::CallExpression` of a 4-element vector of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS, typename _ = DisableIfSource<ARGS...>>
    const ast::CallExpression* vec4(ARGS&&... args) {
        return Construct(ty.vec4<T>(), std::forward<ARGS>(args)...);
    }

    /// @param source the vector source
    /// @param args the arguments for the vector initializer
    /// @return an `ast::CallExpression` of a 4-element vector of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS>
    const ast::CallExpression* vec4(const Source& source, ARGS&&... args) {
        return Construct(source, ty.vec4<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the matrix initializer
    /// @return an `ast::CallExpression` of a 2x2 matrix of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS, typename _ = DisableIfSource<ARGS...>>
    const ast::CallExpression* mat2x2(ARGS&&... args) {
        return Construct(ty.mat2x2<T>(), std::forward<ARGS>(args)...);
    }

    /// @param source the matrix source
    /// @param args the arguments for the matrix initializer
    /// @return an `ast::CallExpression` of a 2x2 matrix of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS>
    const ast::CallExpression* mat2x2(const Source& source, ARGS&&... args) {
        return Construct(source, ty.mat2x2<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the matrix initializer
    /// @return an `ast::CallExpression` of a 2x3 matrix of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS, typename _ = DisableIfSource<ARGS...>>
    const ast::CallExpression* mat2x3(ARGS&&... args) {
        return Construct(ty.mat2x3<T>(), std::forward<ARGS>(args)...);
    }

    /// @param source the matrix source
    /// @param args the arguments for the matrix initializer
    /// @return an `ast::CallExpression` of a 2x3 matrix of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS>
    const ast::CallExpression* mat2x3(const Source& source, ARGS&&... args) {
        return Construct(source, ty.mat2x3<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the matrix initializer
    /// @return an `ast::CallExpression` of a 2x4 matrix of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS, typename _ = DisableIfSource<ARGS...>>
    const ast::CallExpression* mat2x4(ARGS&&... args) {
        return Construct(ty.mat2x4<T>(), std::forward<ARGS>(args)...);
    }

    /// @param source the matrix source
    /// @param args the arguments for the matrix initializer
    /// @return an `ast::CallExpression` of a 2x4 matrix of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS>
    const ast::CallExpression* mat2x4(const Source& source, ARGS&&... args) {
        return Construct(source, ty.mat2x4<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the matrix initializer
    /// @return an `ast::CallExpression` of a 3x2 matrix of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS, typename _ = DisableIfSource<ARGS...>>
    const ast::CallExpression* mat3x2(ARGS&&... args) {
        return Construct(ty.mat3x2<T>(), std::forward<ARGS>(args)...);
    }

    /// @param source the matrix source
    /// @param args the arguments for the matrix initializer
    /// @return an `ast::CallExpression` of a 3x2 matrix of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS>
    const ast::CallExpression* mat3x2(const Source& source, ARGS&&... args) {
        return Construct(source, ty.mat3x2<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the matrix initializer
    /// @return an `ast::CallExpression` of a 3x3 matrix of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS, typename _ = DisableIfSource<ARGS...>>
    const ast::CallExpression* mat3x3(ARGS&&... args) {
        return Construct(ty.mat3x3<T>(), std::forward<ARGS>(args)...);
    }

    /// @param source the matrix source
    /// @param args the arguments for the matrix initializer
    /// @return an `ast::CallExpression` of a 3x3 matrix of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS>
    const ast::CallExpression* mat3x3(const Source& source, ARGS&&... args) {
        return Construct(source, ty.mat3x3<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the matrix initializer
    /// @return an `ast::CallExpression` of a 3x4 matrix of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS, typename _ = DisableIfSource<ARGS...>>
    const ast::CallExpression* mat3x4(ARGS&&... args) {
        return Construct(ty.mat3x4<T>(), std::forward<ARGS>(args)...);
    }

    /// @param source the matrix source
    /// @param args the arguments for the matrix initializer
    /// @return an `ast::CallExpression` of a 3x4 matrix of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS>
    const ast::CallExpression* mat3x4(const Source& source, ARGS&&... args) {
        return Construct(source, ty.mat3x4<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the matrix initializer
    /// @return an `ast::CallExpression` of a 4x2 matrix of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS, typename _ = DisableIfSource<ARGS...>>
    const ast::CallExpression* mat4x2(ARGS&&... args) {
        return Construct(ty.mat4x2<T>(), std::forward<ARGS>(args)...);
    }

    /// @param source the matrix source
    /// @param args the arguments for the matrix initializer
    /// @return an `ast::CallExpression` of a 4x2 matrix of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS>
    const ast::CallExpression* mat4x2(const Source& source, ARGS&&... args) {
        return Construct(source, ty.mat4x2<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the matrix initializer
    /// @return an `ast::CallExpression` of a 4x3 matrix of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS, typename _ = DisableIfSource<ARGS...>>
    const ast::CallExpression* mat4x3(ARGS&&... args) {
        return Construct(ty.mat4x3<T>(), std::forward<ARGS>(args)...);
    }

    /// @param source the matrix source
    /// @param args the arguments for the matrix initializer
    /// @return an `ast::CallExpression` of a 4x3 matrix of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS>
    const ast::CallExpression* mat4x3(const Source& source, ARGS&&... args) {
        return Construct(source, ty.mat4x3<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the matrix initializer
    /// @return an `ast::CallExpression` of a 4x4 matrix of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS, typename _ = DisableIfSource<ARGS...>>
    const ast::CallExpression* mat4x4(ARGS&&... args) {
        return Construct(ty.mat4x4<T>(), std::forward<ARGS>(args)...);
    }

    /// @param source the matrix source
    /// @param args the arguments for the matrix initializer
    /// @return an `ast::CallExpression` of a 4x4 matrix of type
    /// `T`, constructed with the values `args`.
    template <typename T, typename... ARGS>
    const ast::CallExpression* mat4x4(const Source& source, ARGS&&... args) {
        return Construct(source, ty.mat4x4<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the array initializer
    /// @return an `ast::CallExpression` of an array with element type
    /// `T` and size `N`, constructed with the values `args`.
    template <typename T, int N, typename... ARGS>
    const ast::CallExpression* array(ARGS&&... args) {
        return Construct(ty.array<T, N>(), std::forward<ARGS>(args)...);
    }

    /// @param source the array source
    /// @param args the arguments for the array initializer
    /// @return an `ast::CallExpression` of an array with element type
    /// `T` and size `N`, constructed with the values `args`.
    template <typename T, int N, typename... ARGS>
    const ast::CallExpression* array(const Source& source, ARGS&&... args) {
        return Construct(source, ty.array<T, N>(), std::forward<ARGS>(args)...);
    }

    /// @param subtype the array element type
    /// @param n the array size. nullptr represents a runtime-array.
    /// @param args the arguments for the array initializer
    /// @return an `ast::CallExpression` of an array with element type
    /// `subtype`, constructed with the values `args`.
    template <typename EXPR, typename... ARGS>
    const ast::CallExpression* array(const ast::Type* subtype, EXPR&& n, ARGS&&... args) {
        return Construct(ty.array(subtype, std::forward<EXPR>(n)), std::forward<ARGS>(args)...);
    }

    /// @param source the array source
    /// @param subtype the array element type
    /// @param n the array size. nullptr represents a runtime-array.
    /// @param args the arguments for the array initializer
    /// @return an `ast::CallExpression` of an array with element type
    /// `subtype`, constructed with the values `args`.
    template <typename EXPR, typename... ARGS>
    const ast::CallExpression* array(const Source& source,
                                     const ast::Type* subtype,
                                     EXPR&& n,
                                     ARGS&&... args) {
        return Construct(source, ty.array(subtype, std::forward<EXPR>(n)),
                         std::forward<ARGS>(args)...);
    }

    /// Adds the extension to the list of enable directives at the top of the module.
    /// @param ext the extension to enable
    /// @return an `ast::Enable` enabling the given extension.
    const ast::Enable* Enable(ast::Extension ext) {
        auto* enable = create<ast::Enable>(ext);
        AST().AddEnable(enable);
        return enable;
    }

    /// Adds the extension to the list of enable directives at the top of the module.
    /// @param source the enable source
    /// @param ext the extension to enable
    /// @return an `ast::Enable` enabling the given extension.
    const ast::Enable* Enable(const Source& source, ast::Extension ext) {
        auto* enable = create<ast::Enable>(source, ext);
        AST().AddEnable(enable);
        return enable;
    }

    /// @param name the variable name
    /// @param options the extra options passed to the ast::Var initializer
    /// Can be any of the following, in any order:
    ///   * ast::Type*          - specifies the variable type
    ///   * ast::AddressSpace   - specifies the variable address space
    ///   * ast::Access         - specifies the variable's access control
    ///   * ast::Expression*    - specifies the variable's initializer expression
    ///   * ast::Attribute*     - specifies the variable's attributes (repeatable, or vector)
    /// Note that non-repeatable arguments of the same type will use the last argument's value.
    /// @returns a `ast::Var` with the given name, type and additional
    /// options
    template <typename NAME, typename... OPTIONS, typename = DisableIfSource<NAME>>
    const ast::Var* Var(NAME&& name, OPTIONS&&... options) {
        VarOptions opts(std::forward<OPTIONS>(options)...);
        return create<ast::Var>(Sym(std::forward<NAME>(name)), opts.type, opts.address_space,
                                opts.access, opts.initializer, std::move(opts.attributes));
    }

    /// @param source the variable source
    /// @param name the variable name
    /// @param options the extra options passed to the ast::Var initializer
    /// Can be any of the following, in any order:
    ///   * ast::Type*          - specifies the variable type
    ///   * ast::AddressSpace   - specifies the variable address space
    ///   * ast::Access         - specifies the variable's access control
    ///   * ast::Expression*    - specifies the variable's initializer expression
    ///   * ast::Attribute*     - specifies the variable's attributes (repeatable, or vector)
    /// Note that non-repeatable arguments of the same type will use the last argument's value.
    /// @returns a `ast::Var` with the given name, address_space and type
    template <typename NAME, typename... OPTIONS>
    const ast::Var* Var(const Source& source, NAME&& name, OPTIONS&&... options) {
        VarOptions opts(std::forward<OPTIONS>(options)...);
        return create<ast::Var>(source, Sym(std::forward<NAME>(name)), opts.type,
                                opts.address_space, opts.access, opts.initializer,
                                std::move(opts.attributes));
    }

    /// @param name the variable name
    /// @param options the extra options passed to the ast::Var initializer
    /// Can be any of the following, in any order:
    ///   * ast::Expression*    - specifies the variable's initializer expression (required)
    ///   * ast::Type*          - specifies the variable type
    ///   * ast::Attribute*     - specifies the variable's attributes (repeatable, or vector)
    /// Note that non-repeatable arguments of the same type will use the last argument's value.
    /// @returns an `ast::Const` with the given name, type and additional options
    template <typename NAME, typename... OPTIONS, typename = DisableIfSource<NAME>>
    const ast::Const* Const(NAME&& name, OPTIONS&&... options) {
        ConstOptions opts(std::forward<OPTIONS>(options)...);
        return create<ast::Const>(Sym(std::forward<NAME>(name)), opts.type, opts.initializer,
                                  std::move(opts.attributes));
    }

    /// @param source the variable source
    /// @param name the variable name
    /// @param options the extra options passed to the ast::Var initializer
    /// Can be any of the following, in any order:
    ///   * ast::Expression*    - specifies the variable's initializer expression (required)
    ///   * ast::Type*          - specifies the variable type
    ///   * ast::Attribute*     - specifies the variable's attributes (repeatable, or vector)
    /// Note that non-repeatable arguments of the same type will use the last argument's value.
    /// @returns an `ast::Const` with the given name, type and additional options
    template <typename NAME, typename... OPTIONS>
    const ast::Const* Const(const Source& source, NAME&& name, OPTIONS&&... options) {
        ConstOptions opts(std::forward<OPTIONS>(options)...);
        return create<ast::Const>(source, Sym(std::forward<NAME>(name)), opts.type,
                                  opts.initializer, std::move(opts.attributes));
    }

    /// @param name the variable name
    /// @param options the extra options passed to the ast::Var initializer
    /// Can be any of the following, in any order:
    ///   * ast::Expression*    - specifies the variable's initializer expression (required)
    ///   * ast::Type*          - specifies the variable type
    ///   * ast::Attribute*     - specifies the variable's attributes (repeatable, or vector)
    /// Note that non-repeatable arguments of the same type will use the last argument's value.
    /// @returns an `ast::Let` with the given name, type and additional options
    template <typename NAME, typename... OPTIONS, typename = DisableIfSource<NAME>>
    const ast::Let* Let(NAME&& name, OPTIONS&&... options) {
        LetOptions opts(std::forward<OPTIONS>(options)...);
        return create<ast::Let>(Sym(std::forward<NAME>(name)), opts.type, opts.initializer,
                                std::move(opts.attributes));
    }

    /// @param source the variable source
    /// @param name the variable name
    /// @param options the extra options passed to the ast::Var initializer
    /// Can be any of the following, in any order:
    ///   * ast::Expression*    - specifies the variable's initializer expression (required)
    ///   * ast::Type*          - specifies the variable type
    ///   * ast::Attribute*     - specifies the variable's attributes (repeatable, or vector)
    /// Note that non-repeatable arguments of the same type will use the last argument's value.
    /// @returns an `ast::Let` with the given name, type and additional options
    template <typename NAME, typename... OPTIONS>
    const ast::Let* Let(const Source& source, NAME&& name, OPTIONS&&... options) {
        LetOptions opts(std::forward<OPTIONS>(options)...);
        return create<ast::Let>(source, Sym(std::forward<NAME>(name)), opts.type, opts.initializer,
                                std::move(opts.attributes));
    }

    /// @param name the parameter name
    /// @param type the parameter type
    /// @param attributes optional parameter attributes
    /// @returns an `ast::Parameter` with the given name and type
    template <typename NAME>
    const ast::Parameter* Param(NAME&& name,
                                const ast::Type* type,
                                utils::VectorRef<const ast::Attribute*> attributes = utils::Empty) {
        return create<ast::Parameter>(Sym(std::forward<NAME>(name)), type, attributes);
    }

    /// @param source the parameter source
    /// @param name the parameter name
    /// @param type the parameter type
    /// @param attributes optional parameter attributes
    /// @returns an `ast::Parameter` with the given name and type
    template <typename NAME>
    const ast::Parameter* Param(const Source& source,
                                NAME&& name,
                                const ast::Type* type,
                                utils::VectorRef<const ast::Attribute*> attributes = utils::Empty) {
        return create<ast::Parameter>(source, Sym(std::forward<NAME>(name)), type, attributes);
    }

    /// @param name the variable name
    /// @param options the extra options passed to the ast::Var initializer
    /// Can be any of the following, in any order:
    ///   * ast::Type*          - specifies the variable type
    ///   * ast::AddressSpace   - specifies the variable address space
    ///   * ast::Access         - specifies the variable's access control
    ///   * ast::Expression*    - specifies the variable's initializer expression
    ///   * ast::Attribute*     - specifies the variable's attributes (repeatable, or vector)
    /// Note that non-repeatable arguments of the same type will use the last argument's value.
    /// @returns a new `ast::Var`, which is automatically registered as a global variable with the
    /// ast::Module.
    template <typename NAME, typename... OPTIONS, typename = DisableIfSource<NAME>>
    const ast::Var* GlobalVar(NAME&& name, OPTIONS&&... options) {
        auto* variable = Var(std::forward<NAME>(name), std::forward<OPTIONS>(options)...);
        AST().AddGlobalVariable(variable);
        return variable;
    }

    /// @param source the variable source
    /// @param name the variable name
    /// @param options the extra options passed to the ast::Var initializer
    /// Can be any of the following, in any order:
    ///   * ast::Type*          - specifies the variable type
    ///   * ast::AddressSpace   - specifies the variable address space
    ///   * ast::Access         - specifies the variable's access control
    ///   * ast::Expression*    - specifies the variable's initializer expression
    ///   * ast::Attribute*    - specifies the variable's attributes (repeatable, or vector)
    /// Note that non-repeatable arguments of the same type will use the last argument's value.
    /// @returns a new `ast::Var`, which is automatically registered as a global variable with the
    /// ast::Module.
    template <typename NAME, typename... OPTIONS>
    const ast::Var* GlobalVar(const Source& source, NAME&& name, OPTIONS&&... options) {
        auto* variable = Var(source, std::forward<NAME>(name), std::forward<OPTIONS>(options)...);
        AST().AddGlobalVariable(variable);
        return variable;
    }

    /// @param name the variable name
    /// @param options the extra options passed to the ast::Const initializer
    /// Can be any of the following, in any order:
    ///   * ast::Expression*    - specifies the variable's initializer expression (required)
    ///   * ast::Type*          - specifies the variable type
    ///   * ast::Attribute*     - specifies the variable's attributes (repeatable, or vector)
    /// Note that non-repeatable arguments of the same type will use the last argument's value.
    /// @returns an `ast::Const` with the given name, type and additional options, which is
    /// automatically registered as a global variable with the ast::Module.
    template <typename NAME, typename... OPTIONS, typename = DisableIfSource<NAME>>
    const ast::Const* GlobalConst(NAME&& name, OPTIONS&&... options) {
        auto* variable = Const(std::forward<NAME>(name), std::forward<OPTIONS>(options)...);
        AST().AddGlobalVariable(variable);
        return variable;
    }

    /// @param source the variable source
    /// @param name the variable name
    /// @param options the extra options passed to the ast::Const initializer
    /// Can be any of the following, in any order:
    ///   * ast::Expression*    - specifies the variable's initializer expression (required)
    ///   * ast::Type*          - specifies the variable type
    ///   * ast::Attribute*     - specifies the variable's attributes (repeatable, or vector)
    /// Note that non-repeatable arguments of the same type will use the last argument's value.
    /// @returns an `ast::Const` with the given name, type and additional options, which is
    /// automatically registered as a global variable with the ast::Module.
    template <typename NAME, typename... OPTIONS>
    const ast::Const* GlobalConst(const Source& source, NAME&& name, OPTIONS&&... options) {
        auto* variable = Const(source, std::forward<NAME>(name), std::forward<OPTIONS>(options)...);
        AST().AddGlobalVariable(variable);
        return variable;
    }

    /// @param name the variable name
    /// @param options the extra options passed to the ast::Override initializer
    /// Can be any of the following, in any order:
    ///   * ast::Expression*    - specifies the variable's initializer expression (required)
    ///   * ast::Type*          - specifies the variable type
    ///   * ast::Attribute*     - specifies the variable's attributes (repeatable, or vector)
    /// Note that non-repeatable arguments of the same type will use the last argument's value.
    /// @returns an `ast::Override` with the given name, type and additional options, which is
    /// automatically registered as a global variable with the ast::Module.
    template <typename NAME, typename... OPTIONS, typename = DisableIfSource<NAME>>
    const ast::Override* Override(NAME&& name, OPTIONS&&... options) {
        OverrideOptions opts(std::forward<OPTIONS>(options)...);
        auto* variable = create<ast::Override>(Sym(std::forward<NAME>(name)), opts.type,
                                               opts.initializer, std::move(opts.attributes));
        AST().AddGlobalVariable(variable);
        return variable;
    }

    /// @param source the variable source
    /// @param name the variable name
    /// @param options the extra options passed to the ast::Override initializer
    /// Can be any of the following, in any order:
    ///   * ast::Expression*    - specifies the variable's initializer expression (required)
    ///   * ast::Type*          - specifies the variable type
    ///   * ast::Attribute*     - specifies the variable's attributes (repeatable, or vector)
    /// Note that non-repeatable arguments of the same type will use the last argument's value.
    /// @returns an `ast::Override` with the given name, type and additional options, which is
    /// automatically registered as a global variable with the ast::Module.
    template <typename NAME, typename... OPTIONS>
    const ast::Override* Override(const Source& source, NAME&& name, OPTIONS&&... options) {
        OverrideOptions opts(std::forward<OPTIONS>(options)...);
        auto* variable = create<ast::Override>(source, Sym(std::forward<NAME>(name)), opts.type,
                                               opts.initializer, std::move(opts.attributes));
        AST().AddGlobalVariable(variable);
        return variable;
    }

    /// @param source the source information
    /// @param condition the assertion condition
    /// @returns a new `ast::StaticAssert`, which is automatically registered as a global statement
    /// with the ast::Module.
    template <typename EXPR>
    const ast::StaticAssert* GlobalStaticAssert(const Source& source, EXPR&& condition) {
        auto* sa = StaticAssert(source, std::forward<EXPR>(condition));
        AST().AddStaticAssert(sa);
        return sa;
    }

    /// @param condition the assertion condition
    /// @returns a new `ast::StaticAssert`, which is automatically registered as a global statement
    /// with the ast::Module.
    template <typename EXPR, typename = DisableIfSource<EXPR>>
    const ast::StaticAssert* GlobalStaticAssert(EXPR&& condition) {
        auto* sa = StaticAssert(std::forward<EXPR>(condition));
        AST().AddStaticAssert(sa);
        return sa;
    }

    /// @param source the source information
    /// @param condition the assertion condition
    /// @returns a new `ast::StaticAssert` with the given assertion condition
    template <typename EXPR>
    const ast::StaticAssert* StaticAssert(const Source& source, EXPR&& condition) {
        return create<ast::StaticAssert>(source, Expr(std::forward<EXPR>(condition)));
    }

    /// @param condition the assertion condition
    /// @returns a new `ast::StaticAssert` with the given assertion condition
    template <typename EXPR, typename = DisableIfSource<EXPR>>
    const ast::StaticAssert* StaticAssert(EXPR&& condition) {
        return create<ast::StaticAssert>(Expr(std::forward<EXPR>(condition)));
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

    /// @param expr the expression to perform a unary negation on
    /// @return an ast::UnaryOpExpression that is the unary negation of the
    /// input expression
    template <typename EXPR>
    const ast::UnaryOpExpression* Negation(EXPR&& expr) {
        return create<ast::UnaryOpExpression>(ast::UnaryOp::kNegation,
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

    /// @param source the source information
    /// @param lhs the left hand argument to the addition operation
    /// @param rhs the right hand argument to the addition operation
    /// @returns a `ast::BinaryExpression` summing the arguments `lhs` and `rhs`
    template <typename LHS, typename RHS>
    const ast::BinaryExpression* Add(const Source& source, LHS&& lhs, RHS&& rhs) {
        return create<ast::BinaryExpression>(source, ast::BinaryOp::kAdd,
                                             Expr(std::forward<LHS>(lhs)),
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

    /// @param source the source information
    /// @param lhs the left hand argument to the bit shift left operation
    /// @param rhs the right hand argument to the bit shift left operation
    /// @returns a `ast::BinaryExpression` bit shifting left `lhs` by `rhs`
    template <typename LHS, typename RHS>
    const ast::BinaryExpression* Shl(const Source& source, LHS&& lhs, RHS&& rhs) {
        return create<ast::BinaryExpression>(source, ast::BinaryOp::kShiftLeft,
                                             Expr(std::forward<LHS>(lhs)),
                                             Expr(std::forward<RHS>(rhs)));
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
    /// @param val the offset expression
    /// @returns the offset attribute pointer
    template <typename EXPR>
    const ast::StructMemberOffsetAttribute* MemberOffset(EXPR&& val) {
        return create<ast::StructMemberOffsetAttribute>(source_, Expr(std::forward<EXPR>(val)));
    }

    /// Creates a ast::StructMemberOffsetAttribute
    /// @param source the source information
    /// @param val the offset expression
    /// @returns the offset attribute pointer
    template <typename EXPR>
    const ast::StructMemberOffsetAttribute* MemberOffset(const Source& source, EXPR&& val) {
        return create<ast::StructMemberOffsetAttribute>(source, Expr(std::forward<EXPR>(val)));
    }

    /// Creates a ast::StructMemberSizeAttribute
    /// @param source the source information
    /// @param val the size value
    /// @returns the size attribute pointer
    template <typename EXPR>
    const ast::StructMemberSizeAttribute* MemberSize(const Source& source, EXPR&& val) {
        return create<ast::StructMemberSizeAttribute>(source, Expr(std::forward<EXPR>(val)));
    }

    /// Creates a ast::StructMemberSizeAttribute
    /// @param val the size value
    /// @returns the size attribute pointer
    template <typename EXPR>
    const ast::StructMemberSizeAttribute* MemberSize(EXPR&& val) {
        return create<ast::StructMemberSizeAttribute>(source_, Expr(std::forward<EXPR>(val)));
    }

    /// Creates a ast::StructMemberAlignAttribute
    /// @param source the source information
    /// @param val the align value expression
    /// @returns the align attribute pointer
    template <typename EXPR>
    const ast::StructMemberAlignAttribute* MemberAlign(const Source& source, EXPR&& val) {
        return create<ast::StructMemberAlignAttribute>(source, Expr(std::forward<EXPR>(val)));
    }

    /// Creates a ast::StructMemberAlignAttribute
    /// @param val the align value expression
    /// @returns the align attribute pointer
    template <typename EXPR>
    const ast::StructMemberAlignAttribute* MemberAlign(EXPR&& val) {
        return create<ast::StructMemberAlignAttribute>(source_, Expr(std::forward<EXPR>(val)));
    }

    /// Creates the ast::GroupAttribute
    /// @param value group attribute index expresion
    /// @returns the group attribute pointer
    template <typename EXPR>
    const ast::GroupAttribute* Group(EXPR&& value) {
        return create<ast::GroupAttribute>(Expr(std::forward<EXPR>(value)));
    }

    /// Creates the ast::GroupAttribute
    /// @param source the source
    /// @param value group attribute index expression
    /// @returns the group attribute pointer
    template <typename EXPR>
    const ast::GroupAttribute* Group(const Source& source, EXPR&& value) {
        return create<ast::GroupAttribute>(source, Expr(std::forward<EXPR>(value)));
    }

    /// Creates the ast::BindingAttribute
    /// @param value the binding index expression
    /// @returns the binding deocration pointer
    template <typename EXPR>
    const ast::BindingAttribute* Binding(EXPR&& value) {
        return create<ast::BindingAttribute>(Expr(std::forward<EXPR>(value)));
    }

    /// Creates the ast::BindingAttribute
    /// @param source the source
    /// @param value the binding index expression
    /// @returns the binding deocration pointer
    template <typename EXPR>
    const ast::BindingAttribute* Binding(const Source& source, EXPR&& value) {
        return create<ast::BindingAttribute>(source, Expr(std::forward<EXPR>(value)));
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
    const ast::Function* Func(
        const Source& source,
        NAME&& name,
        utils::VectorRef<const ast::Parameter*> params,
        const ast::Type* type,
        utils::VectorRef<const ast::Statement*> body,
        utils::VectorRef<const ast::Attribute*> attributes = utils::Empty,
        utils::VectorRef<const ast::Attribute*> return_type_attributes = utils::Empty) {
        auto* func =
            create<ast::Function>(source, Sym(std::forward<NAME>(name)), std::move(params), type,
                                  create<ast::BlockStatement>(std::move(body)),
                                  std::move(attributes), std::move(return_type_attributes));
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
    const ast::Function* Func(
        NAME&& name,
        utils::VectorRef<const ast::Parameter*> params,
        const ast::Type* type,
        utils::VectorRef<const ast::Statement*> body,
        utils::VectorRef<const ast::Attribute*> attributes = utils::Empty,
        utils::VectorRef<const ast::Attribute*> return_type_attributes = utils::Empty) {
        auto* func =
            create<ast::Function>(Sym(std::forward<NAME>(name)), std::move(params), type,
                                  create<ast::BlockStatement>(std::move(body)),
                                  std::move(attributes), std::move(return_type_attributes));
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

    /// Creates a ast::BreakIfStatement with input condition
    /// @param source the source information for the if statement
    /// @param condition the if statement condition expression
    /// @returns the break-if statement pointer
    template <typename CONDITION>
    const ast::BreakIfStatement* BreakIf(const Source& source, CONDITION&& condition) {
        return create<ast::BreakIfStatement>(source, Expr(std::forward<CONDITION>(condition)));
    }

    /// Creates a ast::BreakIfStatement with input condition
    /// @param condition the if statement condition expression
    /// @returns the break-if statement pointer
    template <typename CONDITION>
    const ast::BreakIfStatement* BreakIf(CONDITION&& condition) {
        return create<ast::BreakIfStatement>(Expr(std::forward<CONDITION>(condition)));
    }

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
    const ast::Struct* Structure(const Source& source,
                                 NAME&& name,
                                 utils::VectorRef<const ast::StructMember*> members) {
        auto sym = Sym(std::forward<NAME>(name));
        auto* type = create<ast::Struct>(source, sym, std::move(members), utils::Empty);
        AST().AddTypeDecl(type);
        return type;
    }

    /// Creates a ast::Struct registering it with the AST().TypeDecls().
    /// @param name the struct name
    /// @param members the struct members
    /// @returns the struct type
    template <typename NAME>
    const ast::Struct* Structure(NAME&& name, utils::VectorRef<const ast::StructMember*> members) {
        auto sym = Sym(std::forward<NAME>(name));
        auto* type = create<ast::Struct>(sym, std::move(members), utils::Empty);
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
    const ast::StructMember* Member(
        const Source& source,
        NAME&& name,
        const ast::Type* type,
        utils::VectorRef<const ast::Attribute*> attributes = utils::Empty) {
        return create<ast::StructMember>(source, Sym(std::forward<NAME>(name)), type,
                                         std::move(attributes));
    }

    /// Creates a ast::StructMember
    /// @param name the struct member name
    /// @param type the struct member type
    /// @param attributes the optional struct member attributes
    /// @returns the struct member pointer
    template <typename NAME>
    const ast::StructMember* Member(
        NAME&& name,
        const ast::Type* type,
        utils::VectorRef<const ast::Attribute*> attributes = utils::Empty) {
        return create<ast::StructMember>(source_, Sym(std::forward<NAME>(name)), type,
                                         std::move(attributes));
    }

    /// Creates a ast::StructMember with the given byte offset
    /// @param offset the offset to use in the StructMemberOffsetAttribute
    /// @param name the struct member name
    /// @param type the struct member type
    /// @returns the struct member pointer
    template <typename NAME>
    const ast::StructMember* Member(uint32_t offset, NAME&& name, const ast::Type* type) {
        return create<ast::StructMember>(source_, Sym(std::forward<NAME>(name)), type,
                                         utils::Vector<const ast::Attribute*, 1>{
                                             MemberOffset(AInt(offset)),
                                         });
    }

    /// Creates a ast::BlockStatement with input statements
    /// @param source the source information for the block
    /// @param statements statements of block
    /// @returns the block statement pointer
    template <typename... Statements>
    const ast::BlockStatement* Block(const Source& source, Statements&&... statements) {
        return create<ast::BlockStatement>(
            source, utils::Vector<const ast::Statement*, sizeof...(statements)>{
                        std::forward<Statements>(statements)...,
                    });
    }

    /// Creates a ast::BlockStatement with input statements
    /// @param statements statements of block
    /// @returns the block statement pointer
    template <typename... STATEMENTS, typename = DisableIfSource<STATEMENTS...>>
    const ast::BlockStatement* Block(STATEMENTS&&... statements) {
        return create<ast::BlockStatement>(
            utils::Vector<const ast::Statement*, sizeof...(statements)>{
                std::forward<STATEMENTS>(statements)...,
            });
    }

    /// A wrapper type for the Else statement used to create If statements.
    struct ElseStmt {
        /// Default constructor - no else statement.
        ElseStmt() : stmt(nullptr) {}
        /// Constructor
        /// @param s The else statement
        explicit ElseStmt(const ast::Statement* s) : stmt(s) {}
        /// The else statement, or nullptr.
        const ast::Statement* stmt;
    };

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
                               const ElseStmt else_stmt = ElseStmt()) {
        return create<ast::IfStatement>(source, Expr(std::forward<CONDITION>(condition)), body,
                                        else_stmt.stmt);
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
                               const ElseStmt else_stmt = ElseStmt()) {
        return create<ast::IfStatement>(Expr(std::forward<CONDITION>(condition)), body,
                                        else_stmt.stmt);
    }

    /// Creates an Else object.
    /// @param stmt else statement
    /// @returns the Else object
    ElseStmt Else(const ast::Statement* stmt) { return ElseStmt(stmt); }

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

    /// Creates a ast::WhileStatement with input body and condition.
    /// @param source the source information
    /// @param cond the loop condition
    /// @param body the loop body
    /// @returns the while statement pointer
    template <typename COND>
    const ast::WhileStatement* While(const Source& source,
                                     COND&& cond,
                                     const ast::BlockStatement* body) {
        return create<ast::WhileStatement>(source, Expr(std::forward<COND>(cond)), body);
    }

    /// Creates a ast::WhileStatement with given condition and body.
    /// @param cond the condition
    /// @param body the loop body
    /// @returns the while loop statement pointer
    template <typename COND>
    const ast::WhileStatement* While(COND&& cond, const ast::BlockStatement* body) {
        return create<ast::WhileStatement>(Expr(std::forward<COND>(cond)), body);
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
        return create<ast::SwitchStatement>(
            source, Expr(std::forward<ExpressionInit>(condition)),
            utils::Vector<const ast::CaseStatement*, sizeof...(cases)>{
                std::forward<Cases>(cases)...});
    }

    /// Creates a ast::SwitchStatement with input expression and cases
    /// @param condition the condition expression initializer
    /// @param cases case statements
    /// @returns the switch statement pointer
    template <typename ExpressionInit,
              typename... Cases,
              typename = DisableIfSource<ExpressionInit>>
    const ast::SwitchStatement* Switch(ExpressionInit&& condition, Cases&&... cases) {
        return create<ast::SwitchStatement>(
            Expr(std::forward<ExpressionInit>(condition)),
            utils::Vector<const ast::CaseStatement*, sizeof...(cases)>{
                std::forward<Cases>(cases)...});
    }

    /// Creates a ast::CaseStatement with input list of selectors, and body
    /// @param source the source information
    /// @param selectors list of selectors
    /// @param body the case body
    /// @returns the case statement pointer
    const ast::CaseStatement* Case(const Source& source,
                                   utils::VectorRef<const ast::CaseSelector*> selectors,
                                   const ast::BlockStatement* body = nullptr) {
        return create<ast::CaseStatement>(source, std::move(selectors), body ? body : Block());
    }

    /// Creates a ast::CaseStatement with input list of selectors, and body
    /// @param selectors list of selectors
    /// @param body the case body
    /// @returns the case statement pointer
    const ast::CaseStatement* Case(utils::VectorRef<const ast::CaseSelector*> selectors,
                                   const ast::BlockStatement* body = nullptr) {
        return create<ast::CaseStatement>(std::move(selectors), body ? body : Block());
    }

    /// Convenient overload that takes a single selector
    /// @param selector a single case selector
    /// @param body the case body
    /// @returns the case statement pointer
    const ast::CaseStatement* Case(const ast::CaseSelector* selector,
                                   const ast::BlockStatement* body = nullptr) {
        return Case(utils::Vector{selector}, body);
    }

    /// Convenience function that creates a 'default' ast::CaseStatement
    /// @param source the source information
    /// @param body the case body
    /// @returns the case statement pointer
    const ast::CaseStatement* DefaultCase(const Source& source,
                                          const ast::BlockStatement* body = nullptr) {
        return Case(source, utils::Vector{DefaultCaseSelector(source)}, body);
    }

    /// Convenience function that creates a 'default' ast::CaseStatement
    /// @param body the case body
    /// @returns the case statement pointer
    const ast::CaseStatement* DefaultCase(const ast::BlockStatement* body = nullptr) {
        return Case(utils::Vector{DefaultCaseSelector()}, body);
    }

    /// Convenience function that creates a case selector
    /// @param source the source information
    /// @param expr the selector expression
    /// @returns the selector pointer
    template <typename EXPR>
    const ast::CaseSelector* CaseSelector(const Source& source, EXPR&& expr) {
        return create<ast::CaseSelector>(source, Expr(std::forward<EXPR>(expr)));
    }

    /// Convenience function that creates a case selector
    /// @param expr the selector expression
    /// @returns the selector pointer
    template <typename EXPR>
    const ast::CaseSelector* CaseSelector(EXPR&& expr) {
        return create<ast::CaseSelector>(source_, Expr(std::forward<EXPR>(expr)));
    }

    /// Convenience function that creates a default case selector
    /// @param source the source information
    /// @returns the selector pointer
    const ast::CaseSelector* DefaultCaseSelector(const Source& source) {
        return create<ast::CaseSelector>(source, nullptr);
    }

    /// Convenience function that creates a default case selector
    /// @returns the selector pointer
    const ast::CaseSelector* DefaultCaseSelector() { return create<ast::CaseSelector>(nullptr); }

    /// Creates an ast::BuiltinAttribute
    /// @param source the source information
    /// @param builtin the builtin value
    /// @returns the builtin attribute pointer
    const ast::BuiltinAttribute* Builtin(const Source& source, ast::BuiltinValue builtin) {
        return create<ast::BuiltinAttribute>(source, builtin);
    }

    /// Creates an ast::BuiltinAttribute
    /// @param builtin the builtin value
    /// @returns the builtin attribute pointer
    const ast::BuiltinAttribute* Builtin(ast::BuiltinValue builtin) {
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
        ast::InterpolationSampling sampling = ast::InterpolationSampling::kUndefined) {
        return create<ast::InterpolateAttribute>(source, type, sampling);
    }

    /// Creates an ast::InterpolateAttribute
    /// @param type the interpolation type
    /// @param sampling the interpolation sampling
    /// @returns the interpolate attribute pointer
    const ast::InterpolateAttribute* Interpolate(
        ast::InterpolationType type,
        ast::InterpolationSampling sampling = ast::InterpolationSampling::kUndefined) {
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
    /// @param location the location value expression
    /// @returns the location attribute pointer
    template <typename EXPR>
    const ast::LocationAttribute* Location(const Source& source, EXPR&& location) {
        return create<ast::LocationAttribute>(source, Expr(std::forward<EXPR>(location)));
    }

    /// Creates an ast::LocationAttribute
    /// @param location the location value expression
    /// @returns the location attribute pointer
    template <typename EXPR>
    const ast::LocationAttribute* Location(EXPR&& location) {
        return create<ast::LocationAttribute>(source_, Expr(std::forward<EXPR>(location)));
    }

    /// Creates an ast::IdAttribute
    /// @param source the source information
    /// @param id the id value
    /// @returns the override attribute pointer
    const ast::IdAttribute* Id(const Source& source, OverrideId id) {
        return create<ast::IdAttribute>(source, Expr(AInt(id.value)));
    }

    /// Creates an ast::IdAttribute with an override identifier
    /// @param id the optional id value
    /// @returns the override attribute pointer
    const ast::IdAttribute* Id(OverrideId id) {
        return create<ast::IdAttribute>(Expr(AInt(id.value)));
    }

    /// Creates an ast::IdAttribute
    /// @param source the source information
    /// @param id the id value expression
    /// @returns the override attribute pointer
    template <typename EXPR>
    const ast::IdAttribute* Id(const Source& source, EXPR&& id) {
        return create<ast::IdAttribute>(source, Expr(std::forward<EXPR>(id)));
    }

    /// Creates an ast::IdAttribute with an override identifier
    /// @param id the optional id value expression
    /// @returns the override attribute pointer
    template <typename EXPR>
    const ast::IdAttribute* Id(EXPR&& id) {
        return create<ast::IdAttribute>(Expr(std::forward<EXPR>(id)));
    }

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
    /// @param source the source information
    /// @param x the x dimension expression
    /// @returns the workgroup attribute pointer
    template <typename EXPR_X>
    const ast::WorkgroupAttribute* WorkgroupSize(const Source& source, EXPR_X&& x) {
        return WorkgroupSize(source, std::forward<EXPR_X>(x), nullptr, nullptr);
    }

    /// Creates an ast::WorkgroupAttribute
    /// @param source the source information
    /// @param x the x dimension expression
    /// @param y the y dimension expression
    /// @returns the workgroup attribute pointer
    template <typename EXPR_X, typename EXPR_Y>
    const ast::WorkgroupAttribute* WorkgroupSize(const Source& source, EXPR_X&& x, EXPR_Y&& y) {
        return WorkgroupSize(source, std::forward<EXPR_X>(x), std::forward<EXPR_Y>(y), nullptr);
    }

    /// Creates an ast::WorkgroupAttribute
    /// @param x the x dimension expression
    /// @param y the y dimension expression
    /// @returns the workgroup attribute pointer
    template <typename EXPR_X, typename EXPR_Y, typename = DisableIfSource<EXPR_X>>
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
    template <typename EXPR_X, typename EXPR_Y, typename EXPR_Z, typename = DisableIfSource<EXPR_X>>
    const ast::WorkgroupAttribute* WorkgroupSize(EXPR_X&& x, EXPR_Y&& y, EXPR_Z&& z) {
        return create<ast::WorkgroupAttribute>(source_, Expr(std::forward<EXPR_X>(x)),
                                               Expr(std::forward<EXPR_Y>(y)),
                                               Expr(std::forward<EXPR_Z>(z)));
    }

    /// Creates an ast::DisableValidationAttribute
    /// @param validation the validation to disable
    /// @returns the disable validation attribute pointer
    const ast::DisableValidationAttribute* Disable(ast::DisabledValidation validation) {
        return ASTNodes().Create<ast::DisableValidationAttribute>(ID(), AllocateNodeID(),
                                                                  validation);
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

    /// @param type a type
    /// @returns the name for `type` that closely resembles how it would be
    /// declared in WGSL.
    std::string FriendlyName(const ast::Type* type) const;

    /// @param type a type
    /// @returns the name for `type` that closely resembles how it would be
    /// declared in WGSL.
    std::string FriendlyName(const sem::Type* type) const;

    /// Overload of FriendlyName, which removes an ambiguity when passing nullptr.
    /// Simplifies test code.
    /// @returns "<null>"
    std::string FriendlyName(std::nullptr_t) const;

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
        utils::Vector stmts{
            WrapInStatement(std::forward<ARGS>(args))...,
        };
        return WrapInFunction(utils::VectorRef<const ast::Statement*>{std::move(stmts)});
    }
    /// @param stmts a list of ast::Statement that will be wrapped by a function,
    /// so that each statement is reachable by the Resolver.
    /// @returns the function
    const ast::Function* WrapInFunction(utils::VectorRef<const ast::Statement*> stmts);

    /// The builder types
    TypesBuilder const ty{this};

  protected:
    /// Asserts that the builder has not been moved.
    void AssertNotMoved() const;

  private:
    ProgramID id_;
    ast::NodeID last_ast_node_id_ = ast::NodeID{static_cast<decltype(ast::NodeID::value)>(0) - 1};
    sem::TypeManager types_;
    ASTNodeAllocator ast_nodes_;
    SemNodeAllocator sem_nodes_;
    ConstantAllocator constant_nodes_;
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
struct ProgramBuilder::TypesBuilder::CToAST<AInt> {
    static const ast::Type* get(const ProgramBuilder::TypesBuilder*) { return nullptr; }
};
template <>
struct ProgramBuilder::TypesBuilder::CToAST<AFloat> {
    static const ast::Type* get(const ProgramBuilder::TypesBuilder*) { return nullptr; }
};
template <>
struct ProgramBuilder::TypesBuilder::CToAST<i32> {
    static const ast::Type* get(const ProgramBuilder::TypesBuilder* t) { return t->i32(); }
};
template <>
struct ProgramBuilder::TypesBuilder::CToAST<u32> {
    static const ast::Type* get(const ProgramBuilder::TypesBuilder* t) { return t->u32(); }
};
template <>
struct ProgramBuilder::TypesBuilder::CToAST<f32> {
    static const ast::Type* get(const ProgramBuilder::TypesBuilder* t) { return t->f32(); }
};
template <>
struct ProgramBuilder::TypesBuilder::CToAST<f16> {
    static const ast::Type* get(const ProgramBuilder::TypesBuilder* t) { return t->f16(); }
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
