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
#include "src/tint/ast/assignment_statement.h"
#include "src/tint/ast/binary_expression.h"
#include "src/tint/ast/binding_attribute.h"
#include "src/tint/ast/bitcast_expression.h"
#include "src/tint/ast/bool_literal_expression.h"
#include "src/tint/ast/break_if_statement.h"
#include "src/tint/ast/break_statement.h"
#include "src/tint/ast/call_expression.h"
#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/case_statement.h"
#include "src/tint/ast/compound_assignment_statement.h"
#include "src/tint/ast/const.h"
#include "src/tint/ast/const_assert.h"
#include "src/tint/ast/continue_statement.h"
#include "src/tint/ast/diagnostic_attribute.h"
#include "src/tint/ast/diagnostic_control.h"
#include "src/tint/ast/diagnostic_directive.h"
#include "src/tint/ast/diagnostic_rule_name.h"
#include "src/tint/ast/disable_validation_attribute.h"
#include "src/tint/ast/discard_statement.h"
#include "src/tint/ast/enable.h"
#include "src/tint/ast/float_literal_expression.h"
#include "src/tint/ast/for_loop_statement.h"
#include "src/tint/ast/id_attribute.h"
#include "src/tint/ast/identifier.h"
#include "src/tint/ast/if_statement.h"
#include "src/tint/ast/increment_decrement_statement.h"
#include "src/tint/ast/index_accessor_expression.h"
#include "src/tint/ast/int_literal_expression.h"
#include "src/tint/ast/interpolate_attribute.h"
#include "src/tint/ast/invariant_attribute.h"
#include "src/tint/ast/let.h"
#include "src/tint/ast/loop_statement.h"
#include "src/tint/ast/member_accessor_expression.h"
#include "src/tint/ast/module.h"
#include "src/tint/ast/must_use_attribute.h"
#include "src/tint/ast/override.h"
#include "src/tint/ast/parameter.h"
#include "src/tint/ast/phony_expression.h"
#include "src/tint/ast/return_statement.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/ast/stride_attribute.h"
#include "src/tint/ast/struct_member_align_attribute.h"
#include "src/tint/ast/struct_member_offset_attribute.h"
#include "src/tint/ast/struct_member_size_attribute.h"
#include "src/tint/ast/switch_statement.h"
#include "src/tint/ast/templated_identifier.h"
#include "src/tint/ast/type.h"
#include "src/tint/ast/unary_op_expression.h"
#include "src/tint/ast/var.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/ast/while_statement.h"
#include "src/tint/ast/workgroup_attribute.h"
#include "src/tint/builtin/extension.h"
#include "src/tint/builtin/interpolation_sampling.h"
#include "src/tint/builtin/interpolation_type.h"
#include "src/tint/constant/composite.h"
#include "src/tint/constant/splat.h"
#include "src/tint/constant/value.h"
#include "src/tint/number.h"
#include "src/tint/program.h"
#include "src/tint/program_id.h"
#include "src/tint/sem/array_count.h"
#include "src/tint/sem/struct.h"
#include "src/tint/type/array.h"
#include "src/tint/type/bool.h"
#include "src/tint/type/depth_texture.h"
#include "src/tint/type/external_texture.h"
#include "src/tint/type/f16.h"
#include "src/tint/type/f32.h"
#include "src/tint/type/i32.h"
#include "src/tint/type/matrix.h"
#include "src/tint/type/multisampled_texture.h"
#include "src/tint/type/pointer.h"
#include "src/tint/type/sampled_texture.h"
#include "src/tint/type/sampler_kind.h"
#include "src/tint/type/storage_texture.h"
#include "src/tint/type/texture_dimension.h"
#include "src/tint/type/u32.h"
#include "src/tint/type/vector.h"
#include "src/tint/type/void.h"
#include "src/tint/utils/string.h"

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

// A sentinel type used by some template arguments to signal that the a type should be inferred.
struct Infer {};

/// Evaluates to true if T is a Infer, AInt or AFloat.
template <typename T>
static constexpr const bool IsInferOrAbstract =
    std::is_same_v<std::decay_t<T>, Infer> || IsAbstract<std::decay_t<T>>;

// Forward declare metafunction that evaluates to true iff T can be wrapped in a statement.
template <typename T, typename = void>
struct CanWrapInStatement;

/// ProgramBuilder is a mutable builder for a Program.
/// To construct a Program, populate the builder and then `std::move` it to a
/// Program.
class ProgramBuilder {
    /// Evaluates to true if T is a Source
    template <typename T>
    static constexpr const bool IsSource = std::is_same_v<T, Source>;

    /// Evaluates to true if T is a Number or bool.
    template <typename T>
    static constexpr const bool IsScalar =
        std::is_integral_v<UnwrapNumber<T>> || std::is_floating_point_v<UnwrapNumber<T>> ||
        std::is_same_v<T, bool>;

    /// Evaluates to true if T can be converted to an identifier.
    template <typename T>
    static constexpr const bool IsIdentifierLike = std::is_same_v<T, Symbol> ||     // Symbol
                                                   std::is_enum_v<T> ||             // Enum
                                                   utils::traits::IsStringLike<T>;  // String

    /// A helper used to disable overloads if the first type in `TYPES` is a Source. Used to avoid
    /// ambiguities in overloads that take a Source as the first parameter and those that
    /// perfectly-forward the first argument.
    template <typename... TYPES>
    using DisableIfSource = utils::traits::EnableIf<
        !IsSource<utils::traits::Decay<utils::traits::NthTypeOf<0, TYPES..., void>>>>;

    /// A helper used to disable overloads if the first type in `TYPES` is a scalar type. Used to
    /// avoid ambiguities in overloads that take a scalar as the first parameter and those that
    /// perfectly-forward the first argument.
    template <typename... TYPES>
    using DisableIfScalar = utils::traits::EnableIf<
        !IsScalar<utils::traits::Decay<utils::traits::NthTypeOf<0, TYPES..., void>>>>;

    /// A helper used to enable overloads if the first type in `TYPES` is a scalar type. Used to
    /// avoid ambiguities in overloads that take a scalar as the first parameter and those that
    /// perfectly-forward the first argument.
    template <typename... TYPES>
    using EnableIfScalar = utils::traits::EnableIf<
        IsScalar<utils::traits::Decay<utils::traits::NthTypeOf<0, TYPES..., void>>>>;

    /// A helper used to disable overloads if the first type in `TYPES` is a utils::Vector,
    /// utils::VectorRef or utils::VectorRef.
    template <typename... TYPES>
    using DisableIfVectorLike = utils::traits::EnableIf<!detail::IsVectorLike<
        utils::traits::Decay<utils::traits::NthTypeOf<0, TYPES..., void>>>::value>;

    /// A helper used to enable overloads if the first type in `TYPES` is identifier-like.
    template <typename... TYPES>
    using EnableIfIdentifierLike = utils::traits::EnableIf<
        IsIdentifierLike<utils::traits::Decay<utils::traits::NthTypeOf<0, TYPES..., void>>>>;

    /// A helper used to disable overloads if the first type in `TYPES` is Infer or an abstract
    /// numeric.
    template <typename... TYPES>
    using DisableIfInferOrAbstract = utils::traits::EnableIf<
        !IsInferOrAbstract<utils::traits::Decay<utils::traits::NthTypeOf<0, TYPES..., void>>>>;

    /// A helper used to enable overloads if the first type in `TYPES` is Infer or an abstract
    /// numeric.
    template <typename... TYPES>
    using EnableIfInferOrAbstract = utils::traits::EnableIf<
        IsInferOrAbstract<utils::traits::Decay<utils::traits::NthTypeOf<0, TYPES..., void>>>>;

    /// VarOptions is a helper for accepting an arbitrary number of order independent options for
    /// constructing an ast::Var.
    struct VarOptions {
        template <typename... ARGS>
        explicit VarOptions(ProgramBuilder& b, ARGS&&... args) {
            (Set(b, std::forward<ARGS>(args)), ...);
        }
        ~VarOptions();

        ast::Type type;
        const ast::Expression* address_space = nullptr;
        const ast::Expression* access = nullptr;
        const ast::Expression* initializer = nullptr;
        utils::Vector<const ast::Attribute*, 4> attributes;

      private:
        void Set(ProgramBuilder&, ast::Type t) { type = t; }
        void Set(ProgramBuilder& b, builtin::AddressSpace addr_space) {
            if (addr_space != builtin::AddressSpace::kUndefined) {
                address_space = b.Expr(addr_space);
            }
        }
        void Set(ProgramBuilder& b, builtin::Access ac) {
            if (ac != builtin::Access::kUndefined) {
                access = b.Expr(ac);
            }
        }
        void Set(ProgramBuilder&, const ast::Expression* c) { initializer = c; }
        void Set(ProgramBuilder&, utils::VectorRef<const ast::Attribute*> l) {
            attributes = std::move(l);
        }
        void Set(ProgramBuilder&, const ast::Attribute* a) { attributes.Push(a); }
    };

    /// LetOptions is a helper for accepting an arbitrary number of order independent options for
    /// constructing an ast::Let.
    struct LetOptions {
        template <typename... ARGS>
        explicit LetOptions(ARGS&&... args) {
            static constexpr bool has_init =
                (utils::traits::IsTypeOrDerived<utils::traits::PtrElTy<ARGS>, ast::Expression> ||
                 ...);
            static_assert(has_init, "Let() must be constructed with an initializer expression");
            (Set(std::forward<ARGS>(args)), ...);
        }
        ~LetOptions();

        ast::Type type;
        const ast::Expression* initializer = nullptr;
        utils::Vector<const ast::Attribute*, 4> attributes;

      private:
        void Set(ast::Type t) { type = t; }
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
                (utils::traits::IsTypeOrDerived<utils::traits::PtrElTy<ARGS>, ast::Expression> ||
                 ...);
            static_assert(has_init, "Const() must be constructed with an initializer expression");
            (Set(std::forward<ARGS>(args)), ...);
        }
        ~ConstOptions();

        ast::Type type;
        const ast::Expression* initializer = nullptr;
        utils::Vector<const ast::Attribute*, 4> attributes;

      private:
        void Set(ast::Type t) { type = t; }
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

        ast::Type type;
        const ast::Expression* initializer = nullptr;
        utils::Vector<const ast::Attribute*, 4> attributes;

