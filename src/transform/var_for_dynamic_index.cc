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

#include "src/transform/var_for_dynamic_index.h"

#include <utility>

#include "src/program_builder.h"
#include "src/sem/array.h"
#include "src/sem/block_statement.h"
#include "src/sem/expression.h"
#include "src/sem/statement.h"
#include "src/transform/for_loop_to_loop.h"

namespace tint {
namespace transform {

VarForDynamicIndex::VarForDynamicIndex() = default;

VarForDynamicIndex::~VarForDynamicIndex() = default;

void VarForDynamicIndex::Run(CloneContext& ctx, const DataMap&, DataMap&) {
  ProgramBuilder out;
  if (!Requires<ForLoopToLoop>(ctx)) {
    return;
  }

  auto& sem = ctx.src->Sem();

  for (auto* node : ctx.src->ASTNodes().Objects()) {
    if (auto* access_expr = node->As<ast::IndexAccessorExpression>()) {
      // Found an array accessor expression
      auto* index_expr = access_expr->index;
      auto* object_expr = access_expr->object;

      if (sem.Get(index_expr)->ConstantValue()) {
        // Index expression resolves to a compile time value.
        // As this isn't a dynamic index, we can ignore this.
        continue;
      }

      auto* indexed = sem.Get(object_expr);
      if (!indexed->Type()->IsAnyOf<sem::Array, sem::Matrix>()) {
        // This transform currently only cares about array and matrices.
        continue;
      }

      // Construct a `var` declaration to hold the value in memory.
      // TODO(bclayton): group multiple accesses in the same object.
      // e.g. arr[i] + arr[i+1] // Don't create two vars for this
      auto var_name = ctx.dst->Symbols().New("var_for_index");
      auto* var_decl = ctx.dst->Decl(
          ctx.dst->Var(var_name, nullptr, ctx.Clone(object_expr)));

      // Statement that owns the expression
      auto* stmt = indexed->Stmt();
      // Block that owns the statement
      auto* block = stmt->Parent()->As<sem::BlockStatement>();
      if (!block) {
        TINT_ICE(Transform, ctx.dst->Diagnostics())
            << "statement parent was not a block";
        continue;
      }

      // Insert the `var` declaration before the statement that performs the
      // indexing. Note that for indexing chains, AST node ordering guarantees
      // that the inner-most index variable will be placed first in the block.
      ctx.InsertBefore(block->Declaration()->statements, stmt->Declaration(),
                       var_decl);

      // Replace the original index expression with the new `var`.
      ctx.Replace(object_expr, ctx.dst->Expr(var_name));
    }
  }

  ctx.Clone();
}

}  // namespace transform
}  // namespace tint
