// Copyright 2022 The Tint Authors.
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

#include "src/tint/transform/std140.h"

#include <algorithm>
#include <string>
#include <utility>
#include <variant>

#include "src/tint/program_builder.h"
#include "src/tint/sem/index_accessor_expression.h"
#include "src/tint/sem/member_accessor_expression.h"
#include "src/tint/sem/module.h"
#include "src/tint/sem/struct.h"
#include "src/tint/sem/variable.h"
#include "src/tint/switch.h"
#include "src/tint/utils/compiler_macros.h"
#include "src/tint/utils/hashmap.h"
#include "src/tint/utils/transform.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::Std140);

using namespace tint::number_suffixes;  // NOLINT

namespace {

/// UniformVariable is used by Std140::State::AccessIndex to indicate the root uniform variable
struct UniformVariable {};

/// Inequality operator for UniformVariable
bool operator!=(const UniformVariable&, const UniformVariable&) {
    return false;
}

/// DynamicIndex is used by Std140::State::AccessIndex to indicate a runtime-expression index
struct DynamicIndex {
    size_t slot;  // The index of the expression in Std140::State::AccessChain::dynamic_indices
};

/// Inequality operator for DynamicIndex
bool operator!=(const DynamicIndex& a, const DynamicIndex& b) {
    return a.slot != b.slot;
}

}  // namespace

namespace tint::utils {

/// Hasher specialization for UniformVariable
template <>
struct Hasher<UniformVariable> {
    /// The hash function for the UniformVariable
    /// @return the hash for the given UniformVariable
    size_t operator()(const UniformVariable&) const { return 0; }
};

/// Hasher specialization for DynamicIndex
template <>
struct Hasher<DynamicIndex> {
    /// The hash function for the DynamicIndex
    /// @param d the DynamicIndex to hash
    /// @return the hash for the given DynamicIndex
    size_t operator()(const DynamicIndex& d) const { return utils::Hash(d.slot); }
};

}  // namespace tint::utils

namespace tint::transform {

/// PIMPL state for the transform
struct Std140::State {
    /// Constructor
    /// @param program the source program
    explicit State(const Program* program) : src(program) {}

    /// Runs the transform
    /// @returns the new program or SkipTransform if the transform is not required
    ApplyResult Run() {
        if (!ShouldRun()) {
            // Transform is not required
            return SkipTransform;
        }

        // Begin by creating forked types for any type that is used as a uniform buffer, that
        // either directly or transitively contains a matrix that needs splitting for std140 layout.
        ForkTypes();

        // Next, replace all the uniform variables to use the forked types.
        ReplaceUniformVarTypes();

        // Finally, replace all expression chains that used the authored types with those that
        // correctly use the forked types.
        ctx.ReplaceAll([&](const ast::Expression* expr) -> const ast::Expression* {
            if (auto access = AccessChainFor(expr)) {
                if (!access->std140_mat_idx.has_value()) {
                    // loading a std140 type, which is not a whole or partial decomposed matrix
                    return LoadWithConvert(access.value());
                }
                if (!access->IsMatrixSubset() ||  // loading a whole matrix
                    std::holds_alternative<DynamicIndex>(
                        access->indices[*access->std140_mat_idx + 1])) {
                    // Whole object or matrix is loaded, or the matrix column is indexed with a
                    // non-constant index. Build a helper function to load the expression chain.
                    return LoadMatrixWithFn(access.value());
                }
                // Matrix column is statically indexed. Can be emitted as an inline expression.
                return LoadSubMatrixInline(access.value());
            }
            // Expression isn't an access to a std140-layout uniform buffer.
            // Just clone.
            return nullptr;
        });

        ctx.Clone();
        return Program(std::move(b));
    }

    /// @returns true if this transform should be run for the given program
    bool ShouldRun() const {
        // Returns true if the type needs to be forked for std140 usage.
        auto needs_fork = [&](const type::Type* ty) {
            while (auto* arr = ty->As<type::Array>()) {
                ty = arr->ElemType();
            }
            if (auto* mat = ty->As<type::Matrix>()) {
                if (MatrixNeedsDecomposing(mat)) {
                    return true;
                }
            }
            return false;
        };

        // Scan structures for members that need forking
        for (auto* ty : src->Types()) {
            if (auto* str = ty->As<type::Struct>()) {
                if (str->UsedAs(builtin::AddressSpace::kUniform)) {
                    for (auto* member : str->Members()) {
                        if (needs_fork(member->Type())) {
                            return true;
                        }
                    }
                }
            }
        }

        // Scan uniform variables that have types that need forking
        for (auto* decl : src->AST().GlobalVariables()) {
            auto* global = src->Sem().Get(decl);
            if (global->AddressSpace() == builtin::AddressSpace::kUniform) {
                if (needs_fork(global->Type()->UnwrapRef())) {
                    return true;
                }
            }
        }

        // If we reach here, no uniform variables use a type that needs forking for std140 layout
        return false;
    }

  private:
    /// Swizzle describes a vector swizzle
    using Swizzle = utils::Vector<uint32_t, 4>;

    /// AccessIndex describes a single access in an access chain.
    /// The access is one of:
    /// UniformVariable - the root uniform variable.
    /// u32             - a static index on a struct, array index, matrix column or vector element.
    /// DynamicIndex    - a runtime index on an array, matrix column, or vector element.
    /// Swizzle         - a static vector swizzle.
    using AccessIndex = std::variant<UniformVariable, u32, DynamicIndex, Swizzle>;

    /// A vector of AccessIndex.
    using AccessIndices = utils::Vector<AccessIndex, 8>;

    /// A key used to cache load functions for an access chain.
    struct LoadFnKey {
        /// The root uniform buffer variable for the access chain.
        const sem::GlobalVariable* var;

        /// The chain of accesses indices.
        AccessIndices indices;

