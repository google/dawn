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

#include "src/tint/transform/packed_vec3.h"

#include <algorithm>
#include <string>
#include <utility>

#include "src/tint/ast/assignment_statement.h"
#include "src/tint/builtin/builtin.h"
#include "src/tint/program_builder.h"
#include "src/tint/sem/array_count.h"
#include "src/tint/sem/index_accessor_expression.h"
#include "src/tint/sem/load.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/type_expression.h"
#include "src/tint/sem/variable.h"
#include "src/tint/switch.h"
#include "src/tint/type/array.h"
#include "src/tint/type/reference.h"
#include "src/tint/type/vector.h"
#include "src/tint/utils/hashmap.h"
#include "src/tint/utils/hashset.h"
#include "src/tint/utils/vector.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::PackedVec3);

using namespace tint::number_suffixes;  // NOLINT

namespace tint::transform {

/// PIMPL state for the transform
struct PackedVec3::State {
    /// Constructor
    /// @param program the source program
    explicit State(const Program* program) : src(program) {}

    /// The name of the struct member used when wrapping packed vec3 types.
    static constexpr const char* kStructMemberName = "elements";

    /// The names of the structures used to wrap packed vec3 types.
    utils::Hashmap<const type::Type*, Symbol, 4> packed_vec3_wrapper_struct_names;

    /// A cache of host-shareable structures that have been rewritten.
    utils::Hashmap<const type::Type*, Symbol, 4> rewritten_structs;

    /// A map from type to the name of a helper function used to pack that type.
    utils::Hashmap<const type::Type*, Symbol, 4> pack_helpers;

    /// A map from type to the name of a helper function used to unpack that type.
    utils::Hashmap<const type::Type*, Symbol, 4> unpack_helpers;

    /// @param ty the type to test
    /// @returns true if `ty` is a vec3, false otherwise
    bool IsVec3(const type::Type* ty) {
        if (auto* vec = ty->As<type::Vector>()) {
            if (vec->Width() == 3) {
                return true;
            }
        }
        return false;
    }

    /// @param ty the type to test
    /// @returns true if `ty` is or contains a vec3, false otherwise
    bool ContainsVec3(const type::Type* ty) {
        return Switch(
            ty,  //
            [&](const type::Vector* vec) { return IsVec3(vec); },
            [&](const type::Matrix* mat) { return ContainsVec3(mat->ColumnType()); },
            [&](const type::Array* arr) { return ContainsVec3(arr->ElemType()); },
            [&](const type::Struct* str) {
                for (auto* member : str->Members()) {
                    if (ContainsVec3(member->Type())) {
                        return true;
                    }
                }
                return false;
            });
    }

    /// Create a `__packed_vec3` type with the same element type as `ty`.
    /// @param ty a three-element vector type
    /// @returns the new AST type
    ast::Type MakePackedVec3(const type::Type* ty) {
        auto* vec = ty->As<type::Vector>();
        TINT_ASSERT(Transform, vec != nullptr && vec->Width() == 3);
        return b.ty(builtin::Builtin::kPackedVec3, CreateASTTypeFor(ctx, vec->type()));
    }

