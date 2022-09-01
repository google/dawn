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

#include "src/tint/transform/vectorize_matrix_conversions.h"

#include <tuple>
#include <unordered_map>
#include <utility>

#include "src/tint/program_builder.h"
#include "src/tint/sem/abstract_numeric.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/expression.h"
#include "src/tint/sem/type_conversion.h"
#include "src/tint/utils/hash.h"
#include "src/tint/utils/map.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::VectorizeMatrixConversions);

namespace tint::transform {

VectorizeMatrixConversions::VectorizeMatrixConversions() = default;

VectorizeMatrixConversions::~VectorizeMatrixConversions() = default;

bool VectorizeMatrixConversions::ShouldRun(const Program* program, const DataMap&) const {
    for (auto* node : program->ASTNodes().Objects()) {
        if (auto* sem = program->Sem().Get<sem::Expression>(node)) {
            if (auto* call = sem->UnwrapMaterialize()->As<sem::Call>()) {
                if (call->Target()->Is<sem::TypeConversion>() && call->Type()->Is<sem::Matrix>()) {
                    auto& args = call->Arguments();
                    if (args.Length() == 1 && args[0]->Type()->UnwrapRef()->is_float_matrix()) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void VectorizeMatrixConversions::Run(CloneContext& ctx, const DataMap&, DataMap&) const {
    using HelperFunctionKey =
        utils::UnorderedKeyWrapper<std::tuple<const sem::Matrix*, const sem::Matrix*>>;

    std::unordered_map<HelperFunctionKey, Symbol> matrix_convs;

    ctx.ReplaceAll([&](const ast::CallExpression* expr) -> const ast::CallExpression* {
        auto* call = ctx.src->Sem().Get(expr)->UnwrapMaterialize()->As<sem::Call>();
        auto* ty_conv = call->Target()->As<sem::TypeConversion>();
        if (!ty_conv) {
            return nullptr;
        }
        auto* dst_type = call->Type()->As<sem::Matrix>();
        if (!dst_type) {
            return nullptr;
        }

        auto& args = call->Arguments();
        if (args.Length() != 1) {
            return nullptr;
        }

        auto& src = args[0];

        auto* src_type = args[0]->Type()->UnwrapRef()->As<sem::Matrix>();
        if (!src_type) {
            return nullptr;
        }

        // The source and destination type of a matrix conversion must have a same shape.
        if (!(src_type->rows() == dst_type->rows() && src_type->columns() == dst_type->columns())) {
            TINT_ICE(Transform, ctx.dst->Diagnostics())
                << "source and destination matrix has different shape in matrix conversion";
            return nullptr;
        }

        auto build_vectorized_conversion_expression = [&](auto&& src_expression_builder) {
            utils::Vector<const ast::Expression*, 4> columns;
            for (uint32_t c = 0; c < dst_type->columns(); c++) {
                auto* src_matrix_expr = src_expression_builder();
                auto* src_column_expr =
                    ctx.dst->IndexAccessor(src_matrix_expr, ctx.dst->Expr(tint::AInt(c)));
                columns.Push(ctx.dst->Construct(CreateASTTypeFor(ctx, dst_type->ColumnType()),
                                                src_column_expr));
            }
            return ctx.dst->Construct(CreateASTTypeFor(ctx, dst_type), columns);
        };

        // Replace the matrix conversion to column vector conversions and a matrix construction.
        if (!src->HasSideEffects()) {
            // Simply use the argument's declaration if it has no side effects.
            return build_vectorized_conversion_expression([&]() {  //
                return ctx.Clone(src->Declaration());
            });
        } else {
            // If has side effects, use a helper function.
            auto fn =
                utils::GetOrCreate(matrix_convs, HelperFunctionKey{{src_type, dst_type}}, [&] {
                    auto name =
                        ctx.dst->Symbols().New("convert_mat" + std::to_string(src_type->columns()) +
                                               "x" + std::to_string(src_type->rows()) + "_" +
                                               ctx.dst->FriendlyName(src_type->type()) + "_" +
                                               ctx.dst->FriendlyName(dst_type->type()));
                    ctx.dst->Func(
                        name,
                        utils::Vector{
                            ctx.dst->Param("value", CreateASTTypeFor(ctx, src_type)),
                        },
                        CreateASTTypeFor(ctx, dst_type),
                        utils::Vector{
                            ctx.dst->Return(build_vectorized_conversion_expression([&]() {  //
                                return ctx.dst->Expr("value");
                            })),
                        });
                    return name;
                });
            return ctx.dst->Call(fn, ctx.Clone(args[0]->Declaration()));
        }
    });

    ctx.Clone();
}

}  // namespace tint::transform