        /// Hash function for LoadFnKey.
        struct Hasher {
            /// @param fn the LoadFnKey to hash
            /// @return the hash for the given LoadFnKey
            size_t operator()(const LoadFnKey& fn) const { return utils::Hash(fn.var, fn.indices); }
        };

        /// Equality operator
        bool operator==(const LoadFnKey& other) const {
            return var == other.var && indices == other.indices;
        }
    };

    /// The source program
    const Program* const src;
    /// The target program builder
    ProgramBuilder b;
    /// The clone context
    CloneContext ctx = {&b, src, /* auto_clone_symbols */ true};
    /// Alias to the semantic info in src
    const sem::Info& sem = src->Sem();
    /// Alias to the symbols in src
    const SymbolTable& sym = src->Symbols();

    /// Map of load function signature, to the generated function
    utils::Hashmap<LoadFnKey, Symbol, 8, LoadFnKey::Hasher> load_fns;

    /// Map of std140-forked type to converter function name
    utils::Hashmap<const type::Type*, Symbol, 8> conv_fns;

    // Uniform variables that have been modified to use a std140 type
    utils::Hashset<const sem::Variable*, 8> std140_uniforms;

    // Map of original structure to 'std140' forked structure
    utils::Hashmap<const type::Struct*, Symbol, 8> std140_structs;

    // Map of structure member in src of a matrix type, to list of decomposed column
    // members in ctx.dst.
    utils::Hashmap<const type::StructMember*, utils::Vector<const ast::StructMember*, 4>, 8>
        std140_mat_members;

    /// Describes a matrix that has been forked to a std140-structure holding the decomposed column
    /// vectors of the matrix.
    struct Std140Matrix {
        /// The decomposed structure name (in ctx.dst)
        Symbol name;
        /// The column vector structure member names (in ctx.dst)
        utils::Vector<Symbol, 4> columns;
    };

    // Map of matrix type in src, to decomposed column structure in ctx.dst.
    utils::Hashmap<const type::Matrix*, Std140Matrix, 8> std140_mats;

    /// AccessChain describes a chain of access expressions to uniform buffer variable.
    struct AccessChain {
        /// The uniform buffer variable.
        const sem::GlobalVariable* var;
        /// The chain of access indices, starting with the first access on #var.
        AccessIndices indices;
        /// The runtime-evaluated expressions. This vector is indexed by the DynamicIndex::slot
        utils::Vector<const sem::ValueExpression*, 8> dynamic_indices;
        /// The type of the std140-decomposed matrix being accessed.
        /// May be nullptr if the chain does not pass through a std140-decomposed matrix.
        const type::Matrix* std140_mat_ty = nullptr;
        /// The index in #indices of the access that resolves to the std140-decomposed matrix.
        /// May hold no value if the chain does not pass through a std140-decomposed matrix.
        std::optional<size_t> std140_mat_idx;

        /// @returns true if the access chain is to part of (not the whole) std140-decomposed matrix
        bool IsMatrixSubset() const {
            return std140_mat_idx.has_value() && (std140_mat_idx.value() + 1 != indices.Length());
        }
    };

    /// @returns true if the given matrix needs decomposing to column vectors for std140 layout.
    /// Std140 layout require matrix stride to be 16, otherwise decomposing is needed.
    static bool MatrixNeedsDecomposing(const type::Matrix* mat) {
        return mat->ColumnStride() != 16;
    }

    /// ForkTypes walks the user-declared types in dependency order, forking structures that are
    /// used as uniform buffers which (transitively) use matrices that need std140 decomposition to
    /// column vectors. Populates the #std140_mat_members map, #std140_structs set and #std140_mats
    /// map (via Std140Type()).
    void ForkTypes() {
        // For each module scope declaration...
        for (auto* global : src->Sem().Module()->DependencyOrderedDeclarations()) {
            // Check to see if this is a structure used by a uniform buffer...
            auto* str = sem.Get<sem::Struct>(global);
            if (str && str->UsedAs(builtin::AddressSpace::kUniform)) {
                // Should this uniform buffer be forked for std140 usage?
                bool fork_std140 = false;
                utils::Vector<const ast::StructMember*, 8> members;
                for (auto* member : str->Members()) {
                    if (auto* mat = member->Type()->As<type::Matrix>()) {
                        // Is this member a matrix that needs decomposition for std140-layout?
                        if (MatrixNeedsDecomposing(mat)) {
                            // Structure member of matrix type needs decomposition.
                            fork_std140 = true;
                            // Replace the member with column vectors.
                            const auto name_prefix = PrefixForUniqueNames(
                                str->Declaration(), member->Name(), mat->columns());

                            // Build a struct member for each column of the matrix
                            auto column_members = DecomposedMatrixStructMembers(
                                mat, name_prefix, member->Align(), member->Size());

                            // Add the member to the forked structure
                            for (auto* column_member : column_members) {
                                members.Push(column_member);
                            }
                            // Record that this matrix member was replaced with the N column
                            // members.
                            std140_mat_members.Add(member, std::move(column_members));

                            continue;  // Next member
                        }
                    } else if (auto std140_ty = Std140Type(member->Type())) {
                        // Member is of a type that requires forking for std140-layout
                        fork_std140 = true;
                        auto attrs = ctx.Clone(member->Declaration()->attributes);
                        members.Push(b.Member(member->Name().Name(), std140_ty, std::move(attrs)));
                        continue;  // Next member
                    }

                    // Nothing special about this member.
                    // Push the member in src to members without first cloning. We'll replace this
                    // with a cloned member once we know whether we need to fork the structure or
                    // not.
                    members.Push(member->Declaration());
                }

                // Did any of the members require forking the structure?
                if (fork_std140) {
                    // Clone any members that have not already been cloned.
                    for (auto& member : members) {
                        if (member->program_id == src->ID()) {
                            member = ctx.Clone(member);
                        }
                    }
                    // Create a new forked structure, and insert it just under the original
                    // structure.
                    auto name = b.Symbols().New(str->Name().Name() + "_std140");
                    auto* std140 = b.create<ast::Struct>(b.Ident(name), std::move(members),
                                                         ctx.Clone(str->Declaration()->attributes));
                    ctx.InsertAfter(src->AST().GlobalDeclarations(), global, std140);
                    std140_structs.Add(str, name);
                }
            }
        }
    }

