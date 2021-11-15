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

#include "src/transform/promote_initializers_to_const_var.h"

#include <utility>

#include "src/program_builder.h"
#include "src/sem/block_statement.h"
#include "src/sem/call.h"
#include "src/sem/expression.h"
#include "src/sem/statement.h"
#include "src/sem/type_constructor.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::PromoteInitializersToConstVar);

namespace tint {
namespace transform {

PromoteInitializersToConstVar::PromoteInitializersToConstVar() = default;

PromoteInitializersToConstVar::~PromoteInitializersToConstVar() = default;

void PromoteInitializersToConstVar::Run(CloneContext& ctx,
                                        const DataMap&,
                                        DataMap&) {
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
    if (auto* src_init = src_node->As<ast::CallExpression>()) {
      auto* call = ctx.src->Sem().Get(src_init);
      if (!call->Target()->Is<sem::TypeConstructor>()) {
        continue;
      }
      auto* src_sem_stmt = call->Stmt();
      if (!src_sem_stmt) {
        // Expression is outside of a statement. This usually means the
        // expression is part of a global (module-scope) constant declaration.
        // These must be constexpr, and so cannot contain the type of
        // expressions that must be sanitized.
        continue;
      }
      auto* src_stmt = src_sem_stmt->Declaration();

      if (auto* src_var_decl = src_stmt->As<ast::VariableDeclStatement>()) {
        if (src_var_decl->variable->constructor == src_init) {
          // This statement is just a variable declaration with the initializer
          // as the constructor value. This is what we're attempting to
          // transform to, and so ignore.
          continue;
        }
      }

      auto* src_ty = call->Type();
      if (src_ty->IsAnyOf<sem::Array, sem::Struct>()) {
        // Create a new symbol for the constant
        auto dst_symbol = ctx.dst->Sym();
        // Clone the type
        auto* dst_ty = CreateASTTypeFor(ctx, call->Type());
        // Clone the initializer
        auto* dst_init = ctx.Clone(src_init);
        // Construct the constant that holds the hoisted initializer
        auto* dst_var = ctx.dst->Const(dst_symbol, dst_ty, dst_init);
        // Construct the variable declaration statement
        auto* dst_var_decl = ctx.dst->Decl(dst_var);
        // Construct the identifier for referencing the constant
        auto* dst_ident = ctx.dst->Expr(dst_symbol);

        // Insert the constant before the usage
        ctx.InsertBefore(src_sem_stmt->Block()->Declaration()->statements,
                         src_stmt, dst_var_decl);
        // Replace the inlined initializer with a reference to the constant
        ctx.Replace(src_init, dst_ident);
      }
    }
  }

  ctx.Clone();
}

}  // namespace transform
}  // namespace tint
