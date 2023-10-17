// Copyright 2022 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/spirv/writer/ast_raise/vectorize_matrix_conversions.h"

#include <tuple>
#include <unordered_map>
#include <utility>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/type/abstract_numeric.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/value_conversion.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"
#include "src/tint/utils/containers/map.h"
#include "src/tint/utils/math/hash.h"

using namespace tint::core::fluent_types;  // NOLINT

TINT_INSTANTIATE_TYPEINFO(tint::spirv::writer::VectorizeMatrixConversions);

namespace tint::spirv::writer {
namespace {

bool ShouldRun(const Program& program) {
    for (auto* node : program.ASTNodes().Objects()) {
        if (auto* sem = program.Sem().GetVal(node)) {
            if (auto* call = sem->UnwrapMaterialize()->As<sem::Call>()) {
                if (call->Target()->Is<sem::ValueConversion>() &&
                    call->Type()->Is<core::type::Matrix>()) {
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

ast::transform::Transform::ApplyResult VectorizeMatrixConversions::Apply(
    const Program& src,
    const ast::transform::DataMap&,
    ast::transform::DataMap&) const {
    if (!ShouldRun(src)) {
        return SkipTransform;
    }

    ProgramBuilder b;
    program::CloneContext ctx{&b, &src, /* auto_clone_symbols */ true};

    using HelperFunctionKey =
        tint::UnorderedKeyWrapper<std::tuple<const core::type::Matrix*, const core::type::Matrix*>>;

    std::unordered_map<HelperFunctionKey, Symbol> matrix_convs;

    ctx.ReplaceAll([&](const ast::CallExpression* expr) -> const ast::CallExpression* {
        auto* call = src.Sem().Get(expr)->UnwrapMaterialize()->As<sem::Call>();
        auto* ty_conv = call->Target()->As<sem::ValueConversion>();
        if (!ty_conv) {
            return nullptr;
        }
        auto* dst_type = call->Type()->As<core::type::Matrix>();
        if (!dst_type) {
            return nullptr;
        }

        auto& args = call->Arguments();
        if (args.Length() != 1) {
            return nullptr;
        }

        auto& matrix = args[0];

        auto* src_type = matrix->Type()->UnwrapRef()->As<core::type::Matrix>();
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
            Vector<const ast::Expression*, 4> columns;
            for (uint32_t c = 0; c < dst_type->columns(); c++) {
                auto* src_matrix_expr = src_expression_builder();
                auto* src_column_expr = b.IndexAccessor(src_matrix_expr, b.Expr(AInt(c)));
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
                       Vector{
                           b.Param("value", CreateASTTypeFor(ctx, src_type)),
                       },
                       CreateASTTypeFor(ctx, dst_type),
                       Vector{
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

}  // namespace tint::spirv::writer