    /// Walks the global variables, replacing the type of those that are a uniform buffer with a
    /// type that has been forked for std140-layout.
    /// Populates the #std140_uniforms set.
    void ReplaceUniformVarTypes() {
        for (auto* global : src->AST().GlobalVariables()) {
            if (auto* var = global->As<ast::Var>()) {
                auto* v = sem.Get(var);
                if (v->AddressSpace() == builtin::AddressSpace::kUniform) {
                    if (auto std140_ty = Std140Type(v->Type()->UnwrapRef())) {
                        ctx.Replace(global->type.expr, b.Expr(std140_ty));
                        std140_uniforms.Add(v);
                    }
                }
            }
        }
    }

    /// @returns a unique structure member prefix for the splitting of a matrix member into @p count
    /// column vector members. The new members must be suffixed with a zero-based index ranging from
    /// `[0..count)`.
    /// @param str the structure that will hold the uniquely named member.
    /// @param unsuffixed the common name prefix to use for the new members.
    /// @param count the number of members that need to be created.
    std::string PrefixForUniqueNames(const ast::Struct* str,
                                     Symbol unsuffixed,
                                     uint32_t count) const {
        auto prefix = unsuffixed.Name();
        // Keep on inserting '_' between the unsuffixed name and the suffix numbers until the name
        // is unique.
        while (true) {
            prefix += "_";

            utils::Hashset<std::string, 4> strings;
            for (uint32_t i = 0; i < count; i++) {
                strings.Add(prefix + std::to_string(i));
            }

            bool unique = true;
            for (auto* member : str->members) {
                // The member name must be unique over the entire set of `count` suffixed names.
                if (strings.Contains(member->name->symbol.Name())) {
                    unique = false;
                    break;
                }
            }

            if (unique) {
                return prefix;
            }
        }
    }

    /// @returns a new, forked std140 AST type for the corresponding non-forked semantic type.
    ///          If the semantic type is not split for std140-layout, then nullptr is returned.
    /// @note will construct new std140 structures to hold decomposed matrices, populating
    ///       #std140_mats.
    ast::Type Std140Type(const type::Type* ty) {
        return Switch(
            ty,  //
            [&](const type::Struct* str) {
                if (auto std140 = std140_structs.Find(str)) {
                    return b.ty(*std140);
                }
                return ast::Type{};
            },
            [&](const type::Matrix* mat) {
                if (MatrixNeedsDecomposing(mat)) {
                    auto std140_mat = std140_mats.GetOrCreate(mat, [&] {
                        auto name = b.Symbols().New("mat" + std::to_string(mat->columns()) + "x" +
                                                    std::to_string(mat->rows()) + "_" +
                                                    mat->type()->FriendlyName());
                        auto members =
                            DecomposedMatrixStructMembers(mat, "col", mat->Align(), mat->Size());
                        b.Structure(name, members);
                        return Std140Matrix{
                            name,
                            utils::Transform(members,
                                             [&](auto* member) { return member->name->symbol; }),
                        };
                    });
                    return b.ty(std140_mat.name);
                }
                return ast::Type{};
            },
            [&](const type::Array* arr) {
                if (auto std140 = Std140Type(arr->ElemType())) {
                    utils::Vector<const ast::Attribute*, 1> attrs;
                    if (!arr->IsStrideImplicit()) {
                        attrs.Push(b.create<ast::StrideAttribute>(arr->Stride()));
                    }
                    auto count = arr->ConstantCount();
                    if (TINT_UNLIKELY(!count)) {
                        // Non-constant counts should not be possible:
                        // * Override-expression counts can only be applied to workgroup arrays, and
                        //   this method only handles types transitively used as uniform buffers.
                        // * Runtime-sized arrays cannot be used in uniform buffers.
                        TINT_ICE(Transform, b.Diagnostics())
                            << "unexpected non-constant array count";
                        count = 1;
                    }
                    return b.ty.array(std140, b.Expr(u32(count.value())), std::move(attrs));
                }
                return ast::Type{};
            });
    }

    /// @param mat the matrix to decompose (in src)
    /// @param name_prefix the name prefix to apply to each of the returned column vector members.
    /// @param align the alignment in bytes of the matrix.
    /// @param size the size in bytes of the matrix.
    /// @returns a vector of decomposed matrix column vectors as structure members (in ctx.dst).
    utils::Vector<const ast::StructMember*, 4> DecomposedMatrixStructMembers(
        const type::Matrix* mat,
        const std::string& name_prefix,
        uint32_t align,
        uint32_t size) {
        // Replace the member with column vectors.
        const auto num_columns = mat->columns();
        // Build a struct member for each column of the matrix
        utils::Vector<const ast::StructMember*, 4> out;
        for (uint32_t i = 0; i < num_columns; i++) {
            utils::Vector<const ast::Attribute*, 1> attributes;
            if ((i == 0) && mat->Align() != align) {
                // The matrix was @align() annotated with a larger alignment
                // than the natural alignment for the matrix. This extra padding
                // needs to be applied to the first column vector.
                attributes.Push(b.MemberAlign(i32(align)));
            }
            if ((i == num_columns - 1) && mat->Size() != size) {
                // The matrix was @size() annotated with a larger size than the
                // natural size for the matrix. This extra padding needs to be
                // applied to the last column vector.
                attributes.Push(
                    b.MemberSize(AInt(size - mat->ColumnType()->Align() * (num_columns - 1))));
            }

            // Build the member
            const auto col_name = name_prefix + std::to_string(i);
            const auto col_ty = CreateASTTypeFor(ctx, mat->ColumnType());
            const auto* col_member = b.Member(col_name, col_ty, std::move(attributes));
            // Record the member for std140_mat_members
            out.Push(col_member);
        }
        return out;
    }