    /// Recursively rewrite a type using `__packed_vec3`, if needed.
    /// When used as an array element type, the `__packed_vec3` type will be wrapped in a structure
    /// and given an `@align()` attribute to give it alignment it needs to yield the correct array
    /// element stride. For vec3 types used in structures directly, the `@align()` attribute is
    /// placed on the containing structure instead. Matrices with three rows become arrays of
    /// columns, and used the aligned wrapper struct for the column type.
    /// @param ty the type to rewrite
    /// @param array_element `true` if this is being called for the element of an array
    /// @returns the new AST type, or nullptr if rewriting was not necessary
    ast::Type RewriteType(const type::Type* ty, bool array_element = false) {
        return Switch(
            ty,
            [&](const type::Vector* vec) -> ast::Type {
                if (IsVec3(vec)) {
                    if (array_element) {
                        // Create a struct with a single `__packed_vec3` member.
                        // Give the struct member the same alignment as the original unpacked vec3
                        // type, to avoid changing the array element stride.
                        return b.ty(packed_vec3_wrapper_struct_names.GetOrCreate(vec, [&]() {
                            auto name = b.Symbols().New(
                                "tint_packed_vec3_" + vec->type()->FriendlyName() +
                                (array_element ? "_array_element" : "_struct_member"));
                            auto* member =
                                b.Member(kStructMemberName, MakePackedVec3(vec),
                                         utils::Vector{b.MemberAlign(AInt(vec->Align()))});
                            b.Structure(b.Ident(name), utils::Vector{member}, utils::Empty);
                            return name;
                        }));
                    } else {
                        return MakePackedVec3(vec);
                    }
                }
                return {};
            },
            [&](const type::Matrix* mat) -> ast::Type {
                // Rewrite the matrix as an array of columns that use the aligned wrapper struct.
                auto new_col_type = RewriteType(mat->ColumnType(), /* array_element */ true);
                if (new_col_type) {
                    return b.ty.array(new_col_type, u32(mat->columns()));
                }
                return {};
            },
            [&](const type::Array* arr) -> ast::Type {
                // Rewrite the array with the modified element type.
                auto new_type = RewriteType(arr->ElemType(), /* array_element */ true);
                if (new_type) {
                    utils::Vector<const ast::Attribute*, 1> attrs;
                    if (arr->Count()->Is<type::RuntimeArrayCount>()) {
                        return b.ty.array(new_type, std::move(attrs));
                    } else if (auto count = arr->ConstantCount()) {
                        return b.ty.array(new_type, u32(count.value()), std::move(attrs));
                    } else {
                        TINT_ICE(Transform, b.Diagnostics())
                            << type::Array::kErrExpectedConstantCount;
                        return {};
                    }
                }
                return {};
            },
            [&](const type::Struct* str) -> ast::Type {
                if (ContainsVec3(str)) {
                    auto name = rewritten_structs.GetOrCreate(str, [&]() {
                        utils::Vector<const ast::StructMember*, 4> members;
                        for (auto* member : str->Members()) {
                            // If the member type contains a vec3, rewrite it.
                            auto new_type = RewriteType(member->Type());
                            if (new_type) {
                                // Copy the member attributes.
                                bool needs_align = true;
                                utils::Vector<const ast::Attribute*, 4> attributes;
                                if (auto* sem_mem = member->As<sem::StructMember>()) {
                                    for (auto* attr : sem_mem->Declaration()->attributes) {
                                        if (attr->IsAnyOf<ast::StructMemberAlignAttribute,
                                                          ast::StructMemberOffsetAttribute>()) {
                                            needs_align = false;
                                        }
                                        attributes.Push(ctx.Clone(attr));
                                    }
                                }
                                // If the alignment wasn't already specified, add an attribute to
                                // make sure that we don't alter the alignment when using the packed
                                // vector type.
                                if (needs_align) {
                                    attributes.Push(b.MemberAlign(AInt(member->Align())));
                                }
                                members.Push(b.Member(ctx.Clone(member->Name()), new_type,
                                                      std::move(attributes)));
                            } else {
                                // No vec3s, just clone the member as is.
                                if (auto* sem_mem = member->As<sem::StructMember>()) {
                                    members.Push(ctx.Clone(sem_mem->Declaration()));
                                } else {
                                    members.Push(b.Member(ctx.Clone(member->Name()), new_type,
                                                          utils::Empty));
                                }
                            }
                        }
                        // Create the new structure.
                        auto struct_name =
                            b.Symbols().New(str->Name().Name() + "_tint_packed_vec3");
                        b.Structure(struct_name, std::move(members));
                        return struct_name;
                    });
                    return b.ty(name);
                }
                return {};
            });
    }

