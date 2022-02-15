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

#include "src/transform/utils/hoist_to_decl_before.h"

#include <unordered_map>

#include "src/ast/variable_decl_statement.h"
#include "src/sem/block_statement.h"
#include "src/sem/for_loop_statement.h"
#include "src/sem/if_statement.h"
#include "src/sem/reference_type.h"
#include "src/sem/variable.h"
#include "src/utils/reverse.h"

namespace tint::transform {

/// Private implementation of HoistToDeclBefore transform
class HoistToDeclBefore::State {
  CloneContext& ctx;
  ProgramBuilder& b;

  /// Holds information about a for-loop that needs to be decomposed into a
  /// loop, so that declaration statements can be inserted before the
  /// condition expression or continuing statement.
  struct LoopInfo {
    ast::StatementList cond_decls;
    ast::StatementList cont_decls;
  };

  /// Holds information about 'if's with 'else-if' statements that need to be
  /// decomposed into 'if {else}' so that declaration statements can be
  /// inserted before the condition expression.
  struct IfInfo {
    /// Info for each else-if that needs decomposing
    struct ElseIfInfo {
      /// Decls to insert before condition
      ast::StatementList cond_decls;
    };

    /// 'else if's that need to be decomposed to 'else { if }'
    std::unordered_map<const sem::ElseStatement*, ElseIfInfo> else_ifs;
  };

  /// For-loops that need to be decomposed to loops.
  std::unordered_map<const sem::ForLoopStatement*, LoopInfo> loops;

  /// If statements with 'else if's that need to be decomposed to 'else { if
  /// }'
  std::unordered_map<const sem::IfStatement*, IfInfo> ifs;

  // Inserts `decl` before `sem_expr`, possibly marking a for-loop to be
  // converted to a loop, or an else-if to an else { if }.
  bool InsertBefore(const sem::Expression* sem_expr,
                    const ast::VariableDeclStatement* decl) {
    auto* sem_stmt = sem_expr->Stmt();
    auto* stmt = sem_stmt->Declaration();

    if (auto* else_if = sem_stmt->As<sem::ElseStatement>()) {
      // Expression used in 'else if' condition.
      // Need to convert 'else if' to 'else { if }'.
      auto& if_info = ifs[else_if->Parent()->As<sem::IfStatement>()];
      if_info.else_ifs[else_if].cond_decls.push_back(decl);
      return true;
    }

    if (auto* fl = sem_stmt->As<sem::ForLoopStatement>()) {
      // Expression used in for-loop condition.
      // For-loop needs to be decomposed to a loop.
      loops[fl].cond_decls.emplace_back(decl);
      return true;
    }

    auto* parent = sem_stmt->Parent();  // The statement's parent
    if (auto* block = parent->As<sem::BlockStatement>()) {
      // Expression's statement sits in a block. Simple case.
      // Insert the decl before the parent statement
      ctx.InsertBefore(block->Declaration()->statements, stmt, decl);
      return true;
    }

    if (auto* fl = parent->As<sem::ForLoopStatement>()) {
      // Expression is used in a for-loop. These require special care.
      if (fl->Declaration()->initializer == stmt) {
        // Expression used in for-loop initializer.
        // Insert the let above the for-loop.
        ctx.InsertBefore(fl->Block()->Declaration()->statements,
                         fl->Declaration(), decl);
        return true;
      }

      if (fl->Declaration()->continuing == stmt) {
        // Expression used in for-loop continuing.
        // For-loop needs to be decomposed to a loop.
        loops[fl].cont_decls.emplace_back(decl);
        return true;
      }

      TINT_ICE(Transform, b.Diagnostics())
          << "unhandled use of expression in for-loop";
      return false;
    }

    TINT_ICE(Transform, b.Diagnostics())
        << "unhandled expression parent statement type: "
        << parent->TypeInfo().name;
    return false;
  }

  // Converts any for-loops marked for conversion to loops, inserting
  // registered declaration statements before the condition or continuing
  // statement.
  void ForLoopsToLoops() {
    if (loops.empty()) {
      return;
    }

    // At least one for-loop needs to be transformed into a loop.
    ctx.ReplaceAll(
        [&](const ast::ForLoopStatement* stmt) -> const ast::Statement* {
          auto& sem = ctx.src->Sem();

          if (auto* fl = sem.Get(stmt)) {
            if (auto it = loops.find(fl); it != loops.end()) {
              auto& info = it->second;
              auto* for_loop = fl->Declaration();
              // For-loop needs to be decomposed to a loop.
              // Build the loop body's statements.
              // Start with any let declarations for the conditional
              // expression.
              auto body_stmts = info.cond_decls;
              // If the for-loop has a condition, emit this next as:
              //   if (!cond) { break; }
              if (auto* cond = for_loop->condition) {
                // !condition
                auto* not_cond = b.create<ast::UnaryOpExpression>(
                    ast::UnaryOp::kNot, ctx.Clone(cond));
                // { break; }
                auto* break_body = b.Block(b.create<ast::BreakStatement>());
                // if (!condition) { break; }
                body_stmts.emplace_back(b.If(not_cond, break_body));
              }
              // Next emit the for-loop body
              body_stmts.emplace_back(ctx.Clone(for_loop->body));

              // Finally create the continuing block if there was one.
              const ast::BlockStatement* continuing = nullptr;
              if (auto* cont = for_loop->continuing) {
                // Continuing block starts with any let declarations used by
                // the continuing.
                auto cont_stmts = info.cont_decls;
                cont_stmts.emplace_back(ctx.Clone(cont));
                continuing = b.Block(cont_stmts);
              }

              auto* body = b.Block(body_stmts);
              auto* loop = b.Loop(body, continuing);
              if (auto* init = for_loop->initializer) {
                return b.Block(ctx.Clone(init), loop);
              }
              return loop;
            }
          }
          return nullptr;
        });
  }