    /// Walks the @p ast_expr, constructing and returning an AccessChain.
    /// @returns an AccessChain if the expression is an access to a std140-forked uniform buffer,
    ///          otherwise returns a std::nullopt.
    std::optional<AccessChain> AccessChainFor(const ast::Expression* ast_expr) {
        auto* expr = sem.GetVal(ast_expr);
        if (!expr) {
            return std::nullopt;
        }

        AccessChain access;

        // Start by looking at the root identifier. This must be a std140-forked uniform buffer.
        access.var = tint::As<sem::GlobalVariable>(expr->RootIdentifier());
        if (!access.var || !std140_uniforms.Contains(access.var)) {
            // Not at std140-forked uniform buffer access chain.
            return std::nullopt;
        }

        // Walk from the outer-most expression, inwards towards the root identifier.
        while (true) {
            enum class Action { kStop, kContinue, kError };
            Action action = Switch(
                expr->Unwrap(),  //
                [&](const sem::VariableUser* user) {
                    if (user->Variable() == access.var) {
                        // Walked all the way to the root identifier. We're done traversing.
                        access.indices.Push(UniformVariable{});
                        return Action::kStop;
                    }
                    if (TINT_LIKELY(user->Variable()->Type()->Is<type::Pointer>())) {
                        // Found a pointer. As the root identifier is a uniform buffer variable,
                        // this must be a pointer-let. Continue traversing from the let
                        // initializer.
                        expr = user->Variable()->Initializer();
                        return Action::kContinue;
                    }
                    TINT_ICE(Transform, b.Diagnostics())
                        << "unexpected variable found walking access chain: "
                        << user->Variable()->Declaration()->name->symbol.Name();
                    return Action::kError;
                },
                [&](const sem::StructMemberAccess* a) {
                    // Is this a std140 decomposed matrix?
                    if (std140_mat_members.Contains(a->Member())) {
                        // Record this on the access.
                        access.std140_mat_idx = access.indices.Length();
                        access.std140_mat_ty = expr->Type()->UnwrapRef()->As<type::Matrix>();
                    }
                    // Structure member accesses are always statically indexed
                    access.indices.Push(u32(a->Member()->Index()));
                    expr = a->Object();
                    return Action::kContinue;
                },
                [&](const sem::IndexAccessorExpression* a) {
                    // Array, matrix or vector index.
                    if (auto* val = a->Index()->ConstantValue()) {
                        access.indices.Push(val->ValueAs<u32>());
                    } else {
                        access.indices.Push(DynamicIndex{access.dynamic_indices.Length()});
                        access.dynamic_indices.Push(a->Index());
                    }
                    expr = a->Object();

                    // Is the object a std140 decomposed matrix?
                    if (auto* mat = expr->Type()->UnwrapRef()->As<type::Matrix>()) {
                        if (std140_mats.Contains(mat)) {
                            // Record this on the access.
                            access.std140_mat_idx = access.indices.Length();
                            access.std140_mat_ty = mat;
                        }
                    }
                    return Action::kContinue;
                },
                [&](const sem::Swizzle* s) {
                    // Vector swizzle.
                    if (s->Indices().Length() == 1) {
                        access.indices.Push(u32(s->Indices()[0]));
                    } else {
                        access.indices.Push(s->Indices());
                    }
                    expr = s->Object();
                    return Action::kContinue;
                },
                [&](const sem::ValueExpression* e) {
                    // Walk past indirection and address-of unary ops.
                    return Switch(e->Declaration(),  //
                                  [&](const ast::UnaryOpExpression* u) {
                                      switch (u->op) {
                                          case ast::UnaryOp::kAddressOf:
                                          case ast::UnaryOp::kIndirection:
                                              expr = sem.GetVal(u->expr);
                                              return Action::kContinue;
                                          default:
                                              TINT_ICE(Transform, b.Diagnostics())
                                                  << "unhandled unary op for access chain: "
                                                  << u->op;
                                              return Action::kError;
                                      }
                                  });
                },
                [&](Default) {
                    TINT_ICE(Transform, b.Diagnostics())
                        << "unhandled expression type for access chain\n"
                        << "AST: " << expr->Declaration()->TypeInfo().name << "\n"
                        << "SEM: " << expr->TypeInfo().name;
                    return Action::kError;
                });

            switch (action) {
                case Action::kContinue:
                    continue;
                case Action::kStop:
                    break;
                case Action::kError:
                    return std::nullopt;
            }

            break;
        }

        // As the access walked from RHS to LHS, the last index operation applies to the source
        // variable. We want this the other way around, so reverse the arrays and fix indicies.
        std::reverse(access.indices.begin(), access.indices.end());
        std::reverse(access.dynamic_indices.begin(), access.dynamic_indices.end());
        if (access.std140_mat_idx.has_value()) {
            access.std140_mat_idx = access.indices.Length() - *access.std140_mat_idx - 1;
        }
        for (auto& index : access.indices) {
            if (auto* dyn_idx = std::get_if<DynamicIndex>(&index)) {
                dyn_idx->slot = access.dynamic_indices.Length() - dyn_idx->slot - 1;
            }
        }

        return access;
    }

