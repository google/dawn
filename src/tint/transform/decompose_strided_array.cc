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

#include "src/tint/transform/decompose_strided_array.h"

#include <unordered_map>
#include <utility>
#include <vector>

#include "src/tint/program_builder.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/expression.h"
#include "src/tint/sem/member_accessor_expression.h"
#include "src/tint/sem/type_constructor.h"
#include "src/tint/transform/simplify_pointers.h"
#include "src/tint/utils/hash.h"
#include "src/tint/utils/map.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::DecomposeStridedArray);

namespace tint::transform {
namespace {

using DecomposedArrays = std::unordered_map<const sem::Array*, Symbol>;

}  // namespace

DecomposeStridedArray::DecomposeStridedArray() = default;

DecomposeStridedArray::~DecomposeStridedArray() = default;

bool DecomposeStridedArray::ShouldRun(const Program* program, const DataMap&) const {
    for (auto* node : program->ASTNodes().Objects()) {
        if (auto* ast = node->As<ast::Array>()) {
            if (ast::GetAttribute<ast::StrideAttribute>(ast->attributes)) {
                return true;
            }
        }
    }
    return false;
}

void DecomposeStridedArray::Run(CloneContext& ctx, const DataMap&, DataMap&) const {
    const auto& sem = ctx.src->Sem();

    static constexpr const char* kMemberName = "el";

    // Maps an array type in the source program to the name of the struct wrapper
    // type in the target program.
    std::unordered_map<const sem::Array*, Symbol> decomposed;

    // Find and replace all arrays with a @stride attribute with a array that has
    // the @stride removed. If the source array stride does not match the natural
    // stride for the array element type, then replace the array element type with
    // a structure, holding a single field with a @size attribute equal to the
    // array stride.
    ctx.ReplaceAll([&](const ast::Array* ast) -> const ast::Array* {
        if (auto* arr = sem.Get(ast)) {
            if (!arr->IsStrideImplicit()) {
                auto el_ty = utils::GetOrCreate(decomposed, arr, [&] {
                    auto name = ctx.dst->Symbols().New("strided_arr");
                    auto* member_ty = ctx.Clone(ast->type);
                    auto* member = ctx.dst->Member(kMemberName, member_ty,
                                                   {ctx.dst->MemberSize(arr->Stride())});
                    ctx.dst->Structure(name, {member});
                    return name;
                });
                auto* count = ctx.Clone(ast->count);
                return ctx.dst->ty.array(ctx.dst->ty.type_name(el_ty), count);
            }
            if (ast::GetAttribute<ast::StrideAttribute>(ast->attributes)) {
                // Strip the @stride attribute
                auto* ty = ctx.Clone(ast->type);
                auto* count = ctx.Clone(ast->count);
                return ctx.dst->ty.array(ty, count);
            }
        }
        return nullptr;
    });

    // Find all array index-accessors expressions for arrays that have had their
    // element changed to a single field structure. These expressions are adjusted
    // to insert an additional member accessor for the single structure field.
    // Example: `arr[i]` -> `arr[i].el`
    ctx.ReplaceAll([&](const ast::IndexAccessorExpression* idx) -> const ast::Expression* {
        if (auto* ty = ctx.src->TypeOf(idx->object)) {
            if (auto* arr = ty->UnwrapRef()->As<sem::Array>()) {
                if (!arr->IsStrideImplicit()) {
                    auto* expr = ctx.CloneWithoutTransform(idx);
                    return ctx.dst->MemberAccessor(expr, kMemberName);
                }
            }
        }
        return nullptr;
    });

    // Find all array type constructor expressions for array types that have had
    // their element changed to a single field structure. These constructors are
    // adjusted to wrap each of the arguments with an additional constructor for
    // the new element structure type.
    // Example:
    //   `@stride(32) array<i32, 3>(1, 2, 3)`
    // ->
    //   `array<strided_arr, 3>(strided_arr(1), strided_arr(2), strided_arr(3))`
    ctx.ReplaceAll([&](const ast::CallExpression* expr) -> const ast::Expression* {
        if (!expr->args.empty()) {
            if (auto* call = sem.Get(expr)) {
                if (auto* ctor = call->Target()->As<sem::TypeConstructor>()) {
                    if (auto* arr = ctor->ReturnType()->As<sem::Array>()) {
                        // Begin by cloning the array constructor type or name
                        // If this is an unaliased array, this may add a new entry to
                        // decomposed.
                        // If this is an aliased array, decomposed should already be
                        // populated with any strided aliases.
                        ast::CallExpression::Target target;
                        if (expr->target.type) {
                            target.type = ctx.Clone(expr->target.type);
                        } else {
                            target.name = ctx.Clone(expr->target.name);
                        }

                        ast::ExpressionList args;
                        if (auto it = decomposed.find(arr); it != decomposed.end()) {
                            args.reserve(expr->args.size());
                            for (auto* arg : expr->args) {
                                args.emplace_back(ctx.dst->Call(it->second, ctx.Clone(arg)));
                            }
                        } else {
                            args = ctx.Clone(expr->args);
                        }

                        return target.type ? ctx.dst->Construct(target.type, args)
                                           : ctx.dst->Call(target.name, args);
                    }
                }
            }
        }
        return nullptr;
    });
    ctx.Clone();
}

}  // namespace tint::transform
