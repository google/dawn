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

#include "src/tint/transform/fold_constants.h"

#include <unordered_map>
#include <utility>
#include <vector>

#include "src/tint/program_builder.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/expression.h"
#include "src/tint/sem/type_constructor.h"
#include "src/tint/sem/type_conversion.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::FoldConstants);

namespace tint::transform {

FoldConstants::FoldConstants() = default;

FoldConstants::~FoldConstants() = default;

void FoldConstants::Run(CloneContext& ctx, const DataMap&, DataMap&) const {
    ctx.ReplaceAll([&](const ast::Expression* expr) -> const ast::Expression* {
        auto* call = ctx.src->Sem().Get<sem::Call>(expr);
        if (!call) {
            return nullptr;
        }

        auto value = call->ConstantValue();
        if (!value.IsValid()) {
            return nullptr;
        }

        auto* ty = call->Type();

        if (!call->Target()->IsAnyOf<sem::TypeConversion, sem::TypeConstructor>()) {
            return nullptr;
        }

        // If original ctor expression had no init values, don't replace the
        // expression
        if (call->Arguments().empty()) {
            return nullptr;
        }

        if (auto* vec = ty->As<sem::Vector>()) {
            uint32_t vec_size = static_cast<uint32_t>(vec->Width());

            // We'd like to construct the new vector with the same number of
            // constructor args that the original node had, but after folding
            // constants, cases like the following are problematic:
            //
            // vec3<f32> = vec3<f32>(vec2<f32>, 1.0) // vec_size=3, ctor_size=2
            //
            // In this case, creating a vec3 with 2 args is invalid, so we should
            // create it with 3. So what we do is construct with vec_size args,
            // except if the original vector was single-value initialized, in
            // which case, we only construct with one arg again.
            uint32_t ctor_size = (call->Arguments().size() == 1) ? 1 : vec_size;

            ast::ExpressionList ctors;
            for (uint32_t i = 0; i < ctor_size; ++i) {
                value.WithScalarAt(i, [&](auto&& s) { ctors.emplace_back(ctx.dst->Expr(s)); });
            }

            auto* el_ty = CreateASTTypeFor(ctx, vec->type());
            return ctx.dst->vec(el_ty, vec_size, ctors);
        }

        if (ty->is_scalar()) {
            return value.WithScalarAt(
                0, [&](auto&& s) -> const ast::LiteralExpression* { return ctx.dst->Expr(s); });
        }

        return nullptr;
    });

    ctx.Clone();
}

}  // namespace tint::transform