    /// @returns a name suffix for a std140 -> non-std140 conversion function based on the type
    ///          being converted.
    const std::string ConvertSuffix(const type::Type* ty) {
        return Switch(
            ty,  //
            [&](const type::Struct* str) { return str->Name().Name(); },
            [&](const type::Array* arr) {
                auto count = arr->ConstantCount();
                if (TINT_UNLIKELY(!count)) {
                    // Non-constant counts should not be possible:
                    // * Override-expression counts can only be applied to workgroup arrays, and
                    //   this method only handles types transitively used as uniform buffers.
                    // * Runtime-sized arrays cannot be used in uniform buffers.
                    TINT_ICE(Transform, b.Diagnostics()) << "unexpected non-constant array count";
                    count = 1;
                }
                return "arr" + std::to_string(count.value()) + "_" + ConvertSuffix(arr->ElemType());
            },
            [&](const type::Matrix* mat) {
                return "mat" + std::to_string(mat->columns()) + "x" + std::to_string(mat->rows()) +
                       "_" + ConvertSuffix(mat->type());
            },
            [&](const type::F32*) { return "f32"; },  //
            [&](const type::F16*) { return "f16"; },
            [&](Default) {
                TINT_ICE(Transform, b.Diagnostics())
                    << "unhandled type for conversion name: " << ty->FriendlyName();
                return "";
            });
    }

    /// Generates and returns an expression that loads the value from a std140 uniform buffer,
    /// converting the final result to a non-std140 type.
    /// @param chain the access chain from a uniform buffer to the value to load.
    const ast::Expression* LoadWithConvert(const AccessChain& chain) {
        const ast::Expression* expr = nullptr;
        const type::Type* ty = nullptr;
        auto dynamic_index = [&](size_t idx) {
            return ctx.Clone(chain.dynamic_indices[idx]->Declaration());
        };
        for (size_t i = 0; i < chain.indices.Length(); i++) {
            auto [new_expr, new_ty, _] = BuildAccessExpr(expr, ty, chain, i, dynamic_index);
            expr = new_expr;
            ty = new_ty;
        }
        return Convert(ty, expr);
    }

    /// Generates and returns an expression that converts the expression @p expr of the
    /// std140-forked type to the type @p ty. If @p expr is not a std140-forked type, then Convert()
    /// will simply return @p expr.
    /// @returns the converted value expression.
    const ast::Expression* Convert(const type::Type* ty, const ast::Expression* expr) {
        // Get an existing, or create a new function for converting the std140 type to ty.
        auto fn = conv_fns.GetOrCreate(ty, [&] {
            auto std140_ty = Std140Type(ty);
            if (!std140_ty) {
                // ty was not forked for std140.
                return Symbol{};
            }

            // The converter function takes a single argument of the std140 type.
            auto* param = b.Param("val", std140_ty);

            utils::Vector<const ast::Statement*, 3> stmts;

            Switch(
                ty,  //
                [&](const type::Struct* str) {
                    // Convert each of the structure members using either a converter function
                    // call, or by reassembling a std140 matrix from column vector members.
                    utils::Vector<const ast::Expression*, 8> args;
                    for (auto* member : str->Members()) {
                        if (auto col_members = std140_mat_members.Find(member)) {
                            // std140 decomposed matrix. Reassemble.
                            auto mat_ty = CreateASTTypeFor(ctx, member->Type());
                            auto mat_args =
                                utils::Transform(*col_members, [&](const ast::StructMember* m) {
                                    return b.MemberAccessor(param, m->name->symbol);
                                });
                            args.Push(b.Call(mat_ty, std::move(mat_args)));
                        } else {
                            // Convert the member
                            args.Push(Convert(member->Type(),
                                              b.MemberAccessor(param, member->Name().Name())));
                        }
                    }
                    stmts.Push(b.Return(b.Call(CreateASTTypeFor(ctx, ty), std::move(args))));
                },  //
                [&](const type::Matrix* mat) {
                    // Reassemble a std140 matrix from the structure of column vector members.
                    auto std140_mat = std140_mats.Get(mat);
                    if (TINT_LIKELY(std140_mat)) {
                        utils::Vector<const ast::Expression*, 8> args;
                        // std140 decomposed matrix. Reassemble.
                        auto mat_ty = CreateASTTypeFor(ctx, mat);
                        auto mat_args = utils::Transform(std140_mat->columns, [&](Symbol name) {
                            return b.MemberAccessor(param, name);
                        });
                        stmts.Push(b.Return(b.Call(mat_ty, std::move(mat_args))));
                    } else {
                        TINT_ICE(Transform, b.Diagnostics())
                            << "failed to find std140 matrix info for: " << ty->FriendlyName();
                    }
                },  //
                [&](const type::Array* arr) {
                    // Converting an array. Create a function var for the converted array, and
                    // loop over the input elements, converting each and assigning the result to
                    // the local array.
                    auto* var = b.Var("arr", CreateASTTypeFor(ctx, ty));
                    auto* i = b.Var("i", b.ty.u32());
                    auto* dst_el = b.IndexAccessor(var, i);
                    auto* src_el = Convert(arr->ElemType(), b.IndexAccessor(param, i));
                    auto count = arr->ConstantCount();
                    if (TINT_UNLIKELY(!count)) {
                        // Non-constant counts should not be possible:
                        // * Override-expression counts can only be applied to workgroup arrays, and
                        //   this method only handles types transitively used as uniform buffers.
                        // * Runtime-sized arrays cannot be used in uniform buffers.
                        TINT_ICE(Transform, b.Diagnostics())
                            << "unexpected non-constant array count";
                        count = 1;
                    }
                    stmts.Push(b.Decl(var));
                    stmts.Push(b.For(b.Decl(i),                          //
                                     b.LessThan(i, u32(count.value())),  //
                                     b.Assign(i, b.Add(i, 1_a)),         //
                                     b.Block(b.Assign(dst_el, src_el))));
                    stmts.Push(b.Return(var));
                },
                [&](Default) {
                    TINT_ICE(Transform, b.Diagnostics())
                        << "unhandled type for conversion: " << ty->FriendlyName();
                });

            // Generate the function
            auto ret_ty = CreateASTTypeFor(ctx, ty);
            auto fn_sym = b.Symbols().New("conv_" + ConvertSuffix(ty));
            b.Func(fn_sym, utils::Vector{param}, ret_ty, std::move(stmts));
            return fn_sym;
        });

        if (!fn.IsValid()) {
            // Not a std140 type, nothing to convert.
            return expr;
        }

        // Call the helper
        return b.Call(fn, utils::Vector{expr});
    }

