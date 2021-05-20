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

namespace tint {
namespace transform {

VarForDynamicIndex::VarForDynamicIndex() = default;

VarForDynamicIndex::~VarForDynamicIndex() = default;

Output VarForDynamicIndex::Run(const Program* in, const DataMap&) {
  ProgramBuilder out;
  CloneContext ctx(&out, in);

  for (auto* node : in->ASTNodes().Objects()) {
    if (auto* access_expr = node->As<ast::ArrayAccessorExpression>()) {
      // Found an array accessor expression
      auto* index_expr = access_expr->idx_expr();
      auto* indexed_expr = access_expr->array();

      if (index_expr->Is<ast::ScalarConstructorExpression>()) {
        // Index expression is a literal value. As this isn't a dynamic index,
        // we can ignore this.
        continue;
      }

      auto* indexed = ctx.src->Sem().Get(indexed_expr);
      if (!indexed->Type()->IsAnyOf<sem::Array, sem::Matrix>()) {
        // This transform currently only cares about array and matrices.
        continue;
      }

      auto* stmt = indexed->Stmt();  // Statement that owns the expression
      auto* block = stmt->Block();   // Block that owns the statement

      // Construct a `var` declaration to hold the value in memory.
      auto* ty = CreateASTTypeFor(&ctx, indexed->Type());
      auto var_name = ctx.dst->Symbols().New("var_for_index");
      auto* var_decl = ctx.dst->Decl(ctx.dst->Var(
          var_name, ty, ast::StorageClass::kNone, ctx.Clone(indexed_expr)));

      // Insert the `var` declaration before the statement that performs the
      // indexing. Note that for indexing chains, AST node ordering guarantees
      // that the inner-most index variable will be placed first in the block.
      ctx.InsertBefore(block->Declaration()->statements(), stmt->Declaration(),
                       var_decl);

      // Replace the original index expression with the new `var`.
      ctx.Replace(indexed_expr, ctx.dst->Expr(var_name));
    }
  }

  ctx.Clone();

  return Output(Program(std::move(out)));
}

}  // namespace transform
}  // namespace tint
