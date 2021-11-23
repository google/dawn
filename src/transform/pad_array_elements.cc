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

#include "src/transform/pad_array_elements.h"

#include <unordered_map>
#include <utility>

#include "src/program_builder.h"
#include "src/sem/array.h"
#include "src/sem/call.h"
#include "src/sem/expression.h"
#include "src/sem/type_constructor.h"
#include "src/utils/map.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::PadArrayElements);

namespace tint {
namespace transform {
namespace {

using ArrayBuilder = std::function<const ast::Array*()>;

/// PadArray returns a function that constructs a new array in `ctx.dst` with
/// the element type padded to account for the explicit stride. PadArray will
/// recursively pad arrays-of-arrays. The new array element type will be added
/// to module-scope type declarations of `ctx.dst`.
/// @param ctx the CloneContext
/// @param create_ast_type_for Transform::CreateASTTypeFor()
/// @param padded_arrays a map of src array type to the new array name
/// @param array the array type
/// @return the new AST array
template <typename CREATE_AST_TYPE_FOR>
ArrayBuilder PadArray(
    CloneContext& ctx,
    CREATE_AST_TYPE_FOR&& create_ast_type_for,
    std::unordered_map<const sem::Array*, ArrayBuilder>& padded_arrays,
    const sem::Array* array) {
  if (array->IsStrideImplicit()) {
    // We don't want to wrap arrays that have an implicit stride
    return nullptr;
  }

  return utils::GetOrCreate(padded_arrays, array, [&] {
    // Generate a unique name for the array element type
    auto name = ctx.dst->Symbols().New("tint_padded_array_element");

    // Examine the element type. Is it also an array?
    const ast::Type* el_ty = nullptr;
    if (auto* el_array = array->ElemType()->As<sem::Array>()) {
      // Array of array - call PadArray() on the element type
      if (auto p =
              PadArray(ctx, create_ast_type_for, padded_arrays, el_array)) {
        el_ty = p();
      }
    }

    // If the element wasn't a padded array, just create the typical AST type
    // for it
    if (el_ty == nullptr) {
      el_ty = create_ast_type_for(ctx, array->ElemType());
    }

    // Structure() will create and append the ast::Struct to the
    // global declarations of `ctx.dst`. As we haven't finished building the
    // current module-scope statement or function, this will be placed
    // immediately before the usage.
    ctx.dst->Structure(
        name,
        {ctx.dst->Member("el", el_ty, {ctx.dst->MemberSize(array->Stride())})});

    auto* dst = ctx.dst;
    return [=] {
      if (array->IsRuntimeSized()) {
        return dst->ty.array(dst->create<ast::TypeName>(name));
      } else {
        return dst->ty.array(dst->create<ast::TypeName>(name), array->Count());
      }
    };
  });
}

}  // namespace

PadArrayElements::PadArrayElements() = default;

PadArrayElements::~PadArrayElements() = default;

void PadArrayElements::Run(CloneContext& ctx, const DataMap&, DataMap&) {
  auto& sem = ctx.src->Sem();

  std::unordered_map<const sem::Array*, ArrayBuilder> padded_arrays;
  auto pad = [&](const sem::Array* array) {
    return PadArray(ctx, CreateASTTypeFor, padded_arrays, array);
  };

  // Replace all array types with their corresponding padded array type
  ctx.ReplaceAll([&](const ast::Type* ast_type) -> const ast::Type* {
    auto* type = ctx.src->TypeOf(ast_type);
    if (auto* array = type->UnwrapRef()->As<sem::Array>()) {
      if (auto p = pad(array)) {
        return p();
      }
    }
    return nullptr;
  });

  // Fix up index accessors so `a[1]` becomes `a[1].el`
  ctx.ReplaceAll([&](const ast::IndexAccessorExpression* accessor)
                     -> const ast::Expression* {
    if (auto* array = tint::As<sem::Array>(
            sem.Get(accessor->object)->Type()->UnwrapRef())) {
      if (pad(array)) {
        // Array element is wrapped in a structure. Emit a member accessor
        // to get to the actual array element.
        auto* idx = ctx.CloneWithoutTransform(accessor);
        return ctx.dst->MemberAccessor(idx, "el");
      }
    }
    return nullptr;
  });

  // Fix up array constructors so `A(1,2)` becomes
  // `A(padded(1), padded(2))`
  ctx.ReplaceAll(
      [&](const ast::CallExpression* expr) -> const ast::Expression* {
        auto* call = sem.Get(expr);
        if (auto* ctor = call->Target()->As<sem::TypeConstructor>()) {
          if (auto* array = ctor->ReturnType()->As<sem::Array>()) {
            if (auto p = pad(array)) {
              auto* arr_ty = p();
              auto el_typename = arr_ty->type->As<ast::TypeName>()->name;

              ast::ExpressionList args;
              args.reserve(call->Arguments().size());
              for (auto* arg : call->Arguments()) {
                auto* val = ctx.Clone(arg->Declaration());
                args.emplace_back(ctx.dst->Construct(
                    ctx.dst->create<ast::TypeName>(el_typename), val));
              }

              return ctx.dst->Construct(arr_ty, args);
            }
          }
        }
        return nullptr;
      });

  ctx.Clone();
}

}  // namespace transform
}  // namespace tint