    /// Loads a part of, or a whole std140-decomposed matrix from a uniform buffer, using a helper
    /// function which will be generated if it hasn't been already.
    /// @param access the access chain from the uniform buffer to either the whole matrix or part of
    ///        the matrix (column, column-swizzle, or element).
    /// @returns the loaded value expression.
    const ast::Expression* LoadMatrixWithFn(const AccessChain& access) {
        // Get an existing, or create a new function for loading the uniform buffer value.
        // This function is keyed off the uniform buffer variable and the access chain.
        auto fn = load_fns.GetOrCreate(LoadFnKey{access.var, access.indices}, [&] {
            if (access.IsMatrixSubset()) {
                // Access chain passes through the matrix, but ends either at a column vector,
                // column swizzle, or element.
                return BuildLoadPartialMatrixFn(access);
            }
            // Access is to the whole matrix.
            return BuildLoadWholeMatrixFn(access);
        });

        // Build the arguments
        auto args = utils::Transform(access.dynamic_indices, [&](const sem::ValueExpression* e) {
            return b.Call<u32>(ctx.Clone(e->Declaration()));
        });

        // Call the helper
        return b.Call(fn, std::move(args));
    }

    /// Loads a part of a std140-decomposed matrix from a uniform buffer, inline (without calling a
    /// helper function).
    /// @param chain the access chain from the uniform buffer to part of the matrix (column,
    ///               column-swizzle, or element).
    /// @note The matrix column must be statically indexed to use this method.
    /// @returns the loaded value expression.
    const ast::Expression* LoadSubMatrixInline(const AccessChain& chain) {
        // Method for generating dynamic index expressions.
        // As this is inline, we can just clone the expression.
        auto dynamic_index = [&](size_t idx) {
            return ctx.Clone(chain.dynamic_indices[idx]->Declaration());
        };

        const ast::Expression* expr = nullptr;
        const type::Type* ty = nullptr;

        // Build the expression up to, but not including the matrix member
        auto std140_mat_idx = *chain.std140_mat_idx;
        for (size_t i = 0; i < std140_mat_idx; i++) {
            auto [new_expr, new_ty, _] = BuildAccessExpr(expr, ty, chain, i, dynamic_index);
            expr = new_expr;
            ty = new_ty;
        }

        // Access is to the std140 decomposed matrix.
        // As this is accessing only part of the matrix, we just need to pick the right column
        // vector member.
        auto column_idx = std::get<u32>(chain.indices[std140_mat_idx + 1]);
        if (auto* str = tint::As<type::Struct>(ty)) {
            // Structure member matrix. The columns are decomposed into the structure.
            auto mat_member_idx = std::get<u32>(chain.indices[std140_mat_idx]);
            auto* mat_member = str->Members()[mat_member_idx];
            auto mat_columns = *std140_mat_members.Get(mat_member);
            expr = b.MemberAccessor(expr, mat_columns[column_idx]->name->symbol);
            ty = mat_member->Type()->As<type::Matrix>()->ColumnType();
        } else {
            // Non-structure-member matrix. The columns are decomposed into a new, bespoke std140
            // structure.
            auto [new_expr, new_ty, _] =
                BuildAccessExpr(expr, ty, chain, std140_mat_idx, dynamic_index);
            expr = new_expr;
            ty = new_ty;
            auto* mat = ty->As<type::Matrix>();
            auto std140_mat = std140_mats.Get(ty->As<type::Matrix>());
            expr = b.MemberAccessor(expr, std140_mat->columns[column_idx]);
            ty = mat->ColumnType();
        }

        // Build any remaining accesses into the column
        for (size_t i = std140_mat_idx + 2; i < chain.indices.Length(); i++) {
            auto [new_expr, new_ty, _] = BuildAccessExpr(expr, ty, chain, i, dynamic_index);
            expr = new_expr;
            ty = new_ty;
        }
        return expr;
    }

