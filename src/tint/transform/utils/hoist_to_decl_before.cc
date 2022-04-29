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

#include "src/tint/transform/utils/hoist_to_decl_before.h"

#include <unordered_map>

#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/sem/block_statement.h"
#include "src/tint/sem/for_loop_statement.h"
#include "src/tint/sem/if_statement.h"
#include "src/tint/sem/reference.h"
#include "src/tint/sem/variable.h"
#include "src/tint/utils/reverse.h"

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

  /// Info for each else-if that needs decomposing
  struct ElseIfInfo {
    /// Decls to insert before condition
    ast::StatementList cond_decls;
  };

  /// For-loops that need to be decomposed to loops.
  std::unordered_map<const sem::ForLoopStatement*, LoopInfo> loops;

  /// 'else if' statements that need to be decomposed to 'else {if}'
  std::unordered_map<const ast::IfStatement*, ElseIfInfo> else_ifs;

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
    // Decompose 'else-if' statements into 'else { if }' blocks.
    ctx.ReplaceAll(
        [&](const ast::IfStatement* else_if) -> const ast::Statement* {
          if (!else_ifs.count(else_if)) {
            return nullptr;
          }
          auto& else_if_info = else_ifs[else_if];

          // Build the else block's body statements, starting with let decls for
          // the conditional expression.
          auto& body_stmts = else_if_info.cond_decls;

          // Move the 'else-if' into the new `else` block as a plain 'if'.
          auto* cond = ctx.Clone(else_if->condition);
          auto* body = ctx.Clone(else_if->body);
          auto* new_if = b.If(cond, body, ctx.Clone(else_if->else_statement));
          body_stmts.emplace_back(new_if);

          // Replace the 'else-if' with the new 'else' block.
          return b.Block(body_stmts);
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
  bool Add(const sem::Expression* before_expr,
           const ast::Expression* expr,
           bool as_const,
           const char* decl_name) {
    auto name = b.Symbols().New(decl_name);

    // Construct the let/var that holds the hoisted expr
    auto* v = as_const ? b.Let(name, nullptr, ctx.Clone(expr))
                       : b.Var(name, nullptr, ctx.Clone(expr));
    auto* decl = b.Decl(v);

    if (!InsertBefore(before_expr->Stmt(), decl)) {
      return false;
    }

    // Replace the initializer expression with a reference to the let
    ctx.Replace(expr, b.Expr(name));
    return true;
  }

  /// Inserts `stmt` before `before_stmt`, possibly marking a for-loop to be
  /// converted to a loop, or an else-if to an else { if }. If `decl` is
  /// nullptr, for-loop and else-if conversions are marked, but no hoisting
  /// takes place.
  /// @param before_stmt statement to insert `stmt` before
  /// @param stmt statement to insert
  /// @return true on success
  bool InsertBefore(const sem::Statement* before_stmt,
                    const ast::Statement* stmt) {
    auto* ip = before_stmt->Declaration();

    auto* else_if = before_stmt->As<sem::IfStatement>();
    if (else_if && else_if->Parent()->Is<sem::IfStatement>()) {
      // Insertion point is an 'else if' condition.
      // Need to convert 'else if' to 'else { if }'.
      auto& else_if_info = else_ifs[else_if->Declaration()];

      // Index the map to convert this else if, even if `stmt` is nullptr.
      auto& decls = else_if_info.cond_decls;
      if (stmt) {
        decls.emplace_back(stmt);
      }
      return true;
    }

    if (auto* fl = before_stmt->As<sem::ForLoopStatement>()) {
      // Insertion point is a for-loop condition.
      // For-loop needs to be decomposed to a loop.

      // Index the map to convert this for-loop, even if `stmt` is nullptr.
      auto& decls = loops[fl].cond_decls;
      if (stmt) {
        decls.emplace_back(stmt);
      }
      return true;
    }

    auto* parent = before_stmt->Parent();  // The statement's parent
    if (auto* block = parent->As<sem::BlockStatement>()) {
      // Insert point sits in a block. Simple case.
      // Insert the stmt before the parent statement.
      if (stmt) {
        ctx.InsertBefore(block->Declaration()->statements, ip, stmt);
      }
      return true;
    }

    if (auto* fl = parent->As<sem::ForLoopStatement>()) {
      // Insertion point is a for-loop initializer or continuing statement.
      // These require special care.
      if (fl->Declaration()->initializer == ip) {
        // Insertion point is a for-loop initializer.
        // Insert the new statement above the for-loop.
        if (stmt) {
          ctx.InsertBefore(fl->Block()->Declaration()->statements,
                           fl->Declaration(), stmt);
        }
        return true;
      }

      if (fl->Declaration()->continuing == ip) {
        // Insertion point is a for-loop continuing statement.
        // For-loop needs to be decomposed to a loop.

        // Index the map to convert this for-loop, even if `stmt` is nullptr.
        auto& decls = loops[fl].cont_decls;
        if (stmt) {
          decls.emplace_back(stmt);
        }
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

  /// Use to signal that we plan on hoisting a decl before `before_expr`. This
  /// will convert 'for-loop's to 'loop's and 'else-if's to 'else {if}'s if
  /// needed.
  /// @param before_expr expression we would hoist a decl before
  /// @return true on success
  bool Prepare(const sem::Expression* before_expr) {
    return InsertBefore(before_expr->Stmt(), nullptr);
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
  return state_->Add(before_expr, expr, as_const, decl_name);
}

bool HoistToDeclBefore::InsertBefore(const sem::Statement* before_stmt,
                                     const ast::Statement* stmt) {
  return state_->InsertBefore(before_stmt, stmt);
}

bool HoistToDeclBefore::Prepare(const sem::Expression* before_expr) {
  return state_->Prepare(before_expr);
}

bool HoistToDeclBefore::Apply() {
  return state_->Apply();
}

}  // namespace tint::transform
