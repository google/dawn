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

#include "src/transform/vectorize_scalar_matrix_constructors.h"

#include <utility>

#include "src/program_builder.h"
#include "src/sem/call.h"
#include "src/sem/expression.h"
#include "src/sem/type_constructor.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::VectorizeScalarMatrixConstructors);

namespace tint {
namespace transform {

VectorizeScalarMatrixConstructors::VectorizeScalarMatrixConstructors() =
    default;

VectorizeScalarMatrixConstructors::~VectorizeScalarMatrixConstructors() =
    default;

void VectorizeScalarMatrixConstructors::Run(CloneContext& ctx,
                                            const DataMap&,
                                            DataMap&) {
  ctx.ReplaceAll(
      [&](const ast::CallExpression* expr) -> const ast::CallExpression* {
        auto* call = ctx.src->Sem().Get(expr);
        auto* ty_ctor = call->Target()->As<sem::TypeConstructor>();
        if (!ty_ctor) {
          return nullptr;
        }
        // Check if this is a matrix constructor with scalar arguments.
        auto* mat_type = call->Type()->As<sem::Matrix>();
        if (!mat_type) {
          return nullptr;
        }

        auto& args = call->Arguments();
        if (args.size() == 0) {
          return nullptr;
        }
        if (!args[0]->Type()->is_scalar()) {
          return nullptr;
        }

        // Build a list of vector expressions for each column.
        ast::ExpressionList columns;
        for (uint32_t c = 0; c < mat_type->columns(); c++) {
          // Build a list of scalar expressions for each value in the column.
          ast::ExpressionList row_values;
          for (uint32_t r = 0; r < mat_type->rows(); r++) {
            row_values.push_back(
                ctx.Clone(args[c * mat_type->rows() + r]->Declaration()));
          }

          // Construct the column vector.
          auto* col = ctx.dst->vec(CreateASTTypeFor(ctx, mat_type->type()),
                                   mat_type->rows(), row_values);
          columns.push_back(col);
        }
        return ctx.dst->Construct(CreateASTTypeFor(ctx, mat_type), columns);
      });

  ctx.Clone();
}

}  // namespace transform
}  // namespace tint