  void ElseIfsToElseWithNestedIfs() {
    if (ifs.empty()) {
      return;
    }

    ctx.ReplaceAll([&](const ast::IfStatement* if_stmt)  //
                   -> const ast::IfStatement* {
      auto& sem = ctx.src->Sem();
      auto* sem_if = sem.Get(if_stmt);
      if (!sem_if) {
        return nullptr;
      }

      auto it = ifs.find(sem_if);
      if (it == ifs.end()) {
        return nullptr;
      }
      auto& if_info = it->second;

      // This if statement has "else if"s that need to be converted to "else
      // { if }"s

      ast::ElseStatementList next_else_stmts;
      next_else_stmts.reserve(if_stmt->else_statements.size());

      for (auto* else_stmt : utils::Reverse(if_stmt->else_statements)) {
        if (else_stmt->condition == nullptr) {
          // The last 'else', keep as is
          next_else_stmts.insert(next_else_stmts.begin(), ctx.Clone(else_stmt));

        } else {
          auto* sem_else_if = sem.Get(else_stmt);

          auto it2 = if_info.else_ifs.find(sem_else_if);
          if (it2 == if_info.else_ifs.end()) {
            // 'else if' we don't need to modify (no decls to insert), so
            // keep as is
            next_else_stmts.insert(next_else_stmts.begin(),
                                   ctx.Clone(else_stmt));

          } else {
            // 'else if' we need to replace with 'else <decls> { if }'
            auto& else_if_info = it2->second;

            // Build the else body's statements, starting with let decls for
            // the conditional expression
            auto& body_stmts = else_if_info.cond_decls;

            // Build nested if
            auto* cond = ctx.Clone(else_stmt->condition);
            auto* body = ctx.Clone(else_stmt->body);
            body_stmts.emplace_back(b.If(cond, body, next_else_stmts));

            // Build else
            auto* else_with_nested_if = b.Else(b.Block(body_stmts));

            // This will be used in parent if (either another nested if, or
            // top-level if)
            next_else_stmts = {else_with_nested_if};
          }
        }
      }

      // Build a new top-level if with new else statements
      if (next_else_stmts.empty()) {
        TINT_ICE(Transform, b.Diagnostics())
            << "Expected else statements to insert into new if";
      }
      auto* cond = ctx.Clone(if_stmt->condition);
      auto* body = ctx.Clone(if_stmt->body);
      auto* new_if = b.If(cond, body, next_else_stmts);
      return new_if;
    });
  }

 public:
  /// Constructor
  /// @param ctx_in the clone context
  explicit State(CloneContext& ctx_in) : ctx(ctx_in), b(*ctx_in.dst) {}

  /// Hoists `expr` to a `let` or `var` with optional `decl_name`, inserting it
  /// before `before_expr`.
  /// @param before_expr expression to insert `expr` before
  /// @param expr expression to hoist
  /// @param as_const hoist to `let` if true, otherwise to `var`
  /// @param decl_name optional name to use for the variable/constant name
  /// @return true on success
  bool HoistToDeclBefore(const sem::Expression* before_expr,
                         const ast::Expression* expr,
                         bool as_const,
                         const char* decl_name = "") {
    auto name = b.Symbols().New(decl_name);

    auto* sem_expr = ctx.src->Sem().Get(expr);
    bool is_ref =
        sem_expr &&
        !sem_expr->Is<sem::VariableUser>()  // Don't need to take a ref to a var
        && sem_expr->Type()->Is<sem::Reference>();

    auto* expr_clone = ctx.Clone(expr);
    if (is_ref) {
      expr_clone = b.AddressOf(expr_clone);
    }

    // Construct the let/var that holds the hoisted expr
    auto* v = as_const ? b.Const(name, nullptr, expr_clone)
                       : b.Var(name, nullptr, expr_clone);
    auto* decl = b.Decl(v);

    if (!InsertBefore(before_expr, decl)) {
      return false;
    }

    // Replace the initializer expression with a reference to the let
    const ast::Expression* new_expr = b.Expr(name);
    if (is_ref) {
      new_expr = b.Deref(new_expr);
    }
    ctx.Replace(expr, new_expr);
    return true;
  }

  /// Applies any scheduled insertions from previous calls to Add() to
  /// CloneContext. Call this once before ctx.Clone().
  /// @return true on success
  bool Apply() {
    ForLoopsToLoops();
    ElseIfsToElseWithNestedIfs();
    return true;
  }
};

HoistToDeclBefore::HoistToDeclBefore(CloneContext& ctx)
    : state_(std::make_unique<State>(ctx)) {}

HoistToDeclBefore::~HoistToDeclBefore() {}

bool HoistToDeclBefore::Add(const sem::Expression* before_expr,
                            const ast::Expression* expr,
                            bool as_const,
                            const char* decl_name) {
  return state_->HoistToDeclBefore(before_expr, expr, as_const, decl_name);
}

bool HoistToDeclBefore::Apply() {
  return state_->Apply();
}

}  // namespace tint::transform
