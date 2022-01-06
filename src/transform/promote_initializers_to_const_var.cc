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

#include <unordered_map>
#include <utility>

#include "src/program_builder.h"
#include "src/sem/block_statement.h"
#include "src/sem/call.h"
#include "src/sem/expression.h"
#include "src/sem/for_loop_statement.h"
#include "src/sem/statement.h"
#include "src/sem/type_constructor.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::PromoteInitializersToConstVar);

namespace tint {
namespace transform {

namespace {

/// Holds information about a for-loop that needs to be decomposed into a loop,
/// so that initializer declaration statements can be inserted before the
/// condition expression or continuing statement.
struct LoopInfo {
  ast::StatementList cond_decls;
  ast::StatementList cont_decls;
};

}  // namespace

PromoteInitializersToConstVar::PromoteInitializersToConstVar() = default;

PromoteInitializersToConstVar::~PromoteInitializersToConstVar() = default;

void PromoteInitializersToConstVar::Run(CloneContext& ctx,
                                        const DataMap&,
                                        DataMap&) {
  auto& sem = ctx.src->Sem();
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

  // For-loops that need to be decomposed to loops.
  std::unordered_map<const sem::ForLoopStatement*, LoopInfo> loops;

  for (auto* node : ctx.src->ASTNodes().Objects()) {
    if (auto* expr = node->As<ast::CallExpression>()) {
      auto* ctor = ctx.src->Sem().Get(expr);
      if (!ctor->Target()->Is<sem::TypeConstructor>()) {
        continue;
      }
      auto* sem_stmt = ctor->Stmt();
      if (!sem_stmt) {
        // Expression is outside of a statement. This usually means the
        // expression is part of a global (module-scope) constant declaration.
        // These must be constexpr, and so cannot contain the type of
        // expressions that must be sanitized.
        continue;
      }
      auto* stmt = sem_stmt->Declaration();

      if (auto* src_var_decl = stmt->As<ast::VariableDeclStatement>()) {
        if (src_var_decl->variable->constructor == expr) {
          // This statement is just a variable declaration with the initializer
          // as the constructor value. This is what we're attempting to
          // transform to, and so ignore.
          continue;
        }
      }

      auto* src_ty = ctor->Type();
      if (src_ty->IsAnyOf<sem::Array, sem::Struct>()) {
        // Create a new symbol for the let
        auto name = ctx.dst->Sym();
        // Construct the let that holds the hoisted initializer
        auto* let = ctx.dst->Const(name, nullptr, ctx.Clone(expr));
        // Construct the let declaration statement
        auto* let_decl = ctx.dst->Decl(let);
        // Replace the initializer expression with a reference to the let
        ctx.Replace(expr, ctx.dst->Expr(name));

        if (auto* fl = sem_stmt->As<sem::ForLoopStatement>()) {
          // Expression used in for-loop condition.
          // For-loop needs to be decomposed to a loop.
          loops[fl].cond_decls.emplace_back(let_decl);
          continue;
        }

        auto* parent = sem_stmt->Parent();  // The statement's parent
        if (auto* block = parent->As<sem::BlockStatement>()) {
          // Expression's statement sits in a block. Simple case.
          // Insert the let before the parent statement
          ctx.InsertBefore(block->Declaration()->statements, stmt, let_decl);
          continue;
        }
        if (auto* fl = parent->As<sem::ForLoopStatement>()) {
          // Expression is used in a for-loop. These require special care.
          if (fl->Declaration()->initializer == stmt) {
            // Expression used in for-loop initializer.
            // Insert the let above the for-loop.
            ctx.InsertBefore(fl->Block()->Declaration()->statements,
                             fl->Declaration(), let_decl);
            continue;
          }
          if (fl->Declaration()->continuing == stmt) {
            // Expression used in for-loop continuing.
            // For-loop needs to be decomposed to a loop.
            loops[fl].cont_decls.emplace_back(let_decl);
            continue;
          }
          TINT_ICE(Transform, ctx.dst->Diagnostics())
              << "unhandled use of expression in for-loop";
        }

        TINT_ICE(Transform, ctx.dst->Diagnostics())
            << "unhandled expression parent statement type: "
            << parent->TypeInfo().name;
      }
    }
  }

  if (!loops.empty()) {
    // At least one for-loop needs to be transformed into a loop.
    ctx.ReplaceAll(
        [&](const ast::ForLoopStatement* stmt) -> const ast::Statement* {
          if (auto* fl = sem.Get(stmt)) {
            if (auto it = loops.find(fl); it != loops.end()) {
              auto& info = it->second;
              auto* for_loop = fl->Declaration();
              // For-loop needs to be decomposed to a loop.
              // Build the loop body's statements.
              // Start with any let declarations for the conditional expression.
              auto body_stmts = info.cond_decls;
              // If the for-loop has a condition, emit this next as:
              //   if (!cond) { break; }
              if (auto* cond = for_loop->condition) {
                // !condition
                auto* not_cond = ctx.dst->create<ast::UnaryOpExpression>(
                    ast::UnaryOp::kNot, ctx.Clone(cond));
                // { break; }
                auto* break_body =
                    ctx.dst->Block(ctx.dst->create<ast::BreakStatement>());
                // if (!condition) { break; }
                body_stmts.emplace_back(ctx.dst->If(not_cond, break_body));
              }
              // Next emit the for-loop body
              for (auto* body_stmt : for_loop->body->statements) {
                body_stmts.emplace_back(ctx.Clone(body_stmt));
              }

              // Finally create the continuing block if there was one.
              const ast::BlockStatement* continuing = nullptr;
              if (auto* cont = for_loop->continuing) {
                // Continuing block starts with any let declarations used by the
                // continuing.
                auto cont_stmts = info.cont_decls;
                cont_stmts.emplace_back(ctx.Clone(cont));
                continuing = ctx.dst->Block(cont_stmts);
              }

              auto* body = ctx.dst->Block(body_stmts);
              auto* loop = ctx.dst->Loop(body, continuing);
              if (auto* init = for_loop->initializer) {
                return ctx.dst->Block(ctx.Clone(init), loop);
              }
              return loop;
            }
          }
          return nullptr;
        });
  }

  ctx.Clone();
}

}  // namespace transform
}  // namespace tint