      private:
        void Set(ast::Type t) { type = t; }
        void Set(const ast::Expression* c) { initializer = c; }
        void Set(utils::VectorRef<const ast::Attribute*> l) { attributes = std::move(l); }
        void Set(const ast::Attribute* a) { attributes.Push(a); }
    };

  public:
    /// ASTNodeAllocator is an alias to BlockAllocator<ast::Node>
    using ASTNodeAllocator = utils::BlockAllocator<ast::Node>;

    /// SemNodeAllocator is an alias to BlockAllocator<sem::Node>
    using SemNodeAllocator = utils::BlockAllocator<sem::Node>;

    /// ConstantAllocator is an alias to BlockAllocator<constant::Value>
    using ConstantAllocator = utils::BlockAllocator<constant::Value>;

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
    type::Manager& Types() {
        AssertNotMoved();
        return types_;
    }

    /// @returns a reference to the program's types
    const type::Manager& Types() const {
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
    /// @param args the arguments to pass to the constructor
    /// @returns the node pointer
    template <typename T, typename... ARGS>
    utils::traits::EnableIfIsType<T, ast::Node>* create(const Source& source, ARGS&&... args) {
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
    utils::traits::EnableIfIsType<T, ast::Node>* create() {
        AssertNotMoved();
        return ast_nodes_.Create<T>(id_, AllocateNodeID(), source_);
    }

    /// Creates a new ast::Node owned by the ProgramBuilder, injecting the current
    /// Source as set by the last call to SetSource() as the first argument to the
    /// constructor.
    /// When the ProgramBuilder is destructed, the ast::Node will also be
    /// destructed.
    /// @param arg0 the first arguments to pass to the constructor
    /// @param args the remaining arguments to pass to the constructor
    /// @returns the node pointer
    template <typename T, typename ARG0, typename... ARGS>
    utils::traits::EnableIf</* T is ast::Node and ARG0 is not Source */
                            utils::traits::IsTypeOrDerived<T, ast::Node> &&
                                !utils::traits::IsTypeOrDerived<ARG0, Source>,
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
    utils::traits::EnableIf<utils::traits::IsTypeOrDerived<T, sem::Node> &&
                                !utils::traits::IsTypeOrDerived<T, type::Node>,
                            T>*
    create(ARGS&&... args) {
        AssertNotMoved();
        return sem_nodes_.Create<T>(std::forward<ARGS>(args)...);
    }

    /// Creates a new constant::Value owned by the ProgramBuilder.
    /// When the ProgramBuilder is destructed, the sem::Node will also be destructed.
    /// @param args the arguments to pass to the constructor
    /// @returns the node pointer
    template <typename T, typename... ARGS>
    utils::traits::EnableIf<utils::traits::IsTypeOrDerived<T, constant::Value> &&
                                !utils::traits::IsTypeOrDerived<T, constant::Composite> &&
                                !utils::traits::IsTypeOrDerived<T, constant::Splat>,
                            T>*
    create(ARGS&&... args) {
        AssertNotMoved();
        return constant_nodes_.Create<T>(std::forward<ARGS>(args)...);
    }

    /// Constructs a constant of a vector, matrix or array type.
    ///
    /// Examines the element values and will return either a constant::Composite or a
    /// constant::Splat, depending on the element types and values.
    ///
    /// @param type the composite type
    /// @param elements the composite elements
    /// @returns the node pointer
    template <
        typename T,
        typename = utils::traits::EnableIf<utils::traits::IsTypeOrDerived<T, constant::Composite> ||
                                           utils::traits::IsTypeOrDerived<T, constant::Splat>>>
    const constant::Value* create(const type::Type* type,
                                  utils::VectorRef<const constant::Value*> elements) {
        AssertNotMoved();
        return createSplatOrComposite(type, elements);
    }

    /// Constructs a splat constant.
    /// @param type the splat type
    /// @param element the splat element
    /// @param n the number of elements
    /// @returns the node pointer
    template <
        typename T,
        typename = utils::traits::EnableIf<utils::traits::IsTypeOrDerived<T, constant::Splat>>>
    const constant::Splat* create(const type::Type* type,
                                  const constant::Value* element,
                                  size_t n) {
        AssertNotMoved();
        return constant_nodes_.Create<constant::Splat>(type, element, n);
    }

    /// Creates a new type::Node owned by the ProgramBuilder.
    /// When the ProgramBuilder is destructed, owned ProgramBuilder and the returned node will also
    /// be destructed. If T derives from type::UniqueNode, then the calling create() for the same
    /// `T` and arguments will return the same pointer.
    /// @param args the arguments to pass to the constructor
    /// @returns the new, or existing node
    template <typename T, typename... ARGS>
    utils::traits::EnableIfIsType<T, type::Node>* create(ARGS&&... args) {
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

        /// @return the C type `T`.
        template <typename T>
        ast::Type Of() const {
            return CToAST<T>::get(this);
        }

        /// @param type the type to return
        /// @return type (passthrough)
        ast::Type operator()(const ast::Type& type) const { return type; }

        /// Creates a type
        /// @param name the name
        /// @param args the optional template arguments
        /// @returns the type
        template <typename NAME,
                  typename... ARGS,
                  typename = DisableIfSource<NAME>,
                  typename = std::enable_if_t<!std::is_same_v<std::decay_t<NAME>, ast::Type>>>
        ast::Type operator()(NAME&& name, ARGS&&... args) const {
            if constexpr (utils::traits::IsTypeOrDerived<utils::traits::PtrElTy<NAME>,
                                                         ast::Expression>) {
                static_assert(sizeof...(ARGS) == 0);
                return {name};
            } else {
                return {builder->Expr(
                    builder->Ident(std::forward<NAME>(name), std::forward<ARGS>(args)...))};
            }
        }

        /// Creates a type
        /// @param source the Source of the node
        /// @param name the name
        /// @param args the optional template arguments
        /// @returns the type
        template <typename NAME,
                  typename... ARGS,
                  typename = std::enable_if_t<!std::is_same_v<std::decay_t<NAME>, ast::Type>>>
        ast::Type operator()(const Source& source, NAME&& name, ARGS&&... args) const {
            return {builder->Expr(
                builder->Ident(source, std::forward<NAME>(name), std::forward<ARGS>(args)...))};
        }

        /// @returns a a nullptr expression wrapped in an ast::Type
        ast::Type void_() const { return ast::Type{}; }

        /// @returns a 'bool' type
        ast::Type bool_() const { return (*this)("bool"); }

        /// @param source the Source of the node
        /// @returns a 'bool' type
        ast::Type bool_(const Source& source) const { return (*this)(source, "bool"); }

        /// @returns a 'f16' type
        ast::Type f16() const { return (*this)("f16"); }

        /// @param source the Source of the node
        /// @returns a 'f16' type
        ast::Type f16(const Source& source) const { return (*this)(source, "f16"); }

        /// @returns a 'f32' type
        ast::Type f32() const { return (*this)("f32"); }

        /// @param source the Source of the node
        /// @returns a 'f32' type
        ast::Type f32(const Source& source) const { return (*this)(source, "f32"); }

        /// @returns a 'i32' type
        ast::Type i32() const { return (*this)("i32"); }

        /// @param source the Source of the node
        /// @returns a 'i32' type
        ast::Type i32(const Source& source) const { return (*this)(source, "i32"); }

        /// @returns a 'u32' type
        ast::Type u32() const { return (*this)("u32"); }

        /// @param source the Source of the node
        /// @returns a 'u32' type
        ast::Type u32(const Source& source) const { return (*this)(source, "u32"); }

        /// @param type vector subtype
        /// @param n vector width in elements
        /// @return a @p n element vector of @p type
        ast::Type vec(ast::Type type, uint32_t n) const { return vec(builder->source_, type, n); }

        /// @param source the Source of the node
        /// @param type vector subtype
        /// @param n vector width in elements
        /// @return a @p n element vector of @p type
        ast::Type vec(const Source& source, ast::Type type, uint32_t n) const {
            switch (n) {
                case 2:
                    return vec2(source, type);
                case 3:
                    return vec3(source, type);
                case 4:
                    return vec4(source, type);
            }
            TINT_ICE(ProgramBuilder, builder->Diagnostics()) << "invalid vector width " << n;
            return ast::Type{};
        }

        /// @param type vector subtype
        /// @return a 2-element vector of @p type
        ast::Type vec2(ast::Type type) const { return vec2(builder->source_, type); }

        /// @param source the vector source
        /// @param type vector subtype
        /// @return a 2-element vector of @p type
        ast::Type vec2(const Source& source, ast::Type type) const {
            return (*this)(source, "vec2", type);
        }

        /// @param type vector subtype
        /// @return a 3-element vector of @p type
        ast::Type vec3(ast::Type type) const { return vec3(builder->source_, type); }

        /// @param source the vector source
        /// @param type vector subtype
        /// @return a 3-element vector of @p type
        ast::Type vec3(const Source& source, ast::Type type) const {
            return (*this)(source, "vec3", type);
        }

        /// @param type vector subtype
        /// @return a 4-element vector of @p type
        ast::Type vec4(ast::Type type) const { return vec4(builder->source_, type); }

        /// @param source the vector source
        /// @param type vector subtype
        /// @return a 4-element vector of @p type
        ast::Type vec4(const Source& source, ast::Type type) const {
            return (*this)(source, "vec4", type);
        }

        /// @param source the Source of the node
        /// @return a 2-element vector of the type `T`
        template <typename T>
        ast::Type vec2(const Source& source) const {
            if constexpr (IsInferOrAbstract<T>) {
                return (*this)(source, "vec2");
            } else {
                return (*this)(source, "vec2", Of<T>());
            }
        }

        /// @param source the Source of the node
        /// @return a 3-element vector of the type `T`
        template <typename T>
        ast::Type vec3(const Source& source) const {
            if constexpr (IsInferOrAbstract<T>) {
                return (*this)(source, "vec3");
            } else {
                return (*this)(source, "vec3", Of<T>());
            }
        }

        /// @param source the Source of the node
        /// @return a 4-element vector of the type `T`
        template <typename T>
        ast::Type vec4(const Source& source) const {
            if constexpr (IsInferOrAbstract<T>) {
                return (*this)(source, "vec4");
            } else {
                return (*this)(source, "vec4", Of<T>());
            }
        }

        /// @return a 2-element vector of the type `T`
        template <typename T>
        ast::Type vec2() const {
            return vec2<T>(builder->source_);
        }

        /// @return a 3-element vector of the type `T`
        template <typename T>
        ast::Type vec3() const {
            return vec3<T>(builder->source_);
        }

        /// @return a 4-element vector of the type `T`
        template <typename T>
        ast::Type vec4() const {
            return vec4<T>(builder->source_);
        }

        /// @param source the Source of the node
        /// @param n vector width in elements
        /// @return a @p n element vector of @p type
        template <typename T>
        ast::Type vec(const Source& source, uint32_t n) const {
            switch (n) {
                case 2:
                    return vec2<T>(source);
                case 3:
                    return vec3<T>(source);
                case 4:
                    return vec4<T>(source);
            }
            TINT_ICE(ProgramBuilder, builder->Diagnostics()) << "invalid vector width " << n;
            return ast::Type{};
        }

        /// @return a @p N element vector of @p type
        template <typename T, uint32_t N>
        ast::Type vec() const {
            return vec<T>(builder->source_, N);
        }

        /// @param n vector width in elements
        /// @return a @p n element vector of @p type
        template <typename T>
        ast::Type vec(uint32_t n) const {
            return vec<T>(builder->source_, n);
        }

        /// @param type matrix subtype
        /// @param columns number of columns for the matrix
        /// @param rows number of rows for the matrix
        /// @return a matrix of @p type
        ast::Type mat(ast::Type type, uint32_t columns, uint32_t rows) const {
            return mat(builder->source_, type, columns, rows);
        }

        /// @param source the Source of the node
        /// @param type matrix subtype
        /// @param columns number of columns for the matrix
        /// @param rows number of rows for the matrix
        /// @return a matrix of @p type
        ast::Type mat(const Source& source, ast::Type type, uint32_t columns, uint32_t rows) const {
            if (TINT_LIKELY(columns >= 2 && columns <= 4 && rows >= 2 && rows <= 4)) {
                static constexpr const char* names[] = {
                    "mat2x2", "mat2x3", "mat2x4",  //
                    "mat3x2", "mat3x3", "mat3x4",  //
                    "mat4x2", "mat4x3", "mat4x4",  //
                };
                auto i = (columns - 2) * 3 + (rows - 2);
                return (*this)(source, names[i], type);
            }
            TINT_ICE(ProgramBuilder, builder->Diagnostics())
                << "invalid matrix dimensions " << columns << "x" << rows;
            return ast::Type{};
        }

        /// @param type matrix subtype
        /// @return a 2x3 matrix of @p type.
        ast::Type mat2x2(ast::Type type) const { return (*this)("mat2x2", type); }

        /// @param type matrix subtype
        /// @return a 2x3 matrix of @p type.
        ast::Type mat2x3(ast::Type type) const { return (*this)("mat2x3", type); }

        /// @param type matrix subtype
        /// @return a 2x4 matrix of @p type.
        ast::Type mat2x4(ast::Type type) const { return (*this)("mat2x4", type); }

        /// @param type matrix subtype
        /// @return a 3x2 matrix of @p type.
        ast::Type mat3x2(ast::Type type) const { return (*this)("mat3x2", type); }

        /// @param type matrix subtype
        /// @return a 3x3 matrix of @p type.
        ast::Type mat3x3(ast::Type type) const { return (*this)("mat3x3", type); }

        /// @param type matrix subtype
        /// @return a 3x4 matrix of @p type.
        ast::Type mat3x4(ast::Type type) const { return (*this)("mat3x4", type); }

        /// @param type matrix subtype
        /// @return a 4x2 matrix of @p type.
        ast::Type mat4x2(ast::Type type) const { return (*this)("mat4x2", type); }

        /// @param type matrix subtype
        /// @return a 4x3 matrix of @p type.
        ast::Type mat4x3(ast::Type type) const { return (*this)("mat4x3", type); }

        /// @param type matrix subtype
        /// @return a 4x4 matrix of @p type.
        ast::Type mat4x4(ast::Type type) const { return (*this)("mat4x4", type); }

        /// @param source the source of the type
        /// @return a 2x2 matrix of the type `T`
        template <typename T>
        ast::Type mat2x2(const Source& source) const {
            if constexpr (IsInferOrAbstract<T>) {
                return (*this)(source, "mat2x2");
            } else {
                return (*this)(source, "mat2x2", Of<T>());
            }
        }

        /// @param source the source of the type
        /// @return a 2x3 matrix of the type `T`
        template <typename T>
        ast::Type mat2x3(const Source& source) const {
            if constexpr (IsInferOrAbstract<T>) {
                return (*this)(source, "mat2x3");
            } else {
                return (*this)(source, "mat2x3", Of<T>());
            }
        }

        /// @param source the source of the type
        /// @return a 2x4 matrix of the type `T`
        template <typename T>
        ast::Type mat2x4(const Source& source) const {
            if constexpr (IsInferOrAbstract<T>) {
                return (*this)(source, "mat2x4");
            } else {
                return (*this)(source, "mat2x4", Of<T>());
            }
        }

        /// @param source the source of the type
        /// @return a 3x2 matrix of the type `T`
        template <typename T>
        ast::Type mat3x2(const Source& source) const {
            if constexpr (IsInferOrAbstract<T>) {
                return (*this)(source, "mat3x2");
            } else {
                return (*this)(source, "mat3x2", Of<T>());
            }
        }

        /// @param source the source of the type
        /// @return a 3x3 matrix of the type `T`
        template <typename T>
        ast::Type mat3x3(const Source& source) const {
            if constexpr (IsInferOrAbstract<T>) {
                return (*this)(source, "mat3x3");
            } else {
                return (*this)(source, "mat3x3", Of<T>());
            }
        }

        /// @param source the source of the type
        /// @return a 3x4 matrix of the type `T`
        template <typename T>
        ast::Type mat3x4(const Source& source) const {
            if constexpr (IsInferOrAbstract<T>) {
                return (*this)(source, "mat3x4");
            } else {
                return (*this)(source, "mat3x4", Of<T>());
            }
        }

        /// @param source the source of the type
        /// @return a 4x2 matrix of the type `T`
        template <typename T>
        ast::Type mat4x2(const Source& source) const {
            if constexpr (IsInferOrAbstract<T>) {
                return (*this)(source, "mat4x2");
            } else {
                return (*this)(source, "mat4x2", Of<T>());
            }
        }

        /// @param source the source of the type
        /// @return a 4x3 matrix of the type `T`
        template <typename T>
        ast::Type mat4x3(const Source& source) const {
            if constexpr (IsInferOrAbstract<T>) {
                return (*this)(source, "mat4x3");
            } else {
                return (*this)(source, "mat4x3", Of<T>());
            }
        }

        /// @param source the source of the type
        /// @return a 4x4 matrix of the type `T`
        template <typename T>
        ast::Type mat4x4(const Source& source) const {
            if constexpr (IsInferOrAbstract<T>) {
                return (*this)(source, "mat4x4");
            } else {
                return (*this)(source, "mat4x4", Of<T>());
            }
        }

        /// @return a 2x2 matrix of the type `T`
        template <typename T>
        ast::Type mat2x2() const {
            return mat2x2<T>(builder->source_);
        }

        /// @return a 2x3 matrix of the type `T`
        template <typename T>
        ast::Type mat2x3() const {
            return mat2x3<T>(builder->source_);
        }

        /// @return a 2x4 matrix of the type `T`
        template <typename T>
        ast::Type mat2x4() const {
            return mat2x4<T>(builder->source_);
        }

        /// @return a 3x2 matrix of the type `T`
        template <typename T>
        ast::Type mat3x2() const {
            return mat3x2<T>(builder->source_);
        }

        /// @return a 3x3 matrix of the type `T`
        template <typename T>
        ast::Type mat3x3() const {
            return mat3x3<T>(builder->source_);
        }

        /// @return a 3x4 matrix of the type `T`
        template <typename T>
        ast::Type mat3x4() const {
            return mat3x4<T>(builder->source_);
        }

        /// @return a 4x2 matrix of the type `T`
        template <typename T>
        ast::Type mat4x2() const {
            return mat4x2<T>(builder->source_);
        }

        /// @return a 4x3 matrix of the type `T`
        template <typename T>
        ast::Type mat4x3() const {
            return mat4x3<T>(builder->source_);
        }

        /// @return a 4x4 matrix of the type `T`
        template <typename T>
        ast::Type mat4x4() const {
            return mat4x4<T>(builder->source_);
        }

        /// @param source the Source of the node
        /// @param columns number of columns for the matrix
        /// @param rows number of rows for the matrix
        /// @return a matrix of @p type
        template <typename T>
        ast::Type mat(const Source& source, uint32_t columns, uint32_t rows) const {
            switch ((columns - 2) * 3 + (rows - 2)) {
                case 0:
                    return mat2x2<T>(source);
                case 1:
                    return mat2x3<T>(source);
                case 2:
                    return mat2x4<T>(source);
                case 3:
                    return mat3x2<T>(source);
                case 4:
                    return mat3x3<T>(source);
                case 5:
                    return mat3x4<T>(source);
                case 6:
                    return mat4x2<T>(source);
                case 7:
                    return mat4x3<T>(source);
                case 8:
                    return mat4x4<T>(source);
                default:
                    TINT_ICE(ProgramBuilder, builder->Diagnostics())
                        << "invalid matrix dimensions " << columns << "x" << rows;
                    return ast::Type{};
            }
        }

        /// @param columns number of columns for the matrix
        /// @param rows number of rows for the matrix
        /// @return a matrix of @p type
        template <typename T>
        ast::Type mat(uint32_t columns, uint32_t rows) const {
            return mat<T>(builder->source_, columns, rows);
        }

        /// @return a matrix of @p type
        template <typename T, uint32_t COLUMNS, uint32_t ROWS>
        ast::Type mat() const {
            return mat<T>(builder->source_, COLUMNS, ROWS);
        }

        /// @param subtype the array element type
        /// @param attrs the optional attributes for the array
        /// @return an array of type `T`
        ast::Type array(ast::Type subtype,
                        utils::VectorRef<const ast::Attribute*> attrs = utils::Empty) const {
            return array(builder->source_, subtype, std::move(attrs));
        }

        /// @param subtype the array element type
        /// @param n the array size. nullptr represents a runtime-array
        /// @param attrs the optional attributes for the array
        /// @return an array of size `n` of type `T`
        template <typename COUNT, typename = DisableIfVectorLike<COUNT>>
        ast::Type array(ast::Type subtype,
                        COUNT&& n,
                        utils::VectorRef<const ast::Attribute*> attrs = utils::Empty) const {
            return array(builder->source_, subtype, std::forward<COUNT>(n), std::move(attrs));
        }

        /// @param source the Source of the node
        /// @param subtype the array element type
        /// @param attrs the optional attributes for the array
        /// @return an array of type `T`
        ast::Type array(const Source& source,
                        ast::Type subtype,
                        utils::VectorRef<const ast::Attribute*> attrs = utils::Empty) const {
            return ast::Type{builder->Expr(
                builder->create<ast::TemplatedIdentifier>(source, builder->Sym("array"),
                                                          utils::Vector{
                                                              subtype.expr,
                                                          },
                                                          std::move(attrs)))};
        }

        /// @param source the Source of the node
        /// @param subtype the array element type
        /// @param n the array size. nullptr represents a runtime-array
        /// @param attrs the optional attributes for the array
        /// @return an array of size `n` of type `T`
        template <typename COUNT, typename = DisableIfVectorLike<COUNT>>
        ast::Type array(const Source& source,
                        ast::Type subtype,
                        COUNT&& n,
                        utils::VectorRef<const ast::Attribute*> attrs = utils::Empty) const {
            return ast::Type{builder->Expr(
                builder->create<ast::TemplatedIdentifier>(source, builder->Sym("array"),
                                                          utils::Vector{
                                                              subtype.expr,
                                                              builder->Expr(std::forward<COUNT>(n)),
                                                          },
                                                          std::move(attrs)))};
        }

        /// @param source the Source of the node
        /// @return a inferred-size or runtime-sized array of type `T`
        template <typename T, typename = EnableIfInferOrAbstract<T>>
        ast::Type array(const Source& source) const {
            return (*this)(source, "array");
        }

        /// @return a inferred-size or runtime-sized array of type `T`
        template <typename T, typename = EnableIfInferOrAbstract<T>>
        ast::Type array() const {
            return array<T>(builder->source_);
        }

        /// @param source the Source of the node
        /// @param attrs the optional attributes for the array
        /// @return a inferred-size or runtime-sized array of type `T`
        template <typename T, typename = DisableIfInferOrAbstract<T>>
        ast::Type array(const Source& source,
                        utils::VectorRef<const ast::Attribute*> attrs = utils::Empty) const {
            return ast::Type{builder->Expr(
                builder->create<ast::TemplatedIdentifier>(source, builder->Sym("array"),
                                                          utils::Vector<const ast::Expression*, 1>{
                                                              Of<T>().expr,
                                                          },
                                                          std::move(attrs)))};
        }

        /// @param attrs the optional attributes for the array
        /// @return a inferred-size or runtime-sized array of type `T`
        template <typename T, typename = DisableIfInferOrAbstract<T>>
        ast::Type array(utils::VectorRef<const ast::Attribute*> attrs = utils::Empty) const {
            return array<T>(builder->source_, std::move(attrs));
        }

        /// @param source the Source of the node
        /// @param attrs the optional attributes for the array
        /// @return an array of size `N` of type `T`
        template <typename T, int N>
        ast::Type array(const Source& source,
                        utils::VectorRef<const ast::Attribute*> attrs = utils::Empty) const {
            static_assert(!IsInferOrAbstract<T>, "arrays with a count cannot be inferred");
            return array(source, Of<T>(), tint::u32(N), std::move(attrs));
        }

        /// @param attrs the optional attributes for the array
        /// @return an array of size `N` of type `T`
        template <typename T, int N>
        ast::Type array(utils::VectorRef<const ast::Attribute*> attrs = utils::Empty) const {
            static_assert(!IsInferOrAbstract<T>, "arrays with a count cannot be inferred");
            return array<T, N>(builder->source_, std::move(attrs));
        }

        /// Creates an alias type
        /// @param name the alias name
        /// @param type the alias type
        /// @returns the alias pointer
        template <typename NAME>
        const ast::Alias* alias(NAME&& name, ast::Type type) const {
            return alias(builder->source_, std::forward<NAME>(name), type);
        }

        /// Creates an alias type
        /// @param source the Source of the node
        /// @param name the alias name
        /// @param type the alias type
        /// @returns the alias pointer
        template <typename NAME>
        const ast::Alias* alias(const Source& source, NAME&& name, ast::Type type) const {
            return builder->create<ast::Alias>(source, builder->Ident(std::forward<NAME>(name)),
                                               type);
        }

        /// @param type the type of the pointer
        /// @param address_space the address space of the pointer
        /// @param access the optional access control of the pointer
        /// @return the pointer to `type` with the given builtin::AddressSpace
        ast::Type pointer(ast::Type type,
                          builtin::AddressSpace address_space,
                          builtin::Access access = builtin::Access::kUndefined) const {
            return pointer(builder->source_, type, address_space, access);
        }

        /// @param source the Source of the node
        /// @param type the type of the pointer
        /// @param address_space the address space of the pointer
        /// @param access the optional access control of the pointer
        /// @return the pointer to `type` with the given builtin::AddressSpace
        ast::Type pointer(const Source& source,
                          ast::Type type,
                          builtin::AddressSpace address_space,
                          builtin::Access access = builtin::Access::kUndefined) const {
            if (access != builtin::Access::kUndefined) {
                return (*this)(source, "ptr", address_space, type, access);
            } else {
                return (*this)(source, "ptr", address_space, type);
            }
        }

        /// @param address_space the address space of the pointer
        /// @param access the optional access control of the pointer
        /// @return the pointer to type `T` with the given builtin::AddressSpace.
        template <typename T>
        ast::Type pointer(builtin::AddressSpace address_space,
                          builtin::Access access = builtin::Access::kUndefined) const {
            return pointer<T>(builder->source_, address_space, access);
        }

        /// @param source the Source of the node
        /// @param address_space the address space of the pointer
        /// @param access the optional access control of the pointer
        /// @return the pointer to type `T` with the given builtin::AddressSpace.
        template <typename T>
        ast::Type pointer(const Source& source,
                          builtin::AddressSpace address_space,
                          builtin::Access access = builtin::Access::kUndefined) const {
            if (access != builtin::Access::kUndefined) {
                return (*this)(source, "ptr", address_space, Of<T>(), access);
            } else {
                return (*this)(source, "ptr", address_space, Of<T>());
            }
        }

        /// @param source the Source of the node
        /// @param type the type of the atomic
        /// @return the atomic to `type`
        ast::Type atomic(const Source& source, ast::Type type) const {
            return (*this)(source, "atomic", type);
        }

        /// @param type the type of the atomic
        /// @return the atomic to `type`
        ast::Type atomic(ast::Type type) const { return (*this)("atomic", type); }

        /// @return the atomic to type `T`
        template <typename T>
        ast::Type atomic() const {
            return atomic(Of<T>());
        }

        /// @param kind the kind of sampler
        /// @returns the sampler
        ast::Type sampler(type::SamplerKind kind) const { return sampler(builder->source_, kind); }

        /// @param source the Source of the node
        /// @param kind the kind of sampler
        /// @returns the sampler
        ast::Type sampler(const Source& source, type::SamplerKind kind) const {
            switch (kind) {
                case type::SamplerKind::kSampler:
                    return (*this)(source, "sampler");
                case type::SamplerKind::kComparisonSampler:
                    return (*this)(source, "sampler_comparison");
            }
            TINT_ICE(ProgramBuilder, builder->Diagnostics()) << "invalid sampler kind " << kind;
            return ast::Type{};
        }

        /// @param dims the dimensionality of the texture
        /// @returns the depth texture
        ast::Type depth_texture(type::TextureDimension dims) const {
            return depth_texture(builder->source_, dims);
        }

        /// @param source the Source of the node
        /// @param dims the dimensionality of the texture
        /// @returns the depth texture
        ast::Type depth_texture(const Source& source, type::TextureDimension dims) const {
            switch (dims) {
                case type::TextureDimension::k2d:
                    return (*this)(source, "texture_depth_2d");
                case type::TextureDimension::k2dArray:
                    return (*this)(source, "texture_depth_2d_array");
                case type::TextureDimension::kCube:
                    return (*this)(source, "texture_depth_cube");
                case type::TextureDimension::kCubeArray:
                    return (*this)(source, "texture_depth_cube_array");
                default:
                    break;
            }
            TINT_ICE(ProgramBuilder, builder->Diagnostics())
                << "invalid depth_texture dimensions: " << dims;
            return ast::Type{};
        }

        /// @param dims the dimensionality of the texture
        /// @returns the multisampled depth texture
        ast::Type depth_multisampled_texture(type::TextureDimension dims) const {
            return depth_multisampled_texture(builder->source_, dims);
        }

        /// @param source the Source of the node
        /// @param dims the dimensionality of the texture
        /// @returns the multisampled depth texture
        ast::Type depth_multisampled_texture(const Source& source,
                                             type::TextureDimension dims) const {
            if (dims == type::TextureDimension::k2d) {
                return (*this)(source, "texture_depth_multisampled_2d");
            }
            TINT_ICE(ProgramBuilder, builder->Diagnostics())
                << "invalid depth_multisampled_texture dimensions: " << dims;
            return ast::Type{};
        }

        /// @param dims the dimensionality of the texture
        /// @param subtype the texture subtype.
        /// @returns the sampled texture
        ast::Type sampled_texture(type::TextureDimension dims, ast::Type subtype) const {
            return sampled_texture(builder->source_, dims, subtype);
        }

        /// @param source the Source of the node
        /// @param dims the dimensionality of the texture
        /// @param subtype the texture subtype.
        /// @returns the sampled texture
        ast::Type sampled_texture(const Source& source,
                                  type::TextureDimension dims,
                                  ast::Type subtype) const {
            switch (dims) {
                case type::TextureDimension::k1d:
                    return (*this)(source, "texture_1d", subtype);
                case type::TextureDimension::k2d:
                    return (*this)(source, "texture_2d", subtype);
                case type::TextureDimension::k3d:
                    return (*this)(source, "texture_3d", subtype);
                case type::TextureDimension::k2dArray:
                    return (*this)(source, "texture_2d_array", subtype);
                case type::TextureDimension::kCube:
                    return (*this)(source, "texture_cube", subtype);
                case type::TextureDimension::kCubeArray:
                    return (*this)(source, "texture_cube_array", subtype);
                default:
                    break;
            }
            TINT_ICE(ProgramBuilder, builder->Diagnostics())
                << "invalid sampled_texture dimensions: " << dims;
            return ast::Type{};
        }

        /// @param dims the dimensionality of the texture
        /// @param subtype the texture subtype.
        /// @returns the multisampled texture
        ast::Type multisampled_texture(type::TextureDimension dims, ast::Type subtype) const {
            return multisampled_texture(builder->source_, dims, subtype);
        }

        /// @param source the Source of the node
        /// @param dims the dimensionality of the texture
        /// @param subtype the texture subtype.
        /// @returns the multisampled texture
        ast::Type multisampled_texture(const Source& source,
                                       type::TextureDimension dims,
                                       ast::Type subtype) const {
            if (dims == type::TextureDimension::k2d) {
                return (*this)(source, "texture_multisampled_2d", subtype);
            }
            TINT_ICE(ProgramBuilder, builder->Diagnostics())
                << "invalid multisampled_texture dimensions: " << dims;
            return ast::Type{};
        }

        /// @param dims the dimensionality of the texture
        /// @param format the texel format of the texture
        /// @param access the access control of the texture
        /// @returns the storage texture
        ast::Type storage_texture(type::TextureDimension dims,
                                  builtin::TexelFormat format,
                                  builtin::Access access) const {
            return storage_texture(builder->source_, dims, format, access);
        }

        /// @param source the Source of the node
        /// @param dims the dimensionality of the texture
        /// @param format the texel format of the texture
        /// @param access the access control of the texture
        /// @returns the storage texture
        ast::Type storage_texture(const Source& source,
                                  type::TextureDimension dims,
                                  builtin::TexelFormat format,
                                  builtin::Access access) const {
            switch (dims) {
                case type::TextureDimension::k1d:
                    return (*this)(source, "texture_storage_1d", format, access);
                case type::TextureDimension::k2d:
                    return (*this)(source, "texture_storage_2d", format, access);
                case type::TextureDimension::k2dArray:
                    return (*this)(source, "texture_storage_2d_array", format, access);
                case type::TextureDimension::k3d:
                    return (*this)(source, "texture_storage_3d", format, access);
                default:
                    break;
            }
            TINT_ICE(ProgramBuilder, builder->Diagnostics())
                << "invalid storage_texture  dimensions: " << dims;
            return ast::Type{};
        }

        /// @returns the external texture
        ast::Type external_texture() const { return (*this)("texture_external"); }

        /// @param source the Source of the node
        /// @returns the external texture
        ast::Type external_texture(const Source& source) const {
            return (*this)(source, "texture_external");
        }

        /// @param type the type
        /// @return an ast::Type of the type declaration.
        ast::Type Of(const ast::TypeDecl* type) const { return (*this)(type->name->symbol); }

        /// The ProgramBuilder
        ProgramBuilder* const builder;

      private:
        /// CToAST<T> is specialized for various `T` types and each specialization
        /// contains a single static `get()` method for obtaining the corresponding
        /// AST type for the C type `T`.
        /// `get()` has the signature:
        ///    `static ast::Type get(Types* t)`
        template <typename T>
        struct CToAST {};
    };

    //////////////////////////////////////////////////////////////////////////////
    // AST helper methods
    //////////////////////////////////////////////////////////////////////////////

    /// @return a new unnamed symbol
    Symbol Sym() { return Symbols().New(); }

    /// Passthrough
    /// @param sym the symbol
    /// @return `sym`
    Symbol Sym(Symbol sym) { return sym; }

    /// @param name the symbol string
    /// @return a Symbol with the given name
    Symbol Sym(std::string_view name) { return Symbols().Register(name); }

    /// @param enumerator the enumerator
    /// @return a Symbol with the given enum value
    template <typename ENUM, typename = std::enable_if_t<std::is_enum_v<std::decay_t<ENUM>>>>
    Symbol Sym(ENUM&& enumerator) {
        return Sym(utils::ToString(enumerator));
    }

    /// @return nullptr
    const ast::Identifier* Ident(std::nullptr_t) { return nullptr; }

    /// @param identifier the identifier symbol
    /// @return an ast::Identifier with the given symbol
    template <typename IDENTIFIER>
    const ast::Identifier* Ident(IDENTIFIER&& identifier) {
        if constexpr (utils::traits::IsTypeOrDerived<utils::traits::PtrElTy<IDENTIFIER>,
                                                     ast::Identifier>) {
            return identifier;  // Passthrough
        } else {
            return Ident(source_, std::forward<IDENTIFIER>(identifier));
        }
    }

    /// @param source the source information
    /// @param identifier the identifier symbol
    /// @return an ast::Identifier with the given symbol
    template <typename IDENTIFIER>
    const ast::Identifier* Ident(const Source& source, IDENTIFIER&& identifier) {
        return create<ast::Identifier>(source, Sym(std::forward<IDENTIFIER>(identifier)));
    }

    /// @param identifier the identifier symbol
    /// @param args the templated identifier arguments
    /// @return an ast::Identifier with the given symbol and template arguments
    template <typename IDENTIFIER, typename... ARGS, typename = DisableIfSource<IDENTIFIER>>
    const ast::Identifier* Ident(IDENTIFIER&& identifier, ARGS&&... args) {
        return Ident(source_, std::forward<IDENTIFIER>(identifier), std::forward<ARGS>(args)...);
    }

    /// @param source the source information
    /// @param identifier the identifier symbol
    /// @param args the templated identifier arguments
    /// @return an ast::Identifier with the given symbol and template arguments
    template <typename IDENTIFIER, typename... ARGS>
    const ast::Identifier* Ident(const Source& source, IDENTIFIER&& identifier, ARGS&&... args) {
        auto arg_exprs = ExprList(std::forward<ARGS>(args)...);
        if (arg_exprs.IsEmpty()) {
            return create<ast::Identifier>(source, Sym(std::forward<IDENTIFIER>(identifier)));
        }
        return create<ast::TemplatedIdentifier>(source, Sym(std::forward<IDENTIFIER>(identifier)),
                                                std::move(arg_exprs), utils::Empty);
    }

    /// @param expr the expression
    /// @return expr (passthrough)
    template <typename T, typename = utils::traits::EnableIfIsType<T, ast::Expression>>
    const T* Expr(const T* expr) {
        return expr;
    }

    /// @param type an ast::Type
    /// @return type.expr
    const ast::IdentifierExpression* Expr(ast::Type type) { return type.expr; }

    /// @param ident the identifier
    /// @return an ast::IdentifierExpression with the given identifier
    const ast::IdentifierExpression* Expr(const ast::Identifier* ident) {
        return ident ? create<ast::IdentifierExpression>(ident->source, ident) : nullptr;
    }

    /// Passthrough for nullptr
    /// @return nullptr
    const ast::IdentifierExpression* Expr(std::nullptr_t) { return nullptr; }

    /// @param name the identifier name
    /// @return an ast::IdentifierExpression with the given name
    template <typename NAME, typename = EnableIfIdentifierLike<NAME>>
    const ast::IdentifierExpression* Expr(NAME&& name) {
        auto* ident = Ident(source_, name);
        return create<ast::IdentifierExpression>(ident->source, ident);
    }

    /// @param source the source information
    /// @param name the identifier name
    /// @return an ast::IdentifierExpression with the given name
    template <typename NAME, typename = EnableIfIdentifierLike<NAME>>
    const ast::IdentifierExpression* Expr(const Source& source, NAME&& name) {
        return create<ast::IdentifierExpression>(source, Ident(source, name));
    }

    /// @param variable the AST variable
    /// @return an ast::IdentifierExpression with the variable's symbol
    const ast::IdentifierExpression* Expr(const ast::Variable* variable) {
        auto* ident = Ident(variable->source, variable->name->symbol);
        return create<ast::IdentifierExpression>(ident->source, ident);
    }

    /// @param source the source information
    /// @param variable the AST variable
    /// @return an ast::IdentifierExpression with the variable's symbol
    const ast::IdentifierExpression* Expr(const Source& source, const ast::Variable* variable) {
        return create<ast::IdentifierExpression>(source, Ident(source, variable->name->symbol));
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

    /// @param source the source information
    /// @param value the float value
    /// @return a 'f'-suffixed FloatLiteralExpression for the f32 value
    const ast::FloatLiteralExpression* Expr(const Source& source, f32 value) {
        return create<ast::FloatLiteralExpression>(source, static_cast<double>(value.value),
                                                   ast::FloatLiteralExpression::Suffix::kF);
    }

    /// @param source the source information
    /// @param value the float value
    /// @return a 'h'-suffixed FloatLiteralExpression for the f16 value
    const ast::FloatLiteralExpression* Expr(const Source& source, f16 value) {
        return create<ast::FloatLiteralExpression>(source, static_cast<double>(value.value),
                                                   ast::FloatLiteralExpression::Suffix::kH);
    }

    /// @param source the source information
    /// @param value the integer value
    /// @return an unsuffixed IntLiteralExpression for the AInt value
    const ast::IntLiteralExpression* Expr(const Source& source, AInt value) {
        return create<ast::IntLiteralExpression>(source, value,
                                                 ast::IntLiteralExpression::Suffix::kNone);
    }

    /// @param source the source information
    /// @param value the integer value
    /// @return an unsuffixed FloatLiteralExpression for the AFloat value
    const ast::FloatLiteralExpression* Expr(const Source& source, AFloat value) {
        return create<ast::FloatLiteralExpression>(source, value.value,
                                                   ast::FloatLiteralExpression::Suffix::kNone);
    }

    /// @param source the source information
    /// @param value the integer value
    /// @return a signed 'i'-suffixed IntLiteralExpression for the i32 value
    const ast::IntLiteralExpression* Expr(const Source& source, i32 value) {
        return create<ast::IntLiteralExpression>(source, value,
                                                 ast::IntLiteralExpression::Suffix::kI);
    }

    /// @param source the source information
    /// @param value the unsigned int value
    /// @return an unsigned 'u'-suffixed IntLiteralExpression for the u32 value
    const ast::IntLiteralExpression* Expr(const Source& source, u32 value) {
        return create<ast::IntLiteralExpression>(source, value,
                                                 ast::IntLiteralExpression::Suffix::kU);
    }

    /// @param value the scalar value
    /// @return literal expression of the appropriate type
    template <typename SCALAR, typename = EnableIfScalar<SCALAR>>
    const auto* Expr(SCALAR&& value) {
        return Expr(source_, std::forward<SCALAR>(value));
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
        return utils::Vector<const ast::Expression*, sizeof...(ARGS)>{Expr(args)...};
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

    /// @param expr the expression for the bitcast
    /// @return an `ast::BitcastExpression` of type `ty`, with the values of
    /// `expr` converted to `ast::Expression`s using `Expr()`
    template <typename T, typename EXPR>
    const ast::BitcastExpression* Bitcast(EXPR&& expr) {
        return Bitcast(ty.Of<T>(), std::forward<EXPR>(expr));
    }

    /// @param type the type to cast to
    /// @param expr the expression for the bitcast
    /// @return an `ast::BitcastExpression` of @p type constructed with the values
    /// `expr`.
    template <typename EXPR>
    const ast::BitcastExpression* Bitcast(ast::Type type, EXPR&& expr) {
        return Bitcast(source_, type, Expr(std::forward<EXPR>(expr)));
    }

    /// @param source the source information
    /// @param type the type to cast to
    /// @param expr the expression for the bitcast
    /// @return an `ast::BitcastExpression` of @p type constructed with the values
    /// `expr`.
    template <typename EXPR>
    const ast::BitcastExpression* Bitcast(const Source& source, ast::Type type, EXPR&& expr) {
        return create<ast::BitcastExpression>(source, type, Expr(std::forward<EXPR>(expr)));
    }

    /// @param type the vector type
    /// @param size the vector size
    /// @param args the arguments for the vector constructor
    /// @return an `ast::CallExpression` of a `size`-element vector of
    /// type `type`, constructed with the values @p args.
    template <typename... ARGS>
    const ast::CallExpression* vec(ast::Type type, uint32_t size, ARGS&&... args) {
        return vec(source_, type, size, std::forward<ARGS>(args)...);
    }

    /// @param source the source of the call
    /// @param type the vector type
    /// @param size the vector size
    /// @param args the arguments for the vector constructor
    /// @return an `ast::CallExpression` of a `size`-element vector of
    /// type `type`, constructed with the values @p args.
    template <typename... ARGS>
    const ast::CallExpression* vec(const Source& source,
                                   ast::Type type,
                                   uint32_t size,
                                   ARGS&&... args) {
        return Call(source, ty.vec(type, size), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the vector constructor
    /// @return an `ast::CallExpression` of a 2-element vector of type `T`, constructed with the
    /// values @p args.
    template <typename T, typename... ARGS, typename = DisableIfSource<ARGS...>>
    const ast::CallExpression* vec2(ARGS&&... args) {
        return vec2<T>(source_, std::forward<ARGS>(args)...);
    }

    /// @param source the vector source
    /// @param args the arguments for the vector constructor
    /// @return an `ast::CallExpression` of a 2-element vector of type `T`, constructed with the
    /// values @p args.
    template <typename T, typename... ARGS>
    const ast::CallExpression* vec2(const Source& source, ARGS&&... args) {
        return Call(source, ty.vec2<T>(), std::forward<ARGS>(args)...);
    }

    /// @param type the element type of the vector
    /// @param args the arguments for the vector constructor
    /// @return an `ast::CallExpression` of a 2-element vector of type @p type, constructed with the
    /// values @p args.
    template <typename... ARGS>
    const ast::CallExpression* vec2(ast::Type type, ARGS&&... args) {
        return vec2(source_, type, std::forward<ARGS>(args)...);
    }

    /// @param source the vector source
    /// @param type the element type of the vector
    /// @param args the arguments for the vector constructor
    /// @return an `ast::CallExpression` of a 2-element vector of type @p type, constructed with the
    /// values @p args.
    template <typename... ARGS>
    const ast::CallExpression* vec2(const Source& source, ast::Type type, ARGS&&... args) {
        return Call(source, ty.vec2(type), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the vector constructor
    /// @return an `ast::CallExpression` of a 3-element vector of type `T`, constructed with the
    /// values @p args.
    template <typename T, typename... ARGS, typename = DisableIfSource<ARGS...>>
    const ast::CallExpression* vec3(ARGS&&... args) {
        return vec3<T>(source_, std::forward<ARGS>(args)...);
    }

    /// @param source the vector source
    /// @param args the arguments for the vector constructor
    /// @return an `ast::CallExpression` of a 3-element vector of type `T`, constructed with the
    /// values @p args.
    template <typename T, typename... ARGS>
    const ast::CallExpression* vec3(const Source& source, ARGS&&... args) {
        return Call(source, ty.vec3<T>(), std::forward<ARGS>(args)...);
    }

    /// @param type the element type of the vector
    /// @param args the arguments for the vector constructor
    /// @return an `ast::CallExpression` of a 3-element vector of type @p type, constructed with the
    /// values @p args.
    template <typename... ARGS>
    const ast::CallExpression* vec3(ast::Type type, ARGS&&... args) {
        return vec3(source_, type, std::forward<ARGS>(args)...);
    }

    /// @param source the vector source
    /// @param type the element type of the vector
    /// @param args the arguments for the vector constructor
    /// @return an `ast::CallExpression` of a 3-element vector of type @p type, constructed with the
    /// values @p args.
    template <typename... ARGS>
    const ast::CallExpression* vec3(const Source& source, ast::Type type, ARGS&&... args) {
        return Call(source, ty.vec3(type), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the vector constructor
    /// @return an `ast::CallExpression` of a 4-element vector of type `T`, constructed with the
    /// values @p args.
    template <typename T, typename... ARGS, typename = DisableIfSource<ARGS...>>
    const ast::CallExpression* vec4(ARGS&&... args) {
        return vec4<T>(source_, std::forward<ARGS>(args)...);
    }

    /// @param source the vector source
    /// @param args the arguments for the vector constructor
    /// @return an `ast::CallExpression` of a 4-element vector of type `T`, constructed with the
    /// values @p args.
    template <typename T, typename... ARGS>
    const ast::CallExpression* vec4(const Source& source, ARGS&&... args) {
        return Call(source, ty.vec4<T>(), std::forward<ARGS>(args)...);
    }

    /// @param type the element type of the vector
    /// @param args the arguments for the vector constructor
    /// @return an `ast::CallExpression` of a 4-element vector of type @p type, constructed with the
    /// values @p args.
    template <typename... ARGS>
    const ast::CallExpression* vec4(ast::Type type, ARGS&&... args) {
        return vec4(source_, type, std::forward<ARGS>(args)...);
    }

    /// @param source the vector source
    /// @param type the element type of the vector
    /// @param args the arguments for the vector constructor
    /// @return an `ast::CallExpression` of a 4-element vector of type @p type, constructed with the
    /// values @p args.
    template <typename... ARGS>
    const ast::CallExpression* vec4(const Source& source, ast::Type type, ARGS&&... args) {
        return Call(source, ty.vec4(type), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the matrix constructor
    /// @return an `ast::CallExpression` of a 2x2 matrix of type
    /// `T`, constructed with the values @p args.
    template <typename T, typename... ARGS, typename = DisableIfSource<ARGS...>>
    const ast::CallExpression* mat2x2(ARGS&&... args) {
        return mat2x2<T>(source_, std::forward<ARGS>(args)...);
    }

    /// @param source the matrix source
    /// @param args the arguments for the matrix constructor
    /// @return an `ast::CallExpression` of a 2x2 matrix of type
    /// `T`, constructed with the values @p args.
    template <typename T, typename... ARGS>
    const ast::CallExpression* mat2x2(const Source& source, ARGS&&... args) {
        return Call(source, ty.mat2x2<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the matrix constructor
    /// @return an `ast::CallExpression` of a 2x3 matrix of type
    /// `T`, constructed with the values @p args.
    template <typename T, typename... ARGS, typename = DisableIfSource<ARGS...>>
    const ast::CallExpression* mat2x3(ARGS&&... args) {
        return mat2x3<T>(source_, std::forward<ARGS>(args)...);
    }

    /// @param source the matrix source
    /// @param args the arguments for the matrix constructor
    /// @return an `ast::CallExpression` of a 2x3 matrix of type
    /// `T`, constructed with the values @p args.
    template <typename T, typename... ARGS>
    const ast::CallExpression* mat2x3(const Source& source, ARGS&&... args) {
        return Call(source, ty.mat2x3<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the matrix constructor
    /// @return an `ast::CallExpression` of a 2x4 matrix of type
    /// `T`, constructed with the values @p args.
    template <typename T, typename... ARGS, typename = DisableIfSource<ARGS...>>
    const ast::CallExpression* mat2x4(ARGS&&... args) {
        return mat2x4<T>(source_, std::forward<ARGS>(args)...);
    }

    /// @param source the matrix source
    /// @param args the arguments for the matrix constructor
    /// @return an `ast::CallExpression` of a 2x4 matrix of type
    /// `T`, constructed with the values @p args.
    template <typename T, typename... ARGS>
    const ast::CallExpression* mat2x4(const Source& source, ARGS&&... args) {
        return Call(source, ty.mat2x4<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the matrix constructor
    /// @return an `ast::CallExpression` of a 3x2 matrix of type
    /// `T`, constructed with the values @p args.
    template <typename T, typename... ARGS, typename = DisableIfSource<ARGS...>>
    const ast::CallExpression* mat3x2(ARGS&&... args) {
        return mat3x2<T>(source_, std::forward<ARGS>(args)...);
    }

    /// @param source the matrix source
    /// @param args the arguments for the matrix constructor
    /// @return an `ast::CallExpression` of a 3x2 matrix of type
    /// `T`, constructed with the values @p args.
    template <typename T, typename... ARGS>
    const ast::CallExpression* mat3x2(const Source& source, ARGS&&... args) {
        return Call(source, ty.mat3x2<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the matrix constructor
    /// @return an `ast::CallExpression` of a 3x3 matrix of type
    /// `T`, constructed with the values @p args.
    template <typename T, typename... ARGS, typename = DisableIfSource<ARGS...>>
    const ast::CallExpression* mat3x3(ARGS&&... args) {
        return mat3x3<T>(source_, std::forward<ARGS>(args)...);
    }

    /// @param source the matrix source
    /// @param args the arguments for the matrix constructor
    /// @return an `ast::CallExpression` of a 3x3 matrix of type
    /// `T`, constructed with the values @p args.
    template <typename T, typename... ARGS>
    const ast::CallExpression* mat3x3(const Source& source, ARGS&&... args) {
        return Call(source, ty.mat3x3<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the matrix constructor
    /// @return an `ast::CallExpression` of a 3x4 matrix of type
    /// `T`, constructed with the values @p args.
    template <typename T, typename... ARGS, typename = DisableIfSource<ARGS...>>
    const ast::CallExpression* mat3x4(ARGS&&... args) {
        return mat3x4<T>(source_, std::forward<ARGS>(args)...);
    }

    /// @param source the matrix source
    /// @param args the arguments for the matrix constructor
    /// @return an `ast::CallExpression` of a 3x4 matrix of type
    /// `T`, constructed with the values @p args.
    template <typename T, typename... ARGS>
    const ast::CallExpression* mat3x4(const Source& source, ARGS&&... args) {
        return Call(source, ty.mat3x4<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the matrix constructor
    /// @return an `ast::CallExpression` of a 4x2 matrix of type
    /// `T`, constructed with the values @p args.
    template <typename T, typename... ARGS, typename = DisableIfSource<ARGS...>>
    const ast::CallExpression* mat4x2(ARGS&&... args) {
        return mat4x2<T>(source_, std::forward<ARGS>(args)...);
    }

    /// @param source the matrix source
    /// @param args the arguments for the matrix constructor
    /// @return an `ast::CallExpression` of a 4x2 matrix of type
    /// `T`, constructed with the values @p args.
    template <typename T, typename... ARGS>
    const ast::CallExpression* mat4x2(const Source& source, ARGS&&... args) {
        return Call(source, ty.mat4x2<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the matrix constructor
    /// @return an `ast::CallExpression` of a 4x3 matrix of type
    /// `T`, constructed with the values @p args.
    template <typename T, typename... ARGS, typename = DisableIfSource<ARGS...>>
    const ast::CallExpression* mat4x3(ARGS&&... args) {
        return mat4x3<T>(source_, std::forward<ARGS>(args)...);
    }

    /// @param source the matrix source
    /// @param args the arguments for the matrix constructor
    /// @return an `ast::CallExpression` of a 4x3 matrix of type
    /// `T`, constructed with the values @p args.
    template <typename T, typename... ARGS>
    const ast::CallExpression* mat4x3(const Source& source, ARGS&&... args) {
        return Call(source, ty.mat4x3<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the matrix constructor
    /// @return an `ast::CallExpression` of a 4x4 matrix of type
    /// `T`, constructed with the values @p args.
    template <typename T, typename... ARGS, typename = DisableIfSource<ARGS...>>
    const ast::CallExpression* mat4x4(ARGS&&... args) {
        return mat4x4<T>(source_, std::forward<ARGS>(args)...);
    }

    /// @param source the matrix source
    /// @param args the arguments for the matrix constructor
    /// @return an `ast::CallExpression` of a 4x4 matrix of type
    /// `T`, constructed with the values @p args.
    template <typename T, typename... ARGS>
    const ast::CallExpression* mat4x4(const Source& source, ARGS&&... args) {
        return Call(source, ty.mat4x4<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the array constructor
    /// @return an `ast::CallExpression` of an array with element type `T`, constructed with the
    /// values @p args.
    template <typename T, typename... ARGS, typename = DisableIfSource<ARGS...>>
    const ast::CallExpression* array(ARGS&&... args) {
        return Call(ty.array<T>(), std::forward<ARGS>(args)...);
    }

    /// @param source the array source
    /// @param args the arguments for the array constructor
    /// @return an `ast::CallExpression` of an array with element type `T`, constructed with the
    /// values @p args.
    template <typename T, typename... ARGS>
    const ast::CallExpression* array(const Source& source, ARGS&&... args) {
        return Call(source, ty.array<T>(), std::forward<ARGS>(args)...);
    }

    /// @param args the arguments for the array constructor
    /// @return an `ast::CallExpression` of an array with element type `T` and size `N`, constructed
    /// with the values @p args.
    template <typename T, int N, typename... ARGS, typename = DisableIfSource<ARGS...>>
    const ast::CallExpression* array(ARGS&&... args) {
        return Call(ty.array<T, N>(), std::forward<ARGS>(args)...);
    }

    /// @param source the array source
    /// @param args the arguments for the array constructor
    /// @return an `ast::CallExpression` of an array with element type `T` and size `N`, constructed
    /// with the values @p args.
    template <typename T, int N, typename... ARGS>
    const ast::CallExpression* array(const Source& source, ARGS&&... args) {
        return Call(source, ty.array<T, N>(), std::forward<ARGS>(args)...);
    }

    /// @param subtype the array element type
    /// @param n the array size. nullptr represents a runtime-array.
    /// @param args the arguments for the array constructor
    /// @return an `ast::CallExpression` of an array with element type
    /// `subtype`, constructed with the values @p args.
    template <typename EXPR, typename... ARGS>
    const ast::CallExpression* array(ast::Type subtype, EXPR&& n, ARGS&&... args) {
        return Call(ty.array(subtype, std::forward<EXPR>(n)), std::forward<ARGS>(args)...);
    }

    /// @param source the array source
    /// @param subtype the array element type
    /// @param n the array size. nullptr represents a runtime-array.
    /// @param args the arguments for the array constructor
    /// @return an `ast::CallExpression` of an array with element type
    /// `subtype`, constructed with the values @p args.
    template <typename EXPR, typename... ARGS>
    const ast::CallExpression* array(const Source& source,
                                     ast::Type subtype,
                                     EXPR&& n,
                                     ARGS&&... args) {
        return Call(source, ty.array(subtype, std::forward<EXPR>(n)), std::forward<ARGS>(args)...);
    }

    /// Adds the extension to the list of enable directives at the top of the module.
    /// @param extension the extension to enable
    /// @return an `ast::Enable` enabling the given extension.
    const ast::Enable* Enable(builtin::Extension extension) {
        auto* ext = create<ast::Extension>(extension);
        auto* enable = create<ast::Enable>(utils::Vector{ext});
        AST().AddEnable(enable);
        return enable;
    }

    /// Adds the extension to the list of enable directives at the top of the module.
    /// @param source the enable source
    /// @param extension the extension to enable
    /// @return an `ast::Enable` enabling the given extension.
    const ast::Enable* Enable(const Source& source, builtin::Extension extension) {
        auto* ext = create<ast::Extension>(source, extension);
        auto* enable = create<ast::Enable>(source, utils::Vector{ext});
        AST().AddEnable(enable);
        return enable;
    }

    /// @param name the variable name
    /// @param options the extra options passed to the ast::Var initializer
    /// Can be any of the following, in any order:
    ///   * ast::Type              - specifies the variable's type
    ///   * builtin::AddressSpace  - specifies the variable's address space
    ///   * builtin::Access        - specifies the variable's access control
    ///   * ast::Expression*       - specifies the variable's initializer expression
    ///   * ast::Attribute*        - specifies the variable's attributes (repeatable, or vector)
    /// Note that non-repeatable arguments of the same type will use the last argument's value.
    /// @returns a `ast::Var` with the given name, type and additional
    /// options
    template <typename NAME, typename... OPTIONS, typename = DisableIfSource<NAME>>
    const ast::Var* Var(NAME&& name, OPTIONS&&... options) {
        return Var(source_, std::forward<NAME>(name), std::forward<OPTIONS>(options)...);
    }

    /// @param source the variable source
    /// @param name the variable name
    /// @param options the extra options passed to the ast::Var initializer
    /// Can be any of the following, in any order:
    ///   * ast::Type              - specifies the variable's type
    ///   * builtin::AddressSpace  - specifies the variable's address space
    ///   * builtin::Access        - specifies the variable's access control
    ///   * ast::Expression*       - specifies the variable's initializer expression
    ///   * ast::Attribute*        - specifies the variable's attributes (repeatable, or vector)
    /// Note that non-repeatable arguments of the same type will use the last argument's value.
    /// @returns a `ast::Var` with the given name, address_space and type
    template <typename NAME, typename... OPTIONS>
    const ast::Var* Var(const Source& source, NAME&& name, OPTIONS&&... options) {
        VarOptions opts(*this, std::forward<OPTIONS>(options)...);
        return create<ast::Var>(source, Ident(std::forward<NAME>(name)), opts.type,
                                opts.address_space, opts.access, opts.initializer,
                                std::move(opts.attributes));
    }

    /// @param name the variable name
    /// @param options the extra options passed to the ast::Var initializer
    /// Can be any of the following, in any order:
    ///   * ast::Expression*    - specifies the variable's initializer expression (required)
    ///   * ast::Type           - specifies the variable's type
    ///   * ast::Attribute*     - specifies the variable's attributes (repeatable, or vector)
    /// Note that non-repeatable arguments of the same type will use the last argument's value.
    /// @returns an `ast::Const` with the given name, type and additional options
    template <typename NAME, typename... OPTIONS, typename = DisableIfSource<NAME>>
    const ast::Const* Const(NAME&& name, OPTIONS&&... options) {
        return Const(source_, std::forward<NAME>(name), std::forward<OPTIONS>(options)...);
    }

    /// @param source the variable source
    /// @param name the variable name
    /// @param options the extra options passed to the ast::Var initializer
    /// Can be any of the following, in any order:
    ///   * ast::Expression*    - specifies the variable's initializer expression (required)
    ///   * ast::Identifier*    - specifies the variable's type
    ///   * ast::Type           - specifies the variable's type
    ///   * ast::Attribute*     - specifies the variable's attributes (repeatable, or vector)
    /// Note that non-repeatable arguments of the same type will use the last argument's value.
    /// @returns an `ast::Const` with the given name, type and additional options
    template <typename NAME, typename... OPTIONS>
    const ast::Const* Const(const Source& source, NAME&& name, OPTIONS&&... options) {
        ConstOptions opts(std::forward<OPTIONS>(options)...);
        return create<ast::Const>(source, Ident(std::forward<NAME>(name)), opts.type,
                                  opts.initializer, std::move(opts.attributes));
    }

    /// @param name the variable name
    /// @param options the extra options passed to the ast::Var initializer
    /// Can be any of the following, in any order:
    ///   * ast::Expression*    - specifies the variable's initializer expression (required)
    ///   * ast::Type           - specifies the variable's type
    ///   * ast::Attribute*     - specifies the variable's attributes (repeatable, or vector)
    /// Note that non-repeatable arguments of the same type will use the last argument's value.
    /// @returns an `ast::Let` with the given name, type and additional options
    template <typename NAME, typename... OPTIONS, typename = DisableIfSource<NAME>>
    const ast::Let* Let(NAME&& name, OPTIONS&&... options) {
        return Let(source_, std::forward<NAME>(name), std::forward<OPTIONS>(options)...);
    }

    /// @param source the variable source
    /// @param name the variable name
    /// @param options the extra options passed to the ast::Var initializer
    /// Can be any of the following, in any order:
    ///   * ast::Expression*    - specifies the variable's initializer expression (required)
    ///   * ast::Type           - specifies the variable's type
    ///   * ast::Attribute*     - specifies the variable's attributes (repeatable, or vector)
    /// Note that non-repeatable arguments of the same type will use the last argument's value.
    /// @returns an `ast::Let` with the given name, type and additional options
    template <typename NAME, typename... OPTIONS>
    const ast::Let* Let(const Source& source, NAME&& name, OPTIONS&&... options) {
        LetOptions opts(std::forward<OPTIONS>(options)...);
        return create<ast::Let>(source, Ident(std::forward<NAME>(name)), opts.type,
                                opts.initializer, std::move(opts.attributes));
    }

    /// @param name the parameter name
    /// @param type the parameter type
    /// @param attributes optional parameter attributes
    /// @returns an `ast::Parameter` with the given name and type
    template <typename NAME>
    const ast::Parameter* Param(NAME&& name,
                                ast::Type type,
                                utils::VectorRef<const ast::Attribute*> attributes = utils::Empty) {
        return Param(source_, std::forward<NAME>(name), type, std::move(attributes));
    }

    /// @param source the parameter source
    /// @param name the parameter name
    /// @param type the parameter type
    /// @param attributes optional parameter attributes
    /// @returns an `ast::Parameter` with the given name and type
    template <typename NAME>
    const ast::Parameter* Param(const Source& source,
                                NAME&& name,
                                ast::Type type,
                                utils::VectorRef<const ast::Attribute*> attributes = utils::Empty) {
        return create<ast::Parameter>(source, Ident(std::forward<NAME>(name)), type,
                                      std::move(attributes));
    }

    /// @param name the variable name
    /// @param options the extra options passed to the ast::Var initializer
    /// Can be any of the following, in any order:
    ///   * ast::Type           - specifies the variable's type
    ///   * builtin::AddressSpace   - specifies the variable address space
    ///   * builtin::Access         - specifies the variable's access control
    ///   * ast::Expression*    - specifies the variable's initializer expression
    ///   * ast::Attribute*     - specifies the variable's attributes (repeatable, or vector)
    /// Note that non-repeatable arguments of the same type will use the last argument's value.
    /// @returns a new `ast::Var`, which is automatically registered as a global variable with the
    /// ast::Module.
    template <typename NAME, typename... OPTIONS, typename = DisableIfSource<NAME>>
    const ast::Var* GlobalVar(NAME&& name, OPTIONS&&... options) {
        return GlobalVar(source_, std::forward<NAME>(name), std::forward<OPTIONS>(options)...);
    }

    /// @param source the variable source
    /// @param name the variable name
    /// @param options the extra options passed to the ast::Var initializer
    /// Can be any of the following, in any order:
    ///   * ast::Type           - specifies the variable's type
    ///   * builtin::AddressSpace   - specifies the variable address space
    ///   * builtin::Access         - specifies the variable's access control
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
    ///   * ast::Type           - specifies the variable's type
    ///   * ast::Attribute*     - specifies the variable's attributes (repeatable, or vector)
    /// Note that non-repeatable arguments of the same type will use the last argument's value.
    /// @returns an `ast::Const` with the given name, type and additional options, which is
    /// automatically registered as a global variable with the ast::Module.
    template <typename NAME, typename... OPTIONS, typename = DisableIfSource<NAME>>
    const ast::Const* GlobalConst(NAME&& name, OPTIONS&&... options) {
        return GlobalConst(source_, std::forward<NAME>(name), std::forward<OPTIONS>(options)...);
    }

    /// @param source the variable source
    /// @param name the variable name
    /// @param options the extra options passed to the ast::Const initializer
    /// Can be any of the following, in any order:
    ///   * ast::Expression*    - specifies the variable's initializer expression (required)
    ///   * ast::Type           - specifies the variable's type
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
    ///   * ast::Type           - specifies the variable's type
    ///   * ast::Attribute*     - specifies the variable's attributes (repeatable, or vector)
    /// Note that non-repeatable arguments of the same type will use the last argument's value.
    /// @returns an `ast::Override` with the given name, type and additional options, which is
    /// automatically registered as a global variable with the ast::Module.
    template <typename NAME, typename... OPTIONS, typename = DisableIfSource<NAME>>
    const ast::Override* Override(NAME&& name, OPTIONS&&... options) {
        return Override(source_, std::forward<NAME>(name), std::forward<OPTIONS>(options)...);
    }

    /// @param source the variable source
    /// @param name the variable name
    /// @param options the extra options passed to the ast::Override initializer
    /// Can be any of the following, in any order:
    ///   * ast::Expression*    - specifies the variable's initializer expression (required)
    ///   * ast::Type           - specifies the variable's type
    ///   * ast::Attribute*     - specifies the variable's attributes (repeatable, or vector)
    /// Note that non-repeatable arguments of the same type will use the last argument's value.
    /// @returns an `ast::Override` with the given name, type and additional options, which is
    /// automatically registered as a global variable with the ast::Module.
    template <typename NAME, typename... OPTIONS>
    const ast::Override* Override(const Source& source, NAME&& name, OPTIONS&&... options) {
        OverrideOptions opts(std::forward<OPTIONS>(options)...);
        auto* variable = create<ast::Override>(source, Ident(std::forward<NAME>(name)), opts.type,
                                               opts.initializer, std::move(opts.attributes));
        AST().AddGlobalVariable(variable);
        return variable;
    }

    /// @param source the source information
    /// @param condition the assertion condition
    /// @returns a new `ast::ConstAssert`, which is automatically registered as a global statement
    /// with the ast::Module.
    template <typename EXPR>
    const ast::ConstAssert* GlobalConstAssert(const Source& source, EXPR&& condition) {
        auto* sa = ConstAssert(source, std::forward<EXPR>(condition));
        AST().AddConstAssert(sa);
        return sa;
    }

    /// @param condition the assertion condition
    /// @returns a new `ast::ConstAssert`, which is automatically registered as a global statement
    /// with the ast::Module.
    template <typename EXPR, typename = DisableIfSource<EXPR>>
    const ast::ConstAssert* GlobalConstAssert(EXPR&& condition) {
        auto* sa = ConstAssert(std::forward<EXPR>(condition));
        AST().AddConstAssert(sa);
        return sa;
    }

    /// @param source the source information
    /// @param condition the assertion condition
    /// @returns a new `ast::ConstAssert` with the given assertion condition
    template <typename EXPR>
    const ast::ConstAssert* ConstAssert(const Source& source, EXPR&& condition) {
        return create<ast::ConstAssert>(source, Expr(std::forward<EXPR>(condition)));
    }

    /// @param condition the assertion condition
    /// @returns a new `ast::ConstAssert` with the given assertion condition
    template <typename EXPR, typename = DisableIfSource<EXPR>>
    const ast::ConstAssert* ConstAssert(EXPR&& condition) {
        return create<ast::ConstAssert>(Expr(std::forward<EXPR>(condition)));
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

    /// @param source the source information
    /// @param expr the expression to perform a unary not on
    /// @return an ast::UnaryOpExpression that is the unary not of the input
    /// expression
    template <typename EXPR>
    const ast::UnaryOpExpression* Not(const Source& source, EXPR&& expr) {
        return create<ast::UnaryOpExpression>(source, ast::UnaryOp::kNot,
                                              Expr(std::forward<EXPR>(expr)));
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

    /// @param args the arguments for the constructor
    /// @returns an ast::CallExpression to the type `T`, with the arguments of @p args converted to
    /// `ast::Expression`s using Expr().
    template <typename T, typename... ARGS, typename = DisableIfSource<ARGS...>>
    const ast::CallExpression* Call(ARGS&&... args) {
        return Call(source_, ty.Of<T>(), std::forward<ARGS>(args)...);
    }

    /// @param source the source of the call
    /// @param args the arguments for the constructor
    /// @returns an ast::CallExpression to the type `T` with the arguments of @p args converted to
    /// `ast::Expression`s using Expr().
    template <typename T, typename... ARGS>
    const ast::CallExpression* Call(const Source& source, ARGS&&... args) {
        return Call(source, ty.Of<T>(), std::forward<ARGS>(args)...);
    }

    /// @param target the call target
    /// @param args the function call arguments
    /// @returns an ast::CallExpression to the target @p target, with the arguments of @p args
    /// converted to `ast::Expression`s using Expr().
    template <typename TARGET,
              typename... ARGS,
              typename = DisableIfSource<TARGET>,
              typename = DisableIfScalar<TARGET>>
    const ast::CallExpression* Call(TARGET&& target, ARGS&&... args) {
        return Call(source_, Expr(std::forward<TARGET>(target)), std::forward<ARGS>(args)...);
    }

    /// @param source the source of the call
    /// @param target the call target
    /// @param args the function call arguments
    /// @returns an ast::CallExpression to the target @p target, with the arguments of @p args
    /// converted to `ast::Expression`s using Expr().
    template <typename TARGET, typename... ARGS, typename = DisableIfScalar<TARGET>>
    const ast::CallExpression* Call(const Source& source, TARGET&& target, ARGS&&... args) {
        return create<ast::CallExpression>(source, Expr(std::forward<TARGET>(target)),
                                           ExprList(std::forward<ARGS>(args)...));
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

    /// @param source the source information
    /// @param lhs the left hand argument to the division operation
    /// @param rhs the right hand argument to the division operation
    /// @returns a `ast::BinaryExpression` dividing `lhs` by `rhs`
    template <typename LHS, typename RHS>
    const ast::BinaryExpression* Div(const Source& source, LHS&& lhs, RHS&& rhs) {
        return create<ast::BinaryExpression>(source, ast::BinaryOp::kDivide,
                                             Expr(std::forward<LHS>(lhs)),
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

    /// @param source the source information
    /// @param lhs the left hand argument to the logical and operation
    /// @param rhs the right hand argument to the logical and operation
    /// @returns a `ast::BinaryExpression` of `lhs` && `rhs`
    template <typename LHS, typename RHS>
    const ast::BinaryExpression* LogicalAnd(const Source& source, LHS&& lhs, RHS&& rhs) {
        return create<ast::BinaryExpression>(source, ast::BinaryOp::kLogicalAnd,
                                             Expr(std::forward<LHS>(lhs)),
                                             Expr(std::forward<RHS>(rhs)));
    }

    /// @param lhs the left hand argument to the logical or operation
    /// @param rhs the right hand argument to the logical or operation
    /// @returns a `ast::BinaryExpression` of `lhs` || `rhs`
    template <typename LHS, typename RHS>
    const ast::BinaryExpression* LogicalOr(LHS&& lhs, RHS&& rhs) {
        return create<ast::BinaryExpression>(
            ast::BinaryOp::kLogicalOr, Expr(std::forward<LHS>(lhs)), Expr(std::forward<RHS>(rhs)));
    }

    /// @param source the source information
    /// @param lhs the left hand argument to the logical or operation
    /// @param rhs the right hand argument to the logical or operation
    /// @returns a `ast::BinaryExpression` of `lhs` || `rhs`
    template <typename LHS, typename RHS>
    const ast::BinaryExpression* LogicalOr(const Source& source, LHS&& lhs, RHS&& rhs) {
        return create<ast::BinaryExpression>(source, ast::BinaryOp::kLogicalOr,
                                             Expr(std::forward<LHS>(lhs)),
                                             Expr(std::forward<RHS>(rhs)));
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

    /// @param source the source information
    /// @param lhs the left hand argument to the equal expression
    /// @param rhs the right hand argument to the equal expression
    /// @returns a `ast::BinaryExpression` comparing `lhs` equal to `rhs`
    template <typename LHS, typename RHS>
    const ast::BinaryExpression* Equal(const Source& source, LHS&& lhs, RHS&& rhs) {
        return create<ast::BinaryExpression>(source, ast::BinaryOp::kEqual,
                                             Expr(std::forward<LHS>(lhs)),
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
    /// @param object the object for the index accessor expression
    /// @param index the index argument for the index accessor expression
    /// @returns a `ast::IndexAccessorExpression` that indexes @p object with @p index
    template <typename OBJECT, typename INDEX>
    const ast::IndexAccessorExpression* IndexAccessor(const Source& source,
                                                      OBJECT&& object,
                                                      INDEX&& index) {
        return create<ast::IndexAccessorExpression>(source, Expr(std::forward<OBJECT>(object)),
                                                    Expr(std::forward<INDEX>(index)));
    }

    /// @param object the object for the index accessor expression
    /// @param index the index argument for the index accessor expression
    /// @returns a `ast::IndexAccessorExpression` that indexes @p object with @p index
    template <typename OBJECT, typename INDEX>
    const ast::IndexAccessorExpression* IndexAccessor(OBJECT&& object, INDEX&& index) {
        return create<ast::IndexAccessorExpression>(Expr(std::forward<OBJECT>(object)),
                                                    Expr(std::forward<INDEX>(index)));
    }

    /// @param source the source information
    /// @param object the object for the member accessor expression
    /// @param member the member argument for the member accessor expression
    /// @returns a `ast::MemberAccessorExpression` that indexes @p object with @p member
    template <typename OBJECT, typename MEMBER>
    const ast::MemberAccessorExpression* MemberAccessor(const Source& source,
                                                        OBJECT&& object,
                                                        MEMBER&& member) {
        static_assert(
            !utils::traits::IsType<utils::traits::PtrElTy<MEMBER>, ast::TemplatedIdentifier>,
            "it is currently invalid for a structure to hold a templated member");
        return create<ast::MemberAccessorExpression>(source, Expr(std::forward<OBJECT>(object)),
                                                     Ident(std::forward<MEMBER>(member)));
    }

    /// @param object the object for the member accessor expression
    /// @param member the member argument for the member accessor expression
    /// @returns a `ast::MemberAccessorExpression` that indexes @p object with @p member
    template <typename OBJECT, typename MEMBER>
    const ast::MemberAccessorExpression* MemberAccessor(OBJECT&& object, MEMBER&& member) {
        return MemberAccessor(source_, std::forward<OBJECT>(object), std::forward<MEMBER>(member));
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

    /// Creates a ast::StrideAttribute
    /// @param stride the array stride
    /// @returns the ast::StrideAttribute attribute
    const ast::StrideAttribute* Stride(uint32_t stride) {
        return create<ast::StrideAttribute>(source_, stride);
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
    /// @param name the function name
    /// @param params the function parameters
    /// @param type the function return type
    /// @param body the function body. Can be an ast::BlockStatement*, as ast::Statement* which will
    /// be automatically placed into a block, or nullptr for a stub function.
    /// @param attributes the optional function attributes
    /// @param return_type_attributes the optional function return type attributes
    /// @returns the function pointer
    template <typename NAME, typename BODY = utils::VectorRef<const ast::Statement*>>
    const ast::Function* Func(
        NAME&& name,
        utils::VectorRef<const ast::Parameter*> params,
        ast::Type type,
        BODY&& body,
        utils::VectorRef<const ast::Attribute*> attributes = utils::Empty,
        utils::VectorRef<const ast::Attribute*> return_type_attributes = utils::Empty) {
        return Func(source_, std::forward<NAME>(name), std::move(params), type,
                    std::forward<BODY>(body), std::move(attributes),
                    std::move(return_type_attributes));
    }

    /// Creates an ast::Function and registers it with the ast::Module.
    /// @param source the source information
    /// @param name the function name
    /// @param params the function parameters
    /// @param type the function return type
    /// @param body the function body. Can be an ast::BlockStatement*, as ast::Statement* which will
    /// be automatically placed into a block, or nullptr for a stub function.
    /// @param attributes the optional function attributes
    /// @param return_type_attributes the optional function return type attributes
    /// @returns the function pointer
    template <typename NAME, typename BODY = utils::VectorRef<const ast::Statement*>>
    const ast::Function* Func(
        const Source& source,
        NAME&& name,
        utils::VectorRef<const ast::Parameter*> params,
        ast::Type type,
        BODY&& body,
        utils::VectorRef<const ast::Attribute*> attributes = utils::Empty,
        utils::VectorRef<const ast::Attribute*> return_type_attributes = utils::Empty) {
        const ast::BlockStatement* block = nullptr;
        using BODY_T = utils::traits::PtrElTy<BODY>;
        if constexpr (utils::traits::IsTypeOrDerived<BODY_T, ast::BlockStatement> ||
                      std::is_same_v<BODY_T, std::nullptr_t>) {
            block = body;
        } else {
            block = Block(std::forward<BODY>(body));
        }
        auto* func =
            create<ast::Function>(source, Ident(std::forward<NAME>(name)), std::move(params), type,
                                  block, std::move(attributes), std::move(return_type_attributes));
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
    /// @param name the alias name
    /// @param type the alias target type
    /// @returns the alias type
    template <typename NAME>
    const ast::Alias* Alias(NAME&& name, ast::Type type) {
        return Alias(source_, std::forward<NAME>(name), type);
    }

    /// Creates a ast::Alias registering it with the AST().TypeDecls().
    /// @param source the source information
    /// @param name the alias name
    /// @param type the alias target type
    /// @returns the alias type
    template <typename NAME>
    const ast::Alias* Alias(const Source& source, NAME&& name, ast::Type type) {
        auto out = ty.alias(source, std::forward<NAME>(name), type);
        AST().AddTypeDecl(out);
        return out;
    }

    /// Creates a ast::Struct registering it with the AST().TypeDecls().
    /// @param name the struct name
    /// @param members the struct members
    /// @param attributes the optional struct attributes
    /// @returns the struct type
    template <typename NAME>
    const ast::Struct* Structure(
        NAME&& name,
        utils::VectorRef<const ast::StructMember*> members,
        utils::VectorRef<const ast::Attribute*> attributes = utils::Empty) {
        return Structure(source_, std::forward<NAME>(name), std::move(members),
                         std::move(attributes));
    }

    /// Creates a ast::Struct registering it with the AST().TypeDecls().
    /// @param source the source information
    /// @param name the struct name
    /// @param members the struct members
    /// @param attributes the optional struct attributes
    /// @returns the struct type
    template <typename NAME>
    const ast::Struct* Structure(
        const Source& source,
        NAME&& name,
        utils::VectorRef<const ast::StructMember*> members,
        utils::VectorRef<const ast::Attribute*> attributes = utils::Empty) {
        auto* type = create<ast::Struct>(source, Ident(std::forward<NAME>(name)),
                                         std::move(members), std::move(attributes));
        AST().AddTypeDecl(type);
        return type;
    }

    /// Creates a ast::StructMember
    /// @param name the struct member name
    /// @param type the struct member type
    /// @param attributes the optional struct member attributes
    /// @returns the struct member pointer
    template <typename NAME, typename = DisableIfSource<NAME>>
    const ast::StructMember* Member(
        NAME&& name,
        ast::Type type,
        utils::VectorRef<const ast::Attribute*> attributes = utils::Empty) {
        return Member(source_, std::forward<NAME>(name), type, std::move(attributes));
    }

    /// Creates a ast::StructMember
    /// @param source the struct member source
    /// @param name the struct member name
    /// @param type the struct member type
    /// @param attributes the optional struct member attributes
    /// @returns the struct member pointer
    template <typename NAME>
    const ast::StructMember* Member(
        const Source& source,
        NAME&& name,
        ast::Type type,
        utils::VectorRef<const ast::Attribute*> attributes = utils::Empty) {
        return create<ast::StructMember>(source, Ident(std::forward<NAME>(name)), type,
                                         std::move(attributes));
    }

    /// Creates a ast::StructMember with the given byte offset
    /// @param offset the offset to use in the StructMemberOffsetAttribute
    /// @param name the struct member name
    /// @param type the struct member type
    /// @returns the struct member pointer
    template <typename NAME>
    const ast::StructMember* Member(uint32_t offset, NAME&& name, ast::Type type) {
        return create<ast::StructMember>(source_, Ident(std::forward<NAME>(name)), type,
                                         utils::Vector<const ast::Attribute*, 1>{
                                             MemberOffset(AInt(offset)),
                                         });
    }

    /// Creates a ast::BlockStatement with input statements and attributes
    /// @param statements the statements of the block
    /// @param attributes the optional attributes of the block
    /// @returns the block statement pointer
    const ast::BlockStatement* Block(
        utils::VectorRef<const ast::Statement*> statements,
        utils::VectorRef<const ast::Attribute*> attributes = utils::Empty) {
        return Block(source_, std::move(statements), std::move(attributes));
    }

    /// Creates a ast::BlockStatement with input statements and attributes
    /// @param source the source information for the block
    /// @param statements the statements of the block
    /// @param attributes the optional attributes of the block
    /// @returns the block statement pointer
    const ast::BlockStatement* Block(
        const Source& source,
        utils::VectorRef<const ast::Statement*> statements,
        utils::VectorRef<const ast::Attribute*> attributes = utils::Empty) {
        return create<ast::BlockStatement>(source, std::move(statements), std::move(attributes));
    }

    /// Creates a ast::BlockStatement with a parameter list of input statements
    /// @param statements the optional statements of the block
    /// @returns the block statement pointer
    template <typename... STATEMENTS,
              typename = DisableIfSource<STATEMENTS...>,
              typename = DisableIfVectorLike<STATEMENTS...>>
    const ast::BlockStatement* Block(STATEMENTS&&... statements) {
        return Block(source_, std::forward<STATEMENTS>(statements)...);
    }

    /// Creates a ast::BlockStatement with a parameter list of input statements
    /// @param source the source information for the block
    /// @param statements the optional statements of the block
    /// @returns the block statement pointer
    template <typename... STATEMENTS, typename = DisableIfVectorLike<STATEMENTS...>>
    const ast::BlockStatement* Block(const Source& source, STATEMENTS&&... statements) {
        return create<ast::BlockStatement>(
            source,
            utils::Vector<const ast::Statement*, sizeof...(statements)>{
                std::forward<STATEMENTS>(statements)...,
            },
            utils::Empty);
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
    /// @param attributes optional attributes
    /// @returns the if statement pointer
    template <typename CONDITION>
    const ast::IfStatement* If(const Source& source,
                               CONDITION&& condition,
                               const ast::BlockStatement* body,
                               const ElseStmt else_stmt = ElseStmt(),
                               utils::VectorRef<const ast::Attribute*> attributes = utils::Empty) {
        return create<ast::IfStatement>(source, Expr(std::forward<CONDITION>(condition)), body,
                                        else_stmt.stmt, std::move(attributes));
    }

    /// Creates a ast::IfStatement with input condition, body, and optional
    /// else statement
    /// @param condition the if statement condition expression
    /// @param body the if statement body
    /// @param else_stmt optional else statement
    /// @param attributes optional attributes
    /// @returns the if statement pointer
    template <typename CONDITION>
    const ast::IfStatement* If(CONDITION&& condition,
                               const ast::BlockStatement* body,
                               const ElseStmt else_stmt = ElseStmt(),
                               utils::VectorRef<const ast::Attribute*> attributes = utils::Empty) {
        return create<ast::IfStatement>(Expr(std::forward<CONDITION>(condition)), body,
                                        else_stmt.stmt, std::move(attributes));
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
    /// @param attributes optional attributes
    /// @returns the loop statement pointer
    const ast::LoopStatement* Loop(
        const Source& source,
        const ast::BlockStatement* body,
        const ast::BlockStatement* continuing = nullptr,
        utils::VectorRef<const ast::Attribute*> attributes = utils::Empty) {
        return create<ast::LoopStatement>(source, body, continuing, std::move(attributes));
    }

    /// Creates a ast::LoopStatement with input body and optional continuing
    /// @param body the loop body
    /// @param continuing the optional continuing block
    /// @param attributes optional attributes
    /// @returns the loop statement pointer
    const ast::LoopStatement* Loop(
        const ast::BlockStatement* body,
        const ast::BlockStatement* continuing = nullptr,
        utils::VectorRef<const ast::Attribute*> attributes = utils::Empty) {
        return create<ast::LoopStatement>(body, continuing, std::move(attributes));
    }

    /// Creates a ast::ForLoopStatement with input body and optional initializer, condition,
    /// continuing, and attributes.
    /// @param source the source information
    /// @param init the optional loop initializer
    /// @param cond the optional loop condition
    /// @param cont the optional loop continuing
    /// @param body the loop body
    /// @param attributes optional attributes
    /// @returns the for loop statement pointer
    template <typename COND>
    const ast::ForLoopStatement* For(
        const Source& source,
        const ast::Statement* init,
        COND&& cond,
        const ast::Statement* cont,
        const ast::BlockStatement* body,
        utils::VectorRef<const ast::Attribute*> attributes = utils::Empty) {
        return create<ast::ForLoopStatement>(source, init, Expr(std::forward<COND>(cond)), cont,
                                             body, std::move(attributes));
    }

    /// Creates a ast::ForLoopStatement with input body and optional initializer, condition,
    /// continuing, and attributes.
    /// @param init the optional loop initializer
    /// @param cond the optional loop condition
    /// @param cont the optional loop continuing
    /// @param body the loop body
    /// @param attributes optional attributes
    /// @returns the for loop statement pointer
    template <typename COND>
    const ast::ForLoopStatement* For(
        const ast::Statement* init,
        COND&& cond,
        const ast::Statement* cont,
        const ast::BlockStatement* body,
        utils::VectorRef<const ast::Attribute*> attributes = utils::Empty) {
        return create<ast::ForLoopStatement>(init, Expr(std::forward<COND>(cond)), cont, body,
                                             std::move(attributes));
    }

    /// Creates a ast::WhileStatement with input body, condition, and optional attributes.
    /// @param source the source information
    /// @param cond the loop condition
    /// @param body the loop body
    /// @param attributes optional attributes
    /// @returns the while statement pointer
    template <typename COND>
    const ast::WhileStatement* While(
        const Source& source,
        COND&& cond,
        const ast::BlockStatement* body,
        utils::VectorRef<const ast::Attribute*> attributes = utils::Empty) {
        return create<ast::WhileStatement>(source, Expr(std::forward<COND>(cond)), body,
                                           std::move(attributes));
    }

    /// Creates a ast::WhileStatement with input body, condition, and optional attributes.
    /// @param cond the condition
    /// @param body the loop body
    /// @param attributes optional attributes
    /// @returns the while loop statement pointer
    template <typename COND>
    const ast::WhileStatement* While(
        COND&& cond,
        const ast::BlockStatement* body,
        utils::VectorRef<const ast::Attribute*> attributes = utils::Empty) {
        return create<ast::WhileStatement>(Expr(std::forward<COND>(cond)), body,
                                           std::move(attributes));
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
    template <typename ExpressionInit, typename... Cases, typename = DisableIfVectorLike<Cases...>>
    const ast::SwitchStatement* Switch(const Source& source,
                                       ExpressionInit&& condition,
                                       Cases&&... cases) {
        return create<ast::SwitchStatement>(
            source, Expr(std::forward<ExpressionInit>(condition)),
            utils::Vector<const ast::CaseStatement*, sizeof...(cases)>{
                std::forward<Cases>(cases)...},
            utils::Empty, utils::Empty);
    }

    /// Creates a ast::SwitchStatement with input expression and cases
    /// @param condition the condition expression initializer
    /// @param cases case statements
    /// @returns the switch statement pointer
    template <typename ExpressionInit,
              typename... Cases,
              typename = DisableIfSource<ExpressionInit>,
              typename = DisableIfVectorLike<Cases...>>
    const ast::SwitchStatement* Switch(ExpressionInit&& condition, Cases&&... cases) {
        return create<ast::SwitchStatement>(
            Expr(std::forward<ExpressionInit>(condition)),
            utils::Vector<const ast::CaseStatement*, sizeof...(cases)>{
                std::forward<Cases>(cases)...},
            utils::Empty, utils::Empty);
    }

    /// Creates a ast::SwitchStatement with input expression, cases, and optional attributes
    /// @param source the source information
    /// @param condition the condition expression initializer
    /// @param cases case statements
    /// @param stmt_attributes optional statement attributes
    /// @param body_attributes optional body attributes
    /// @returns the switch statement pointer
    template <typename ExpressionInit>
    const ast::SwitchStatement* Switch(
        const Source& source,
        ExpressionInit&& condition,
        utils::VectorRef<const ast::CaseStatement*> cases,
        utils::VectorRef<const ast::Attribute*> stmt_attributes = utils::Empty,
        utils::VectorRef<const ast::Attribute*> body_attributes = utils::Empty) {
        return create<ast::SwitchStatement>(source, Expr(std::forward<ExpressionInit>(condition)),
                                            cases, std::move(stmt_attributes),
                                            std::move(body_attributes));
    }

    /// Creates a ast::SwitchStatement with input expression, cases, and optional attributes
    /// @param condition the condition expression initializer
    /// @param cases case statements
    /// @param stmt_attributes optional statement attributes
    /// @param body_attributes optional body attributes
    /// @returns the switch statement pointer
    template <typename ExpressionInit, typename = DisableIfSource<ExpressionInit>>
    const ast::SwitchStatement* Switch(
        ExpressionInit&& condition,
        utils::VectorRef<const ast::CaseStatement*> cases,
        utils::VectorRef<const ast::Attribute*> stmt_attributes = utils::Empty,
        utils::VectorRef<const ast::Attribute*> body_attributes = utils::Empty) {
        return create<ast::SwitchStatement>(Expr(std::forward<ExpressionInit>(condition)), cases,
                                            std::move(stmt_attributes), std::move(body_attributes));
    }

    /// Creates a ast::CaseStatement with input list of selectors, and body
    /// @param selectors list of selectors
    /// @param body the case body
    /// @returns the case statement pointer
    const ast::CaseStatement* Case(utils::VectorRef<const ast::CaseSelector*> selectors,
                                   const ast::BlockStatement* body = nullptr) {
        return Case(source_, std::move(selectors), body);
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

    /// Convenient overload that takes a single selector
    /// @param selector a single case selector
    /// @param body the case body
    /// @returns the case statement pointer
    const ast::CaseStatement* Case(const ast::CaseSelector* selector,
                                   const ast::BlockStatement* body = nullptr) {
        return Case(utils::Vector{selector}, body ? body : Block());
    }

    /// Convenience function that creates a 'default' ast::CaseStatement
    /// @param body the case body
    /// @returns the case statement pointer
    const ast::CaseStatement* DefaultCase(const ast::BlockStatement* body = nullptr) {
        return DefaultCase(source_, body);
    }

    /// Convenience function that creates a 'default' ast::CaseStatement
    /// @param source the source information
    /// @param body the case body
    /// @returns the case statement pointer
    const ast::CaseStatement* DefaultCase(const Source& source,
                                          const ast::BlockStatement* body = nullptr) {
        return Case(source, utils::Vector{DefaultCaseSelector(source)}, body);
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
    template <typename BUILTIN>
    const ast::BuiltinAttribute* Builtin(const Source& source, BUILTIN&& builtin) {
        return create<ast::BuiltinAttribute>(source, Expr(std::forward<BUILTIN>(builtin)));
    }

    /// Creates an ast::BuiltinAttribute
    /// @param builtin the builtin value
    /// @returns the builtin attribute pointer
    template <typename BUILTIN>
    const ast::BuiltinAttribute* Builtin(BUILTIN&& builtin) {
        return create<ast::BuiltinAttribute>(source_, Expr(std::forward<BUILTIN>(builtin)));
    }

    /// Creates an ast::InterpolateAttribute
    /// @param type the interpolation type
    /// @returns the interpolate attribute pointer
    template <typename TYPE, typename = DisableIfSource<TYPE>>
    const ast::InterpolateAttribute* Interpolate(TYPE&& type) {
        return Interpolate(source_, std::forward<TYPE>(type));
    }

    /// Creates an ast::InterpolateAttribute
    /// @param source the source information
    /// @param type the interpolation type
    /// @returns the interpolate attribute pointer
    template <typename TYPE>
    const ast::InterpolateAttribute* Interpolate(const Source& source, TYPE&& type) {
        return create<ast::InterpolateAttribute>(source, Expr(std::forward<TYPE>(type)), nullptr);
    }

    /// Creates an ast::InterpolateAttribute
    /// @param type the interpolation type
    /// @param sampling the interpolation sampling
    /// @returns the interpolate attribute pointer
    template <typename TYPE, typename SAMPLING, typename = DisableIfSource<TYPE>>
    const ast::InterpolateAttribute* Interpolate(TYPE&& type, SAMPLING&& sampling) {
        return Interpolate(source_, std::forward<TYPE>(type), std::forward<SAMPLING>(sampling));
    }

    /// Creates an ast::InterpolateAttribute
    /// @param source the source information
    /// @param type the interpolation type
    /// @param sampling the interpolation sampling
    /// @returns the interpolate attribute pointer
    template <typename TYPE, typename SAMPLING>
    const ast::InterpolateAttribute* Interpolate(const Source& source,
                                                 TYPE&& type,
                                                 SAMPLING&& sampling) {
        if constexpr (std::is_same_v<std::decay_t<SAMPLING>, builtin::InterpolationSampling>) {
            if (sampling == builtin::InterpolationSampling::kUndefined) {
                return create<ast::InterpolateAttribute>(source, Expr(std::forward<TYPE>(type)),
                                                         nullptr);
            }
        }
        return create<ast::InterpolateAttribute>(source, Expr(std::forward<TYPE>(type)),
                                                 Expr(std::forward<SAMPLING>(sampling)));
    }

    /// Creates an ast::InterpolateAttribute using flat interpolation
    /// @param source the source information
    /// @returns the interpolate attribute pointer
    const ast::InterpolateAttribute* Flat(const Source& source) {
        return Interpolate(source, builtin::InterpolationType::kFlat);
    }

    /// Creates an ast::InterpolateAttribute using flat interpolation
    /// @returns the interpolate attribute pointer
    const ast::InterpolateAttribute* Flat() {
        return Interpolate(builtin::InterpolationType::kFlat);
    }

    /// Creates an ast::InvariantAttribute
    /// @param source the source information
    /// @returns the invariant attribute pointer
    const ast::InvariantAttribute* Invariant(const Source& source) {
        return create<ast::InvariantAttribute>(source);
    }

    /// Creates an ast::InvariantAttribute
    /// @returns the invariant attribute pointer
    const ast::InvariantAttribute* Invariant() { return create<ast::InvariantAttribute>(source_); }

    /// Creates an ast::MustUseAttribute
    /// @param source the source information
    /// @returns the invariant attribute pointer
    const ast::MustUseAttribute* MustUse(const Source& source) {
        return create<ast::MustUseAttribute>(source);
    }

    /// Creates an ast::MustUseAttribute
    /// @returns the invariant attribute pointer
    const ast::MustUseAttribute* MustUse() { return create<ast::MustUseAttribute>(source_); }

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

    /// Passthrough overload
    /// @param name the diagnostic rule name
    /// @returns @p name
    const ast::DiagnosticRuleName* DiagnosticRuleName(const ast::DiagnosticRuleName* name) {
        return name;
    }

    /// Creates an ast::DiagnosticRuleName
    /// @param name the diagnostic rule name
    /// @returns the diagnostic rule name
    template <typename NAME>
    const ast::DiagnosticRuleName* DiagnosticRuleName(NAME&& name) {
        static_assert(
            !utils::traits::IsType<utils::traits::PtrElTy<NAME>, ast::TemplatedIdentifier>,
            "it is invalid for a diagnostic rule name to be templated");
        auto* name_ident = Ident(std::forward<NAME>(name));
        return create<ast::DiagnosticRuleName>(name_ident->source, name_ident);
    }

    /// Creates an ast::DiagnosticRuleName
    /// @param category the diagnostic rule category
    /// @param name the diagnostic rule name
    /// @returns the diagnostic rule name
    template <typename CATEGORY, typename NAME, typename = DisableIfSource<CATEGORY>>
    const ast::DiagnosticRuleName* DiagnosticRuleName(CATEGORY&& category, NAME&& name) {
        static_assert(
            !utils::traits::IsType<utils::traits::PtrElTy<NAME>, ast::TemplatedIdentifier>,
            "it is invalid for a diagnostic rule name to be templated");
        static_assert(
            !utils::traits::IsType<utils::traits::PtrElTy<CATEGORY>, ast::TemplatedIdentifier>,
            "it is invalid for a diagnostic rule category to be templated");
        auto* category_ident = Ident(std::forward<CATEGORY>(category));
        auto* name_ident = Ident(std::forward<NAME>(name));
        Source source = category_ident->source;
        source.range.end = name_ident->source.range.end;
        return create<ast::DiagnosticRuleName>(source, category_ident, name_ident);
    }

    /// Creates an ast::DiagnosticRuleName
    /// @param source the source information
    /// @param name the diagnostic rule name
    /// @returns the diagnostic rule name
    template <typename NAME>
    const ast::DiagnosticRuleName* DiagnosticRuleName(const Source& source, NAME&& name) {
        static_assert(
            !utils::traits::IsType<utils::traits::PtrElTy<NAME>, ast::TemplatedIdentifier>,
            "it is invalid for a diagnostic rule name to be templated");
        auto* name_ident = Ident(std::forward<NAME>(name));
        return create<ast::DiagnosticRuleName>(source, name_ident);
    }

    /// Creates an ast::DiagnosticRuleName
    /// @param source the source information
    /// @param category the diagnostic rule category
    /// @param name the diagnostic rule name
    /// @returns the diagnostic rule name
    template <typename CATEGORY, typename NAME>
    const ast::DiagnosticRuleName* DiagnosticRuleName(const Source& source,
                                                      CATEGORY&& category,
                                                      NAME&& name) {
        static_assert(
            !utils::traits::IsType<utils::traits::PtrElTy<NAME>, ast::TemplatedIdentifier>,
            "it is invalid for a diagnostic rule name to be templated");
        static_assert(
            !utils::traits::IsType<utils::traits::PtrElTy<CATEGORY>, ast::TemplatedIdentifier>,
            "it is invalid for a diagnostic rule category to be templated");
        auto* category_ident = Ident(std::forward<CATEGORY>(category));
        auto* name_ident = Ident(std::forward<NAME>(name));
        return create<ast::DiagnosticRuleName>(source, category_ident, name_ident);
    }

    /// Creates an ast::DiagnosticAttribute
    /// @param source the source information
    /// @param severity the diagnostic severity control
    /// @param rule_args the arguments used to construct the rule name
    /// @returns the diagnostic attribute pointer
    template <typename... RULE_ARGS>
    const ast::DiagnosticAttribute* DiagnosticAttribute(const Source& source,
                                                        builtin::DiagnosticSeverity severity,
                                                        RULE_ARGS&&... rule_args) {
        return create<ast::DiagnosticAttribute>(
            source, ast::DiagnosticControl(
                        severity, DiagnosticRuleName(std::forward<RULE_ARGS>(rule_args)...)));
    }

    /// Creates an ast::DiagnosticAttribute
    /// @param severity the diagnostic severity control
    /// @param rule_args the arguments used to construct the rule name
    /// @returns the diagnostic attribute pointer
    template <typename... RULE_ARGS>
    const ast::DiagnosticAttribute* DiagnosticAttribute(builtin::DiagnosticSeverity severity,
                                                        RULE_ARGS&&... rule_args) {
        return create<ast::DiagnosticAttribute>(
            source_, ast::DiagnosticControl(
                         severity, DiagnosticRuleName(std::forward<RULE_ARGS>(rule_args)...)));
    }

    /// Add a diagnostic directive to the module.
    /// @param source the source information
    /// @param severity the diagnostic severity control
    /// @param rule_args the arguments used to construct the rule name
    /// @returns the diagnostic directive pointer
    template <typename... RULE_ARGS>
    const ast::DiagnosticDirective* DiagnosticDirective(const Source& source,
                                                        builtin::DiagnosticSeverity severity,
                                                        RULE_ARGS&&... rule_args) {
        auto* rule = DiagnosticRuleName(std::forward<RULE_ARGS>(rule_args)...);
        auto* directive =
            create<ast::DiagnosticDirective>(source, ast::DiagnosticControl(severity, rule));
        AST().AddDiagnosticDirective(directive);
        return directive;
    }

    /// Add a diagnostic directive to the module.
    /// @param severity the diagnostic severity control
    /// @param rule_args the arguments used to construct the rule name
    /// @returns the diagnostic directive pointer
    template <typename... RULE_ARGS>
    const ast::DiagnosticDirective* DiagnosticDirective(builtin::DiagnosticSeverity severity,
                                                        RULE_ARGS&&... rule_args) {
        auto* rule = DiagnosticRuleName(std::forward<RULE_ARGS>(rule_args)...);
        auto* directive =
            create<ast::DiagnosticDirective>(source_, ast::DiagnosticControl(severity, rule));
        AST().AddDiagnosticDirective(directive);
        return directive;
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
    const type::Type* TypeOf(const ast::Expression* expr) const;

    /// Helper for returning the resolved semantic type of the variable `var`.
    /// @note As the Resolver is run when the Program is built, this will only be
    /// useful for the Resolver itself and tests that use their own Resolver.
    /// @param var the AST variable
    /// @return the resolved semantic type for the variable, or nullptr if the
    /// variable has no resolved type.
    const type::Type* TypeOf(const ast::Variable* var) const;

    /// Helper for returning the resolved semantic type of the AST type
    /// declaration `type_decl`.
    /// @note As the Resolver is run when the Program is built, this will only be
    /// useful for the Resolver itself and tests that use their own Resolver.
    /// @param type_decl the AST type declaration
    /// @return the resolved semantic type for the type declaration, or nullptr if
    /// the type declaration has no resolved type.
    const type::Type* TypeOf(const ast::TypeDecl* type_decl) const;

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
    template <typename... ARGS,
              typename = utils::traits::EnableIf<(CanWrapInStatement<ARGS>::value && ...)>>
    const ast::Function* WrapInFunction(ARGS&&... args) {
        utils::Vector stmts{
            WrapInStatement(std::forward<ARGS>(args))...,
        };
        return WrapInFunction(std::move(stmts));
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
    const constant::Value* createSplatOrComposite(
        const type::Type* type,
        utils::VectorRef<const constant::Value*> elements);

    ProgramID id_;
    ast::NodeID last_ast_node_id_ = ast::NodeID{static_cast<decltype(ast::NodeID::value)>(0) - 1};
    type::Manager types_;
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
    static ast::Type get(const ProgramBuilder::TypesBuilder*) { return ast::Type{}; }
};
template <>
struct ProgramBuilder::TypesBuilder::CToAST<AFloat> {
    static ast::Type get(const ProgramBuilder::TypesBuilder*) { return ast::Type{}; }
};
template <>
struct ProgramBuilder::TypesBuilder::CToAST<i32> {
    static ast::Type get(const ProgramBuilder::TypesBuilder* t) { return t->i32(); }
};
template <>
struct ProgramBuilder::TypesBuilder::CToAST<u32> {
    static ast::Type get(const ProgramBuilder::TypesBuilder* t) { return t->u32(); }
};
template <>
struct ProgramBuilder::TypesBuilder::CToAST<f32> {
    static ast::Type get(const ProgramBuilder::TypesBuilder* t) { return t->f32(); }
};
template <>
struct ProgramBuilder::TypesBuilder::CToAST<f16> {
    static ast::Type get(const ProgramBuilder::TypesBuilder* t) { return t->f16(); }
};
template <>
struct ProgramBuilder::TypesBuilder::CToAST<bool> {
    static ast::Type get(const ProgramBuilder::TypesBuilder* t) { return t->bool_(); }
};
//! @endcond

/// @param builder the ProgramBuilder
/// @returns the ProgramID of the ProgramBuilder
inline ProgramID ProgramIDOf(const ProgramBuilder* builder) {
    return builder->ID();
}

// Primary template for metafunction that evaluates to true iff T can be wrapped in a statement.
template <typename T, typename /*  = void */>
struct CanWrapInStatement : std::false_type {};

// Specialization of CanWrapInStatement
template <typename T>
struct CanWrapInStatement<
    T,
    std::void_t<decltype(std::declval<ProgramBuilder>().WrapInStatement(std::declval<T>()))>>
    : std::true_type {};

}  // namespace tint

#endif  // SRC_TINT_PROGRAM_BUILDER_H_