    /// Generates a function to load part of a std140-decomposed matrix from a uniform buffer.
    /// The generated function will have a parameter per dynamic (runtime-evaluated) index in the
    /// access chain.
    /// The generated function uses a WGSL switch statement to dynamically select the decomposed
    /// matrix column.
    /// @param chain the access chain from the uniform buffer to part of the matrix (column,
    ///               column-swizzle, or element).
    /// @note The matrix column must be dynamically indexed to use this method.
    /// @returns the generated function name.
    Symbol BuildLoadPartialMatrixFn(const AccessChain& chain) {
        // Build the dynamic index parameters
        auto dynamic_index_params = utils::Transform(chain.dynamic_indices, [&](auto*, size_t i) {
            return b.Param("p" + std::to_string(i), b.ty.u32());
        });
        // Method for generating dynamic index expressions.
        // These are passed in as arguments to the function.
        auto dynamic_index = [&](size_t idx) {
            return b.Expr(dynamic_index_params[idx]->name->symbol);
        };

        // Fetch the access chain indices of the matrix access and the parameter index that
        // holds the matrix column index.
        auto std140_mat_idx = *chain.std140_mat_idx;
        auto column_param_idx = std::get<DynamicIndex>(chain.indices[std140_mat_idx + 1]).slot;

        // Begin building the function name. This is extended with logic in the loop below
        // (when column_idx == 0).
        std::string name = "load";

        // The switch cases
        utils::Vector<const ast::CaseStatement*, 4> cases;

        // The function return type.
        const type::Type* ret_ty = nullptr;

        // Build switch() cases for each column of the matrix
        auto num_columns = chain.std140_mat_ty->columns();
        for (uint32_t column_idx = 0; column_idx < num_columns; column_idx++) {
            const ast::Expression* expr = nullptr;
            const type::Type* ty = nullptr;

            // Build the expression up to, but not including the matrix
            for (size_t i = 0; i < std140_mat_idx; i++) {
                auto [new_expr, new_ty, access_name] =
                    BuildAccessExpr(expr, ty, chain, i, dynamic_index);
                expr = new_expr;
                ty = new_ty;
                if (column_idx == 0) {
                    name += "_" + access_name;
                }
            }

            if (auto* str = tint::As<type::Struct>(ty)) {
                // Structure member matrix. The columns are decomposed into the structure.
                auto mat_member_idx = std::get<u32>(chain.indices[std140_mat_idx]);
                auto* mat_member = str->Members()[mat_member_idx];
                if (column_idx == 0) {
                    name +=
                        "_" + mat_member->Name().Name() + "_p" + std::to_string(column_param_idx);
                }
                auto mat_columns = *std140_mat_members.Get(mat_member);
                expr = b.MemberAccessor(expr, mat_columns[column_idx]->name->symbol);
                ty = mat_member->Type()->As<type::Matrix>()->ColumnType();
            } else {
                // Non-structure-member matrix. The columns are decomposed into a new, bespoke
                // std140 structure.
                auto [new_expr, new_ty, mat_name] =
                    BuildAccessExpr(expr, ty, chain, std140_mat_idx, dynamic_index);
                expr = new_expr;
                ty = new_ty;
                if (column_idx == 0) {
                    name += "_" + mat_name + "_p" + std::to_string(column_param_idx);
                }
                auto* mat = ty->As<type::Matrix>();
                auto std140_mat = std140_mats.Get(ty->As<type::Matrix>());
                expr = b.MemberAccessor(expr, std140_mat->columns[column_idx]);
                ty = mat->ColumnType();
            }

            // Build the rest of the expression, skipping over the column index.
            for (size_t i = std140_mat_idx + 2; i < chain.indices.Length(); i++) {
                auto [new_expr, new_ty, access_name] =
                    BuildAccessExpr(expr, ty, chain, i, dynamic_index);
                expr = new_expr;
                ty = new_ty;
                if (column_idx == 0) {
                    name += "_" + access_name;
                }
            }

            if (column_idx == 0) {
                ret_ty = ty;
            }

            auto* case_sel = b.CaseSelector(b.Expr(u32(column_idx)));
            auto* case_body = b.Block(utils::Vector{b.Return(expr)});
            cases.Push(b.Case(case_sel, case_body));
        }

        // Build the default case (required in WGSL).
        // This just returns a zero value of the return type, as the index must be out of
        // bounds.
        cases.Push(b.DefaultCase(b.Block(b.Return(b.Call(CreateASTTypeFor(ctx, ret_ty))))));

        auto* column_selector = dynamic_index(column_param_idx);
        auto* stmt = b.Switch(column_selector, std::move(cases));

        auto fn_sym = b.Symbols().New(name);
        b.Func(fn_sym, std::move(dynamic_index_params), CreateASTTypeFor(ctx, ret_ty),
               utils::Vector{stmt});
        return fn_sym;
    }

    /// Generates a function to load a whole std140-decomposed matrix from a uniform buffer.
    /// The generated function will have a parameter per dynamic (runtime-evaluated) index in the
    /// access chain.
    /// @param chain the access chain from the uniform buffer to the whole std140-decomposed
    ///        matrix.
    /// @returns the generated function name.
    Symbol BuildLoadWholeMatrixFn(const AccessChain& chain) {
        // Build the dynamic index parameters
        auto dynamic_index_params = utils::Transform(chain.dynamic_indices, [&](auto*, size_t i) {
            return b.Param("p" + std::to_string(i), b.ty.u32());
        });
        // Method for generating dynamic index expressions.
        // These are passed in as arguments to the function.
        auto dynamic_index = [&](size_t idx) {
            return b.Expr(dynamic_index_params[idx]->name->symbol);
        };

        const ast::Expression* expr = nullptr;
        const type::Type* ty = nullptr;
        std::string name = "load";

        // Build the expression up to, but not including the matrix member
        auto std140_mat_idx = *chain.std140_mat_idx;
        for (size_t i = 0; i < std140_mat_idx; i++) {
            auto [new_expr, new_ty, access_name] =
                BuildAccessExpr(expr, ty, chain, i, dynamic_index);
            expr = new_expr;
            ty = new_ty;
            name += "_" + access_name;
        }

        utils::Vector<const ast::Statement*, 2> stmts;

        // Create a temporary pointer to the structure that holds the matrix columns
        auto* let = b.Let("s", b.AddressOf(expr));
        stmts.Push(b.Decl(let));

        utils::Vector<const ast::MemberAccessorExpression*, 4> columns;
        if (auto* str = tint::As<type::Struct>(ty)) {
            // Structure member matrix. The columns are decomposed into the structure.
            auto mat_member_idx = std::get<u32>(chain.indices[std140_mat_idx]);
            auto* mat_member = str->Members()[mat_member_idx];
            auto mat_columns = *std140_mat_members.Get(mat_member);
            columns = utils::Transform(mat_columns, [&](auto* column_member) {
                return b.MemberAccessor(b.Deref(let), column_member->name->symbol);
            });
            ty = mat_member->Type();
            name += "_" + mat_member->Name().Name();
        } else {
            // Non-structure-member matrix. The columns are decomposed into a new, bespoke
            // std140 structure.
            auto [new_expr, new_ty, mat_name] =
                BuildAccessExpr(expr, ty, chain, std140_mat_idx, dynamic_index);
            expr = new_expr;
            auto* mat = ty->As<type::Matrix>();
            auto std140_mat = std140_mats.Get(ty->As<type::Matrix>());
            columns = utils::Transform(std140_mat->columns, [&](auto column_name) {
                return b.MemberAccessor(b.Deref(let), column_name);
            });
            ty = mat;
            name += "_" + mat_name;
        }

        // Reconstruct the matrix from the columns
        expr = b.Call(CreateASTTypeFor(ctx, chain.std140_mat_ty), std::move(columns));

        // Have the function return the constructed matrix
        stmts.Push(b.Return(expr));

        // Build the function
        auto ret_ty = CreateASTTypeFor(ctx, ty);
        auto fn_sym = b.Symbols().New(name);
        b.Func(fn_sym, std::move(dynamic_index_params), ret_ty, std::move(stmts));
        return fn_sym;
    }

