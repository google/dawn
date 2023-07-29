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

#include "src/tint/lang/wgsl/ast/transform/vectorize_matrix_conversions.h"

#include <tuple>
#include <unordered_map>
#include <utility>

#include "src/tint/lang/core/type/abstract_numeric.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/value_conversion.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"
#include "src/tint/utils/containers/map.h"
#include "src/tint/utils/math/hash.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::VectorizeMatrixConversions);

namespace tint::ast::transform {

namespace {

bool ShouldRun(const Program* program) {
    for (auto* node : program->ASTNodes().Objects()) {
        if (auto* sem = program->Sem().GetVal(node)) {
            if (auto* call = sem->UnwrapMaterialize()->As<sem::Call>()) {
                if (call->Target()->Is<sem::ValueConversion>() &&
                    call->Type()->Is<type::Matrix>()) {
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

}  // namespace

VectorizeMatrixConversions::VectorizeMatrixConversions() = default;

VectorizeMatrixConversions::~VectorizeMatrixConversions() = default;

Transform::ApplyResult VectorizeMatrixConversions::Apply(const Program* src,
                                                         const DataMap&,
                                                         DataMap&) const {
    if (!ShouldRun(src)) {
        return SkipTransform;
    }

    ProgramBuilder b;
    program::CloneContext ctx{&b, src, /* auto_clone_symbols */ true};

    using HelperFunctionKey =
        tint::UnorderedKeyWrapper<std::tuple<const type::Matrix*, const type::Matrix*>>;

    std::unordered_map<HelperFunctionKey, Symbol> matrix_convs;

    ctx.ReplaceAll([&](const CallExpression* expr) -> const CallExpression* {
        auto* call = src->Sem().Get(expr)->UnwrapMaterialize()->As<sem::Call>();
        auto* ty_conv = call->Target()->As<sem::ValueConversion>();
        if (!ty_conv) {
            return nullptr;
        }
        auto* dst_type = call->Type()->As<type::Matrix>();
        if (!dst_type) {
            return nullptr;
        }

        auto& args = call->Arguments();
        if (args.Length() != 1) {
            return nullptr;
        }

        auto& matrix = args[0];

        auto* src_type = matrix->Type()->UnwrapRef()->As<type::Matrix>();
        if (!src_type) {
            return nullptr;
        }

        // The source and destination type of a matrix conversion must have a same shape.
        if (TINT_UNLIKELY(!(src_type->rows() == dst_type->rows() &&
                            src_type->columns() == dst_type->columns()))) {
            TINT_ICE() << "source and destination matrix has different shape in matrix conversion";
            return nullptr;
        }

        auto build_vectorized_conversion_expression = [&](auto&& src_expression_builder) {
            tint::Vector<const Expression*, 4> columns;
            for (uint32_t c = 0; c < dst_type->columns(); c++) {
                auto* src_matrix_expr = src_expression_builder();
                auto* src_column_expr = b.IndexAccessor(src_matrix_expr, b.Expr(tint::AInt(c)));
                columns.Push(
                    b.Call(CreateASTTypeFor(ctx, dst_type->ColumnType()), src_column_expr));
            }
            return b.Call(CreateASTTypeFor(ctx, dst_type), columns);
        };

        // Replace the matrix conversion to column vector conversions and a matrix construction.
        if (!matrix->HasSideEffects()) {
            // Simply use the argument's declaration if it has no side effects.
            return build_vectorized_conversion_expression([&] {  //
                return ctx.Clone(matrix->Declaration());
            });
        } else {
            // If has side effects, use a helper function.
            auto fn = tint::GetOrCreate(matrix_convs, HelperFunctionKey{{src_type, dst_type}}, [&] {
                auto name = b.Symbols().New("convert_mat" + std::to_string(src_type->columns()) +
                                            "x" + std::to_string(src_type->rows()) + "_" +
                                            src_type->type()->FriendlyName() + "_" +
                                            dst_type->type()->FriendlyName());
                b.Func(name,
                       tint::Vector{
                           b.Param("value", CreateASTTypeFor(ctx, src_type)),
                       },
                       CreateASTTypeFor(ctx, dst_type),
                       tint::Vector{
                           b.Return(build_vectorized_conversion_expression([&] {  //
                               return b.Expr("value");
                           })),
                       });
                return name;
            });
            return b.Call(fn, ctx.Clone(args[0]->Declaration()));
        }
    });

    ctx.Clone();
    return resolver::Resolve(b);
}

}  // namespace tint::ast::transform
