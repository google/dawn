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

#include "src/tint/transform/wrap_arrays_in_structs.h"

#include <utility>

#include "src/tint/program_builder.h"
#include "src/tint/sem/array.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/expression.h"
#include "src/tint/sem/type_constructor.h"
#include "src/tint/utils/map.h"
#include "src/tint/utils/transform.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::WrapArraysInStructs);

namespace tint::transform {

WrapArraysInStructs::WrappedArrayInfo::WrappedArrayInfo() = default;
WrapArraysInStructs::WrappedArrayInfo::WrappedArrayInfo(const WrappedArrayInfo&) = default;
WrapArraysInStructs::WrappedArrayInfo::~WrappedArrayInfo() = default;

WrapArraysInStructs::WrapArraysInStructs() = default;

WrapArraysInStructs::~WrapArraysInStructs() = default;

bool WrapArraysInStructs::ShouldRun(const Program* program, const DataMap&) const {
    for (auto* node : program->ASTNodes().Objects()) {
        if (program->Sem().Get<sem::Array>(node->As<ast::Type>())) {
            return true;
        }
    }
    return false;
}

void WrapArraysInStructs::Run(CloneContext& ctx, const DataMap&, DataMap&) const {
    auto& sem = ctx.src->Sem();

    std::unordered_map<const sem::Array*, WrappedArrayInfo> wrapped_arrays;
    auto wrapper = [&](const sem::Array* array) { return WrapArray(ctx, wrapped_arrays, array); };
    auto wrapper_typename = [&](const sem::Array* arr) -> ast::TypeName* {
        auto info = wrapper(arr);
        return info ? ctx.dst->create<ast::TypeName>(info.wrapper_name) : nullptr;
    };

    // Replace all array types with their corresponding wrapper
    ctx.ReplaceAll([&](const ast::Type* ast_type) -> const ast::Type* {
        auto* type = ctx.src->TypeOf(ast_type);
        if (auto* array = type->UnwrapRef()->As<sem::Array>()) {
            return wrapper_typename(array);
        }
        return nullptr;
    });

    // Fix up index accessors so `a[1]` becomes `a.arr[1]`
    ctx.ReplaceAll(
        [&](const ast::IndexAccessorExpression* accessor) -> const ast::IndexAccessorExpression* {
            if (auto* array =
                    ::tint::As<sem::Array>(sem.Get(accessor->object)->Type()->UnwrapRef())) {
                if (wrapper(array)) {
                    // Array is wrapped in a structure. Emit a member accessor to get
                    // to the actual array.
                    auto* arr = ctx.Clone(accessor->object);
                    auto* idx = ctx.Clone(accessor->index);
                    auto* unwrapped = ctx.dst->MemberAccessor(arr, "arr");
                    return ctx.dst->IndexAccessor(accessor->source, unwrapped, idx);
                }
            }
            return nullptr;
        });

    // Fix up array constructors so `A(1,2)` becomes `tint_array_wrapper(A(1,2))`
    ctx.ReplaceAll([&](const ast::CallExpression* expr) -> const ast::Expression* {
        if (auto* call = sem.Get(expr)) {
            if (auto* ctor = call->Target()->As<sem::TypeConstructor>()) {
                if (auto* array = ctor->ReturnType()->As<sem::Array>()) {
                    if (auto w = wrapper(array)) {
                        // Wrap the array type constructor with another constructor for
                        // the wrapper
                        auto* wrapped_array_ty = ctx.dst->ty.type_name(w.wrapper_name);
                        auto* array_ty = w.array_type(ctx);
                        auto args = utils::Transform(call->Arguments(),
                                                     [&](const tint::sem::Expression* s) {
                                                         return ctx.Clone(s->Declaration());
                                                     });
                        auto* arr_ctor = ctx.dst->Construct(array_ty, args);
                        return ctx.dst->Construct(wrapped_array_ty, arr_ctor);
                    }
                }
            }
        }
        return nullptr;
    });

    ctx.Clone();
}

WrapArraysInStructs::WrappedArrayInfo WrapArraysInStructs::WrapArray(
    CloneContext& ctx,
    std::unordered_map<const sem::Array*, WrappedArrayInfo>& wrapped_arrays,
    const sem::Array* array) const {
    if (array->IsRuntimeSized()) {
        return {};  // We don't want to wrap runtime sized arrays
    }

    return utils::GetOrCreate(wrapped_arrays, array, [&] {
        WrappedArrayInfo info;

        // Generate a unique name for the array wrapper
        info.wrapper_name = ctx.dst->Symbols().New("tint_array_wrapper");

        // Examine the element type. Is it also an array?
        std::function<const ast::Type*(CloneContext&)> el_type;
        if (auto* el_array = array->ElemType()->As<sem::Array>()) {
            // Array of array - call WrapArray() on the element type
            if (auto el = WrapArray(ctx, wrapped_arrays, el_array)) {
                el_type = [=](CloneContext& c) {
                    return c.dst->create<ast::TypeName>(el.wrapper_name);
                };
            }
        }

        // If the element wasn't an array, just create the typical AST type for it
        if (!el_type) {
            el_type = [=](CloneContext& c) { return CreateASTTypeFor(c, array->ElemType()); };
        }

        // Construct the single structure field type
        info.array_type = [=](CloneContext& c) {
            ast::AttributeList attrs;
            if (!array->IsStrideImplicit()) {
                attrs.emplace_back(c.dst->create<ast::StrideAttribute>(array->Stride()));
            }
            return c.dst->ty.array(el_type(c), array->Count(), std::move(attrs));
        };

        // Structure() will create and append the ast::Struct to the
        // global declarations of `ctx.dst`. As we haven't finished building the
        // current module-scope statement or function, this will be placed
        // immediately before the usage.
        ctx.dst->Structure(info.wrapper_name, {ctx.dst->Member("arr", info.array_type(ctx))});
        return info;
    });
}

}  // namespace tint::transform
