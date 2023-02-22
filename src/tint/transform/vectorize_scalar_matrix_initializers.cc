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

#include "src/tint/transform/vectorize_scalar_matrix_initializers.h"

#include <unordered_map>
#include <utility>

#include "src/tint/program_builder.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/value_constructor.h"
#include "src/tint/sem/value_expression.h"
#include "src/tint/type/abstract_numeric.h"
#include "src/tint/utils/map.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::VectorizeScalarMatrixInitializers);

namespace tint::transform {
namespace {

bool ShouldRun(const Program* program) {
    for (auto* node : program->ASTNodes().Objects()) {
        if (auto* call = program->Sem().Get<sem::Call>(node)) {
            if (call->Target()->Is<sem::ValueConstructor>() && call->Type()->Is<type::Matrix>()) {
                auto& args = call->Arguments();
                if (!args.IsEmpty() && args[0]->Type()->UnwrapRef()->is_scalar()) {
                    return true;
                }
            }
        }
    }
    return false;
}

}  // namespace

VectorizeScalarMatrixInitializers::VectorizeScalarMatrixInitializers() = default;

VectorizeScalarMatrixInitializers::~VectorizeScalarMatrixInitializers() = default;

Transform::ApplyResult VectorizeScalarMatrixInitializers::Apply(const Program* src,
                                                                const DataMap&,
                                                                DataMap&) const {
    if (!ShouldRun(src)) {
        return SkipTransform;
    }

    ProgramBuilder b;
    CloneContext ctx{&b, src, /* auto_clone_symbols */ true};

    std::unordered_map<const type::Matrix*, Symbol> scalar_inits;

    ctx.ReplaceAll([&](const ast::CallExpression* expr) -> const ast::CallExpression* {
        auto* call = src->Sem().Get(expr)->UnwrapMaterialize()->As<sem::Call>();
        auto* ty_init = call->Target()->As<sem::ValueConstructor>();
        if (!ty_init) {
            return nullptr;
        }
        auto* mat_type = call->Type()->As<type::Matrix>();
        if (!mat_type) {
            return nullptr;
        }

        auto& args = call->Arguments();
        if (args.IsEmpty()) {
            return nullptr;
        }

        // If the argument type is a matrix, then this is an identity / conversion initializer.
        // If the argument type is a vector, then we're already column vectors.
        // If the argument type is abstract, then we're const-expression and there's no need to
        // adjust this, as it'll be constant folded by the backend.
        if (args[0]
                ->Type()
                ->UnwrapRef()
                ->IsAnyOf<type::Matrix, type::Vector, type::AbstractNumeric>()) {
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
                columns.Push(b.vec(CreateASTTypeFor(ctx, mat_type->type()), mat_type->rows(),
                                   std::move(row_values)));
            }
            return b.Call(CreateASTTypeFor(ctx, mat_type), columns);
        };

        if (args.Length() == 1) {
            // Generate a helper function for constructing the matrix.
            // This is done to ensure that the single argument value is only evaluated once, and
            // with the correct expression evaluation order.
            auto fn = utils::GetOrCreate(scalar_inits, mat_type, [&] {
                auto name = b.Symbols().New("build_mat" + std::to_string(mat_type->columns()) +
                                            "x" + std::to_string(mat_type->rows()));
                b.Func(name,
                       utils::Vector{
                           // Single scalar parameter
                           b.Param("value", CreateASTTypeFor(ctx, mat_type->type())),
                       },
                       CreateASTTypeFor(ctx, mat_type),
                       utils::Vector{
                           b.Return(build_mat([&](uint32_t, uint32_t) {  //
                               return b.Expr("value");
                           })),
                       });
                return name;
            });
            return b.Call(fn, ctx.Clone(args[0]->Declaration()));
        }

        if (TINT_LIKELY(args.Length() == mat_type->columns() * mat_type->rows())) {
            return build_mat([&](uint32_t c, uint32_t r) {
                return ctx.Clone(args[c * mat_type->rows() + r]->Declaration());
            });
        }

        TINT_ICE(Transform, b.Diagnostics())
            << "matrix initializer has unexpected number of arguments";
        return nullptr;
    });

    ctx.Clone();
    return Program(std::move(b));
}

}  // namespace tint::transform
