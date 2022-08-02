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

#include "src/tint/transform/vectorize_scalar_matrix_constructors.h"

#include <unordered_map>
#include <utility>

#include "src/tint/program_builder.h"
#include "src/tint/sem/abstract_numeric.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/expression.h"
#include "src/tint/sem/type_constructor.h"
#include "src/tint/utils/map.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::VectorizeScalarMatrixConstructors);

namespace tint::transform {

VectorizeScalarMatrixConstructors::VectorizeScalarMatrixConstructors() = default;

VectorizeScalarMatrixConstructors::~VectorizeScalarMatrixConstructors() = default;

bool VectorizeScalarMatrixConstructors::ShouldRun(const Program* program, const DataMap&) const {
    for (auto* node : program->ASTNodes().Objects()) {
        if (auto* call = program->Sem().Get<sem::Call>(node)) {
            if (call->Target()->Is<sem::TypeConstructor>() && call->Type()->Is<sem::Matrix>()) {
                auto& args = call->Arguments();
                if (!args.IsEmpty() && args[0]->Type()->UnwrapRef()->is_scalar()) {
                    return true;
                }
            }
        }
    }
    return false;
}

void VectorizeScalarMatrixConstructors::Run(CloneContext& ctx, const DataMap&, DataMap&) const {
    std::unordered_map<const sem::Matrix*, Symbol> scalar_ctors;

    ctx.ReplaceAll([&](const ast::CallExpression* expr) -> const ast::CallExpression* {
        auto* call = ctx.src->Sem().Get(expr)->UnwrapMaterialize()->As<sem::Call>();
        auto* ty_ctor = call->Target()->As<sem::TypeConstructor>();
        if (!ty_ctor) {
            return nullptr;
        }
        auto* mat_type = call->Type()->As<sem::Matrix>();
        if (!mat_type) {
            return nullptr;
        }

        auto& args = call->Arguments();
        if (args.IsEmpty()) {
            return nullptr;
        }

        // If the argument type is a matrix, then this is an identity / conversion constructor.
        // If the argument type is a vector, then we're already column vectors.
        // If the argument type is abstract, then we're const-expression and there's no need to
        // adjust this, as it'll be constant folded by the backend.
        if (args[0]
                ->Type()
                ->UnwrapRef()
                ->IsAnyOf<sem::Matrix, sem::Vector, sem::AbstractNumeric>()) {
            return nullptr;
        }

        // Constructs a matrix using vector columns, with the elements constructed using the
        // 'element(uint32_t c, uint32_t r)' callback.
        auto build_mat = [&](auto&& element) {
            utils::Vector<const ast::Expression*, 4> columns;
            for (uint32_t c = 0; c < mat_type->columns(); c++) {
                utils::Vector<const ast::Expression*, 4> row_values;
                for (uint32_t r = 0; r < mat_type->rows(); r++) {
                    row_values.Push(element(c, r));
                }

                // Construct the column vector.
                columns.Push(ctx.dst->vec(CreateASTTypeFor(ctx, mat_type->type()), mat_type->rows(),
                                          std::move(row_values)));
            }
            return ctx.dst->Construct(CreateASTTypeFor(ctx, mat_type), columns);
        };

        if (args.Length() == 1) {
            // Generate a helper function for constructing the matrix.
            // This is done to ensure that the single argument value is only evaluated once, and
            // with the correct expression evaluation order.
            auto fn = utils::GetOrCreate(scalar_ctors, mat_type, [&] {
                auto name =
                    ctx.dst->Symbols().New("build_mat" + std::to_string(mat_type->columns()) + "x" +
                                           std::to_string(mat_type->rows()));
                ctx.dst->Func(name,
                              utils::Vector{
                                  // Single scalar parameter
                                  ctx.dst->Param("value", CreateASTTypeFor(ctx, mat_type->type())),
                              },
                              CreateASTTypeFor(ctx, mat_type),
                              utils::Vector{
                                  ctx.dst->Return(build_mat([&](uint32_t, uint32_t) {  //
                                      return ctx.dst->Expr("value");
                                  })),
                              });
                return name;
            });
            return ctx.dst->Call(fn, ctx.Clone(args[0]->Declaration()));
        }

        if (args.Length() == mat_type->columns() * mat_type->rows()) {
            return build_mat([&](uint32_t c, uint32_t r) {
                return ctx.Clone(args[c * mat_type->rows() + r]->Declaration());
            });
        }

        TINT_ICE(Transform, ctx.dst->Diagnostics())
            << "matrix constructor has unexpected number of arguments";
        return nullptr;
    });

    ctx.Clone();
}

}  // namespace tint::transform