    /// Create a helper function to recursively pack or unpack a composite that contains vec3 types.
    /// @param name_prefix the name of the helper function
    /// @param ty the composite type to pack or unpack
    /// @param pack_or_unpack_element a function that packs or unpacks an element with a given type
    /// @param in_type a function that create an AST type for the input type
    /// @param out_type a function that create an AST type for the output type
    /// @returns the name of the helper function
    Symbol MakePackUnpackHelper(
        const char* name_prefix,
        const type::Type* ty,
        const std::function<const ast::Expression*(const ast::Expression*, const type::Type*)>&
            pack_or_unpack_element,
        const std::function<ast::Type()>& in_type,
        const std::function<ast::Type()>& out_type) {
        // Allocate a variable to hold the return value of the function.
        utils::Vector<const ast::Statement*, 4> statements;
        statements.Push(b.Decl(b.Var("result", out_type())));

        // Helper that generates a loop to copy and pack/unpack elements of an array to the result:
        //   for (var i = 0u; i < num_elements; i = i + 1) {
        //     result[i] = pack_or_unpack_element(in[i]);
        //   }
        auto copy_array_elements = [&](uint32_t num_elements, const type::Type* element_type) {
            // Generate an expression for packing or unpacking an element of the array.
            auto* element = pack_or_unpack_element(b.IndexAccessor("in", "i"), element_type);
            statements.Push(b.For(                   //
                b.Decl(b.Var("i", b.ty.u32())),      //
                b.LessThan("i", u32(num_elements)),  //
                b.Assign("i", b.Add("i", 1_a)),      //
                b.Block(utils::Vector{
                    b.Assign(b.IndexAccessor("result", "i"), element),
                })));
        };

        // Copy the elements of the value over to the result.
        Switch(
            ty,
            [&](const type::Array* arr) {
                TINT_ASSERT(Transform, arr->ConstantCount());
                copy_array_elements(arr->ConstantCount().value(), arr->ElemType());
            },
            [&](const type::Matrix* mat) {
                copy_array_elements(mat->columns(), mat->ColumnType());
            },
            [&](const type::Struct* str) {
                // Copy the struct members over one at a time, packing/unpacking as necessary.
                for (auto* member : str->Members()) {
                    const ast::Expression* element =
                        b.MemberAccessor("in", b.Ident(ctx.Clone(member->Name())));
                    if (ContainsVec3(member->Type())) {
                        element = pack_or_unpack_element(element, member->Type());
                    }
                    statements.Push(b.Assign(
                        b.MemberAccessor("result", b.Ident(ctx.Clone(member->Name()))), element));
                }
            });

        // Return the result.
        statements.Push(b.Return("result"));

        // Create the function and return its name.
        auto name = b.Symbols().New(name_prefix);
        b.Func(name, utils::Vector{b.Param("in", in_type())}, out_type(), std::move(statements));
        return name;
    }

    /// Unpack the composite value `expr` to the unpacked type `ty`. If `ty` is a matrix, this will
    /// produce a regular matNx3 value from an array of packed column vectors.
    /// @param expr the composite value expression to unpack
    /// @param ty the unpacked type
    /// @returns an expression that holds the unpacked value
    const ast::Expression* UnpackComposite(const ast::Expression* expr, const type::Type* ty) {
        auto helper = unpack_helpers.GetOrCreate(ty, [&]() {
            return MakePackUnpackHelper(
                "tint_unpack_vec3_in_composite", ty,
                [&](const ast::Expression* element,
                    const type::Type* element_type) -> const ast::Expression* {
                    if (element_type->Is<type::Vector>()) {
                        // Unpack a `__packed_vec3` by casting it to a regular vec3.
                        // If it is an array element, extract the vector from the wrapper struct.
                        if (element->Is<ast::IndexAccessorExpression>()) {
                            element = b.MemberAccessor(element, kStructMemberName);
                        }
                        return b.Call(CreateASTTypeFor(ctx, element_type), element);
                    } else {
                        return UnpackComposite(element, element_type);
                    }
                },
                [&]() { return RewriteType(ty); },  //
                [&]() { return CreateASTTypeFor(ctx, ty); });
        });
        return b.Call(helper, expr);
    }

    /// Pack the composite value `expr` from the unpacked type `ty`. If `ty` is a matrix, this will
    /// produce an array of packed column vectors.
    /// @param expr the composite value expression to pack
    /// @param ty the unpacked type
    /// @returns an expression that holds the packed value
    const ast::Expression* PackComposite(const ast::Expression* expr, const type::Type* ty) {
        auto helper = pack_helpers.GetOrCreate(ty, [&]() {
            return MakePackUnpackHelper(
                "tint_pack_vec3_in_composite", ty,
                [&](const ast::Expression* element,
                    const type::Type* element_type) -> const ast::Expression* {
                    if (element_type->Is<type::Vector>()) {
                        // Pack a vector element by casting it to a packed_vec3.
                        // If it is an array element, construct a wrapper struct.
                        auto* packed = b.Call(MakePackedVec3(element_type), element);
                        if (element->Is<ast::IndexAccessorExpression>()) {
                            packed = b.Call(RewriteType(element_type, true), packed);
                        }
                        return packed;
                    } else {
                        return PackComposite(element, element_type);
                    }
                },
                [&]() { return CreateASTTypeFor(ctx, ty); },  //
                [&]() { return RewriteType(ty); });
        });
        return b.Call(helper, expr);
    }

