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

#ifndef SRC_TINT_LANG_WGSL_PROGRAM_PROGRAM_BUILDER_H_
#define SRC_TINT_LANG_WGSL_PROGRAM_PROGRAM_BUILDER_H_

#include <string>
#include <unordered_set>
#include <utility>

#include "tint/override_id.h"

#include "src/tint/lang/core/constant/manager.h"
#include "src/tint/lang/core/extension.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/interpolation_sampling.h"
#include "src/tint/lang/core/interpolation_type.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/external_texture.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/sampler_kind.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/lang/wgsl/ast/builder.h"
#include "src/tint/lang/wgsl/program/program.h"
#include "src/tint/lang/wgsl/sem/array_count.h"
#include "src/tint/lang/wgsl/sem/struct.h"
#include "src/tint/utils/id/generation_id.h"
#include "src/tint/utils/text/string.h"

#ifdef CURRENTLY_IN_TINT_PUBLIC_HEADER
#error "internal tint header being #included from tint.h"
#endif

namespace tint {

/// ProgramBuilder is a mutable builder for a Program.
/// To construct a Program, populate the builder and then `std::move` it to a
/// Program.
class ProgramBuilder : public ast::Builder {
  public:
    /// SemNodeAllocator is an alias to BlockAllocator<sem::Node>
    using SemNodeAllocator = BlockAllocator<sem::Node>;

    /// Constructor
    ProgramBuilder();

    /// Move constructor
    /// @param rhs the builder to move
    ProgramBuilder(ProgramBuilder&& rhs);

    /// Destructor
    ~ProgramBuilder();

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

    /// @returns a reference to the program's types
    type::Manager& Types() {
        AssertNotMoved();
        return constants.types;
    }

    /// @returns a reference to the program's types
    const type::Manager& Types() const {
        AssertNotMoved();
        return constants.types;
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

    /// Overlay Builder::create() overloads
    using Builder::create;

    /// Creates a new sem::Node owned by the ProgramBuilder.
    /// When the ProgramBuilder is destructed, the sem::Node will also be destructed.
    /// @param args the arguments to pass to the constructor
    /// @returns the node pointer
    template <typename T, typename... ARGS>
    tint::traits::EnableIf<tint::traits::IsTypeOrDerived<T, sem::Node> &&
                               !tint::traits::IsTypeOrDerived<T, type::Node>,
                           T>*
    create(ARGS&&... args) {
        AssertNotMoved();
        return sem_nodes_.Create<T>(std::forward<ARGS>(args)...);
    }

    /// Creates a new type::Node owned by the ProgramBuilder.
    /// When the ProgramBuilder is destructed, owned ProgramBuilder and the returned node will also
    /// be destructed. If T derives from type::UniqueNode, then the calling create() for the same
    /// `T` and arguments will return the same pointer.
    /// @param args the arguments to pass to the constructor
    /// @returns the new, or existing node
    template <typename T, typename... ARGS>
    tint::traits::EnableIfIsType<T, type::Node>* create(ARGS&&... args) {
        AssertNotMoved();
        return constants.types.Get<T>(std::forward<ARGS>(args)...);
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

    /// The constants manager
    constant::Manager constants;

  protected:
    /// Asserts that the builder has not been moved.
    void AssertNotMoved() const;

  private:
    SemNodeAllocator sem_nodes_;
    sem::Info sem_;
};

/// @param builder the ProgramBuilder
/// @returns the GenerationID of the ProgramBuilder
inline GenerationID GenerationIDOf(const ProgramBuilder* builder) {
    return builder->ID();
}

}  // namespace tint

#endif  // SRC_TINT_LANG_WGSL_PROGRAM_PROGRAM_BUILDER_H_
