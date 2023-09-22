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

#include "src/tint/lang/spirv/reader/ast_lower/decompose_strided_array.h"

#include <unordered_map>
#include <utility>
#include <vector>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/wgsl/ast/transform/simplify_pointers.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/member_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/type_expression.h"
#include "src/tint/lang/wgsl/sem/value_constructor.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"
#include "src/tint/utils/containers/map.h"
#include "src/tint/utils/math/hash.h"

using namespace tint::core::fluent_types;  // NOLINT

TINT_INSTANTIATE_TYPEINFO(tint::spirv::reader::DecomposeStridedArray);

namespace tint::spirv::reader {
namespace {

using DecomposedArrays = std::unordered_map<const core::type::Array*, Symbol>;

bool ShouldRun(const Program& program) {
    for (auto* node : program.ASTNodes().Objects()) {
        if (auto* ident = node->As<ast::TemplatedIdentifier>()) {
            if (ast::GetAttribute<ast::StrideAttribute>(ident->attributes)) {
                return true;
            }
        }
    }
    return false;
}

}  // namespace

DecomposeStridedArray::DecomposeStridedArray() = default;

DecomposeStridedArray::~DecomposeStridedArray() = default;

ast::transform::Transform::ApplyResult DecomposeStridedArray::Apply(
    const Program& src,
    const ast::transform::DataMap&,
    ast::transform::DataMap&) const {
    if (!ShouldRun(src)) {
        return SkipTransform;
    }

    ProgramBuilder b;
    program::CloneContext ctx{&b, &src, /* auto_clone_symbols */ true};
    const auto& sem = src.Sem();

    static constexpr const char* kMemberName = "el";

    // Maps an array type in the source program to the name of the struct wrapper
    // type in the target program.
    std::unordered_map<const core::type::Array*, Symbol> decomposed;

    // Find and replace all arrays with a @stride attribute with a array that has
    // the @stride removed. If the source array stride does not match the natural
    // stride for the array element type, then replace the array element type with
    // a structure, holding a single field with a @size attribute equal to the
    // array stride.
    ctx.ReplaceAll([&](const ast::IdentifierExpression* expr) -> const ast::IdentifierExpression* {
        auto* ident = expr->identifier->As<ast::TemplatedIdentifier>();
        if (!ident) {
            return nullptr;
        }
        auto* type_expr = sem.Get<sem::TypeExpression>(expr);
        if (!type_expr) {
            return nullptr;
        }
        auto* arr = type_expr->Type()->As<core::type::Array>();
        if (!arr) {
            return nullptr;
        }
        if (!arr->IsStrideImplicit()) {
            auto el_ty = tint::GetOrCreate(decomposed, arr, [&] {
                auto name = b.Symbols().New("strided_arr");
                auto* member_ty = ctx.Clone(ident->arguments[0]->As<ast::IdentifierExpression>());
                auto* member = b.Member(kMemberName, ast::Type{member_ty},
                                        Vector{
                                            b.MemberSize(AInt(arr->Stride())),
                                        });
                b.Structure(name, Vector{member});
                return name;
            });
            if (ident->arguments.Length() > 1) {
                auto* count = ctx.Clone(ident->arguments[1]);
                return b.Expr(b.ty.array(b.ty(el_ty), count));
            } else {
                return b.Expr(b.ty.array(b.ty(el_ty)));
            }
        }
        if (ast::GetAttribute<ast::StrideAttribute>(ident->attributes)) {
            // Strip the @stride attribute
            auto* ty = ctx.Clone(ident->arguments[0]->As<ast::IdentifierExpression>());
            if (ident->arguments.Length() > 1) {
                auto* count = ctx.Clone(ident->arguments[1]);
                return b.Expr(b.ty.array(ast::Type{ty}, count));
            } else {
                return b.Expr(b.ty.array(ast::Type{ty}));
            }
        }
        return nullptr;
    });

    // Find all array index-accessors expressions for arrays that have had their
    // element changed to a single field structure. These expressions are adjusted
    // to insert an additional member accessor for the single structure field.
    // Example: `arr[i]` -> `arr[i].el`
    ctx.ReplaceAll([&](const ast::IndexAccessorExpression* idx) -> const ast::Expression* {
        if (auto* ty = src.TypeOf(idx->object)) {
            if (auto* arr = ty->UnwrapRef()->As<core::type::Array>()) {
                if (!arr->IsStrideImplicit()) {
                    auto* expr = ctx.CloneWithoutTransform(idx);
                    return b.MemberAccessor(expr, kMemberName);
                }
            }
        }
        return nullptr;
    });

    // Find all constructor expressions for array types that have had their element changed to a
    // single field structure. These constructors are adjusted to wrap each of the arguments with an
    // additional initializer for the new element structure type. Example:
    //   `@stride(32) array<i32, 3>(1, 2, 3)`
    // ->
    //   `array<strided_arr, 3>(strided_arr(1), strided_arr(2), strided_arr(3))`
    ctx.ReplaceAll([&](const ast::CallExpression* expr) -> const ast::Expression* {
        if (!expr->args.IsEmpty()) {
            if (auto* call = sem.Get(expr)->UnwrapMaterialize()->As<sem::Call>()) {
                if (auto* ctor = call->Target()->As<sem::ValueConstructor>()) {
                    if (auto* arr = ctor->ReturnType()->As<core::type::Array>()) {
                        // Begin by cloning the array initializer type or name
                        // If this is an unaliased array, this may add a new entry to
                        // decomposed.
                        // If this is an aliased array, decomposed should already be
                        // populated with any strided aliases.

                        auto* target = ctx.Clone(expr->target);

                        Vector<const ast::Expression*, 8> args;
                        if (auto it = decomposed.find(arr); it != decomposed.end()) {
                            args.Reserve(expr->args.Length());
                            for (auto* arg : expr->args) {
                                args.Push(b.Call(it->second, ctx.Clone(arg)));
                            }
                        } else {
                            args = ctx.Clone(expr->args);
                        }

                        return b.Call(target, std::move(args));
                    }
                }
            }
        }
        return nullptr;
    });

    ctx.Clone();
    return resolver::Resolve(b);
}

}  // namespace tint::spirv::reader