    /// @returns true if there are host-shareable vec3's that need transforming
    bool ShouldRun() {
        // Check for vec3s in the types of all uniform and storage buffer variables to determine
        // if the transform is necessary.
        for (auto* decl : src->AST().GlobalVariables()) {
            auto* var = sem.Get<sem::GlobalVariable>(decl);
            if (var && builtin::IsHostShareable(var->AddressSpace()) &&
                ContainsVec3(var->Type()->UnwrapRef())) {
                return true;
            }
        }
        return false;
    }

    /// Runs the transform
    /// @returns the new program or SkipTransform if the transform is not required
    ApplyResult Run() {
        if (!ShouldRun()) {
            return SkipTransform;
        }

        // Changing the types of certain structure members can trigger stricter layout validation
        // rules for the uniform address space. In particular, replacing 16-bit matrices with arrays
        // violates the requirement that the array element stride is a multiple of 16 bytes, and
        // replacing vec3s with a structure violates the requirement that there must be at least 16
        // bytes from the start of a structure to the start of the next member.
        // Disable these validation rules using an internal extension, as MSL does not have these
        // restrictions.
        b.Enable(builtin::Extension::kChromiumInternalRelaxedUniformLayout);

        // Track expressions that need to be packed or unpacked.
        utils::Hashset<const sem::ValueExpression*, 8> to_pack;
        utils::Hashset<const sem::ValueExpression*, 8> to_unpack;

        // Replace vec3 types in host-shareable address spaces with `__packed_vec3` types, and
        // collect expressions that need to be converted to or from values that use the
        // `__packed_vec3` type.
        for (auto* node : ctx.src->ASTNodes().Objects()) {
            Switch(
                sem.Get(node),
                [&](const sem::TypeExpression* type) {
                    // Rewrite pointers to types that contain vec3s.
                    auto* ptr = type->Type()->As<type::Pointer>();
                    if (ptr && builtin::IsHostShareable(ptr->AddressSpace())) {
                        auto new_store_type = RewriteType(ptr->StoreType());
                        if (new_store_type) {
                            auto access = ptr->AddressSpace() == builtin::AddressSpace::kStorage
                                              ? ptr->Access()
                                              : builtin::Access::kUndefined;
                            auto new_ptr_type =
                                b.ty.pointer(new_store_type, ptr->AddressSpace(), access);
                            ctx.Replace(node, new_ptr_type.expr);
                        }
                    }
                },
                [&](const sem::Variable* var) {
                    if (!builtin::IsHostShareable(var->AddressSpace())) {
                        return;
                    }

                    // Rewrite the var type, if it contains vec3s.
                    auto new_store_type = RewriteType(var->Type()->UnwrapRef());
                    if (new_store_type) {
                        ctx.Replace(var->Declaration()->type.expr, new_store_type.expr);
                    }
                },
                [&](const sem::Statement* stmt) {
                    // Pack the RHS of assignment statements that are writing to packed types.
                    if (auto* assign = stmt->Declaration()->As<ast::AssignmentStatement>()) {
                        auto* lhs = sem.GetVal(assign->lhs);
                        auto* rhs = sem.GetVal(assign->rhs);
                        if (!ContainsVec3(rhs->Type()) ||
                            !builtin::IsHostShareable(
                                lhs->Type()->As<type::Reference>()->AddressSpace())) {
                            // Skip assignments to address spaces that are not host-shareable, or
                            // that do not contain vec3 types.
                            return;
                        }

                        // Pack the RHS expression.
                        if (to_unpack.Contains(rhs)) {
                            // The expression will already be packed, so skip the pending unpack.
                            to_unpack.Remove(rhs);

                            // If the expression produces a vec3 from an array element, extract
                            // the packed vector from the wrapper struct.
                            if (IsVec3(rhs->Type()) &&
                                rhs->UnwrapLoad()->Is<sem::IndexAccessorExpression>()) {
                                ctx.Replace(rhs->Declaration(),
                                            b.MemberAccessor(ctx.Clone(rhs->Declaration()),
                                                             kStructMemberName));
                            }
                        } else if (rhs) {
                            to_pack.Add(rhs);
                        }
                    }
                },
                [&](const sem::Load* load) {
                    // Unpack loads of types that contain vec3s in host-shareable address spaces.
                    if (ContainsVec3(load->Type()) &&
                        builtin::IsHostShareable(load->ReferenceType()->AddressSpace())) {
                        to_unpack.Add(load);
                    }
                },
                [&](const sem::IndexAccessorExpression* accessor) {
                    // If the expression produces a reference to a vec3 in a host-shareable address
                    // space from an array element, extract the packed vector from the wrapper
                    // struct.
                    if (auto* ref = accessor->Type()->As<type::Reference>()) {
                        if (IsVec3(ref->StoreType()) &&
                            builtin::IsHostShareable(ref->AddressSpace())) {
                            ctx.Replace(node, b.MemberAccessor(ctx.Clone(accessor->Declaration()),
                                                               kStructMemberName));
                        }
                    }
                });
        }

        // Sort the pending pack/unpack operations by AST node ID to make the order deterministic.
        auto to_unpack_sorted = to_unpack.Vector();
        auto to_pack_sorted = to_pack.Vector();
        auto pred = [&](auto* expr_a, auto* expr_b) {
            return expr_a->Declaration()->node_id < expr_b->Declaration()->node_id;
        };
        to_unpack_sorted.Sort(pred);
        to_pack_sorted.Sort(pred);

        // Apply all of the pending unpack operations that we have collected.
        for (auto* expr : to_unpack_sorted) {
            TINT_ASSERT(Transform, ContainsVec3(expr->Type()));
            auto* packed = ctx.Clone(expr->Declaration());
            const ast::Expression* unpacked = nullptr;
            if (IsVec3(expr->Type())) {
                if (expr->UnwrapLoad()->Is<sem::IndexAccessorExpression>()) {
                    // If we are unpacking a vec3 from an array element, extract the vector from the
                    // wrapper struct.
                    packed = b.MemberAccessor(packed, kStructMemberName);
                }
                // Cast the packed vector to a regular vec3.
                unpacked = b.Call(CreateASTTypeFor(ctx, expr->Type()), packed);
            } else {
                // Use a helper function to unpack an array or matrix.
                unpacked = UnpackComposite(packed, expr->Type());
            }
            TINT_ASSERT(Transform, unpacked != nullptr);
            ctx.Replace(expr->Declaration(), unpacked);
        }

        // Apply all of the pending pack operations that we have collected.
        for (auto* expr : to_pack_sorted) {
            TINT_ASSERT(Transform, ContainsVec3(expr->Type()));
            auto* unpacked = ctx.Clone(expr->Declaration());
            const ast::Expression* packed = nullptr;
            if (IsVec3(expr->Type())) {
                // Cast the regular vec3 to a packed vector type.
                packed = b.Call(MakePackedVec3(expr->Type()), unpacked);
            } else {
                // Use a helper function to pack an array or matrix.
                packed = PackComposite(unpacked, expr->Type());
            }
            TINT_ASSERT(Transform, packed != nullptr);
            ctx.Replace(expr->Declaration(), packed);
        }

        ctx.Clone();
        return Program(std::move(b));
    }

  private:
    /// The source program
    const Program* const src;
    /// The target program builder
    ProgramBuilder b;
    /// The clone context
    CloneContext ctx = {&b, src, /* auto_clone_symbols */ true};
    /// Alias to the semantic info in ctx.src
    const sem::Info& sem = ctx.src->Sem();
};

PackedVec3::PackedVec3() = default;
PackedVec3::~PackedVec3() = default;

Transform::ApplyResult PackedVec3::Apply(const Program* src, const DataMap&, DataMap&) const {
    return State{src}.Run();
}

}  // namespace tint::transform
