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

#include "src/transform/hlsl.h"

#include <utility>

#include "src/ast/stage_decoration.h"
#include "src/ast/variable_decl_statement.h"
#include "src/program_builder.h"
#include "src/sem/block_statement.h"
#include "src/sem/expression.h"
#include "src/sem/statement.h"
#include "src/sem/variable.h"
#include "src/transform/calculate_array_length.h"
#include "src/transform/canonicalize_entry_point_io.h"
#include "src/transform/decompose_storage_access.h"
#include "src/transform/external_texture_transform.h"
#include "src/transform/inline_pointer_lets.h"
#include "src/transform/manager.h"
#include "src/transform/simplify.h"

namespace tint {
namespace transform {

Hlsl::Hlsl() = default;
Hlsl::~Hlsl() = default;

Output Hlsl::Run(const Program* in, const DataMap& data) {
  Manager manager;
  manager.Add<CanonicalizeEntryPointIO>();
  manager.Add<InlinePointerLets>();
  // Simplify cleans up messy `*(&(expr))` expressions from InlinePointerLets.
  manager.Add<Simplify>();
  // DecomposeStorageAccess must come after InlinePointerLets as we cannot take
  // the address of calls to DecomposeStorageAccess::Intrinsic. Must also come
  // after Simplify, as we need to fold away the address-of and defers of
  // `*(&(intrinsic_load()))` expressions.
  manager.Add<DecomposeStorageAccess>();
  manager.Add<CalculateArrayLength>();
  manager.Add<ExternalTextureTransform>();
  auto out = manager.Run(in, data);
  if (!out.program.IsValid()) {
    return out;
  }

  ProgramBuilder builder;
  CloneContext ctx(&builder, &out.program);
  PromoteInitializersToConstVar(ctx);
  AddEmptyEntryPoint(ctx);
  ctx.Clone();
  return Output{Program(std::move(builder))};
}

void Hlsl::PromoteInitializersToConstVar(CloneContext& ctx) const {
  // Scan the AST nodes for array and structure initializers which
  // need to be promoted to their own constant declaration.

  // Note: Correct handling of nested expressions is guaranteed due to the
  // depth-first traversal of the ast::Node::Clone() methods:
  //
  // The inner-most initializers are traversed first, and they are hoisted
  // to const variables declared just above the statement of use. The outer
  // initializer will then be hoisted, inserting themselves between the
  // inner declaration and the statement of use. This pattern applies correctly
  // to any nested depth.
  //
  // Depth-first traversal of the AST is guaranteed because AST nodes are fully
  // immutable and require their children to be constructed first so their
  // pointer can be passed to the parent's constructor.

  for (auto* src_node : ctx.src->ASTNodes().Objects()) {
    if (auto* src_init = src_node->As<ast::TypeConstructorExpression>()) {
      auto* src_sem_expr = ctx.src->Sem().Get(src_init);
      if (!src_sem_expr) {
        TINT_ICE(ctx.dst->Diagnostics())
            << "ast::TypeConstructorExpression has no semantic expression node";
        continue;
      }
      auto* src_sem_stmt = src_sem_expr->Stmt();
      if (!src_sem_stmt) {
        // Expression is outside of a statement. This usually means the
        // expression is part of a global (module-scope) constant declaration.
        // These must be constexpr, and so cannot contain the type of
        // expressions that must be sanitized.
        continue;
      }
      auto* src_stmt = src_sem_stmt->Declaration();

      if (auto* src_var_decl = src_stmt->As<ast::VariableDeclStatement>()) {
        if (src_var_decl->variable()->constructor() == src_init) {
          // This statement is just a variable declaration with the initializer
          // as the constructor value. This is what we're attempting to
          // transform to, and so ignore.
          continue;
        }
      }

      auto* src_ty = src_sem_expr->Type();
      if (src_ty->IsAnyOf<sem::Array, sem::Struct>()) {
        // Create a new symbol for the constant
        auto dst_symbol = ctx.dst->Sym();
        // Clone the type
        auto* dst_ty = ctx.Clone(src_init->type());
        // Clone the initializer
        auto* dst_init = ctx.Clone(src_init);
        // Construct the constant that holds the hoisted initializer
        auto* dst_var = ctx.dst->Const(dst_symbol, dst_ty, dst_init);
        // Construct the variable declaration statement
        auto* dst_var_decl = ctx.dst->Decl(dst_var);
        // Construct the identifier for referencing the constant
        auto* dst_ident = ctx.dst->Expr(dst_symbol);

        // Insert the constant before the usage
        ctx.InsertBefore(src_sem_stmt->Block()->Declaration()->statements(),
                         src_stmt, dst_var_decl);
        // Replace the inlined initializer with a reference to the constant
        ctx.Replace(src_init, dst_ident);
      }
    }
  }
}

void Hlsl::AddEmptyEntryPoint(CloneContext& ctx) const {
  for (auto* func : ctx.src->AST().Functions()) {
    if (func->IsEntryPoint()) {
      return;
    }
  }
  ctx.dst->Func(ctx.dst->Symbols().New("unused_entry_point"), {},
                ctx.dst->ty.void_(), {},
                {ctx.dst->Stage(ast::PipelineStage::kCompute)});
}

}  // namespace transform
}  // namespace tint