    /// Return type of BuildAccessExpr()
    struct ExprTypeName {
        /// The new, post-access expression
        const ast::Expression* expr;
        /// The type of #expr
        const type::Type* type;
        /// A name segment which can be used to build sensible names for helper functions
        std::string name;
    };

    /// Builds a single access in an access chain.
    /// @param lhs the expression to index using @p access
    /// @param ty the type of the expression @p lhs
    /// @param chain the access index to perform on @p lhs
    /// @param dynamic_index a function that obtains the i'th dynamic index
    /// @returns a ExprTypeName which holds the new expression, new type and a name segment which
    ///          can be used for creating helper function names.
    ExprTypeName BuildAccessExpr(const ast::Expression* lhs,
                                 const type::Type* ty,
                                 const AccessChain& chain,
                                 size_t index,
                                 std::function<const ast::Expression*(size_t)> dynamic_index) {
        auto& access = chain.indices[index];

        if (std::get_if<UniformVariable>(&access)) {
            const auto symbol = chain.var->Declaration()->name->symbol;
            const auto* expr = b.Expr(ctx.Clone(symbol));
            const auto name = symbol.Name();
            ty = chain.var->Type()->UnwrapRef();
            return {expr, ty, name};
        }

        if (auto* dyn_idx = std::get_if<DynamicIndex>(&access)) {
            /// The access uses a dynamic (runtime-expression) index.
            auto name = "p" + std::to_string(dyn_idx->slot);
            return Switch(
                ty,  //
                [&](const type::Array* arr) -> ExprTypeName {
                    auto* idx = dynamic_index(dyn_idx->slot);
                    auto* expr = b.IndexAccessor(lhs, idx);
                    return {expr, arr->ElemType(), name};
                },  //
                [&](const type::Matrix* mat) -> ExprTypeName {
                    auto* idx = dynamic_index(dyn_idx->slot);
                    auto* expr = b.IndexAccessor(lhs, idx);
                    return {expr, mat->ColumnType(), name};
                },  //
                [&](const type::Vector* vec) -> ExprTypeName {
                    auto* idx = dynamic_index(dyn_idx->slot);
                    auto* expr = b.IndexAccessor(lhs, idx);
                    return {expr, vec->type(), name};
                },  //
                [&](Default) -> ExprTypeName {
                    TINT_ICE(Transform, b.Diagnostics())
                        << "unhandled type for access chain: " << ty->FriendlyName();
                    return {};
                });
        }
        if (auto* swizzle = std::get_if<Swizzle>(&access)) {
            /// The access is a vector swizzle.
            return Switch(
                ty,  //
                [&](const type::Vector* vec) -> ExprTypeName {
                    static const char xyzw[] = {'x', 'y', 'z', 'w'};
                    std::string rhs;
                    for (auto el : *swizzle) {
                        rhs += xyzw[el];
                    }
                    auto swizzle_ty = src->Types().Find<type::Vector>(
                        vec->type(), static_cast<uint32_t>(swizzle->Length()));
                    auto* expr = b.MemberAccessor(lhs, rhs);
                    return {expr, swizzle_ty, rhs};
                },  //
                [&](Default) -> ExprTypeName {
                    TINT_ICE(Transform, b.Diagnostics())
                        << "unhandled type for access chain: " << ty->FriendlyName();
                    return {};
                });
        }
        /// The access is a static index.
        auto idx = std::get<u32>(access);
        return Switch(
            ty,  //
            [&](const type::Struct* str) -> ExprTypeName {
                auto* member = str->Members()[idx];
                auto member_name = member->Name().Name();
                auto* expr = b.MemberAccessor(lhs, member_name);
                ty = member->Type();
                return {expr, ty, member_name};
            },  //
            [&](const type::Array* arr) -> ExprTypeName {
                auto* expr = b.IndexAccessor(lhs, idx);
                return {expr, arr->ElemType(), std::to_string(idx)};
            },  //
            [&](const type::Matrix* mat) -> ExprTypeName {
                auto* expr = b.IndexAccessor(lhs, idx);
                return {expr, mat->ColumnType(), std::to_string(idx)};
            },  //
            [&](const type::Vector* vec) -> ExprTypeName {
                auto* expr = b.IndexAccessor(lhs, idx);
                return {expr, vec->type(), std::to_string(idx)};
            },  //
            [&](Default) -> ExprTypeName {
                TINT_ICE(Transform, b.Diagnostics())
                    << "unhandled type for access chain: " << ty->FriendlyName();
                return {};
            });
    }
};

Std140::Std140() = default;

Std140::~Std140() = default;

Transform::ApplyResult Std140::Apply(const Program* src, const DataMap&, DataMap&) const {
    return State(src).Run();
}

}  // namespace tint::transform
