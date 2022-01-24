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

#include "src/transform/promote_side_effects_to_decl.h"

#include <string>
#include <unordered_map>
#include <utility>

#include "src/program_builder.h"
#include "src/sem/block_statement.h"
#include "src/sem/call.h"
#include "src/sem/expression.h"
#include "src/sem/for_loop_statement.h"
#include "src/sem/if_statement.h"
#include "src/sem/statement.h"
#include "src/sem/type_constructor.h"
#include "src/utils/reverse.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::PromoteSideEffectsToDecl);
TINT_INSTANTIATE_TYPEINFO(tint::transform::PromoteSideEffectsToDecl::Config);

namespace tint {
namespace transform {

/// Private implementation of PromoteSideEffectsToDecl transform
class PromoteSideEffectsToDecl::State {
 private:
  CloneContext& ctx;
  const Config& cfg;
  ProgramBuilder& b;

  /// Holds information about a for-loop that needs to be decomposed into a
  /// loop, so that declaration statements can be inserted before the condition
  /// expression or continuing statement.
  struct LoopInfo {
    ast::StatementList cond_decls;
    ast::StatementList cont_decls;
  };

  /// Holds information about 'if's with 'else-if' statements that need to be
  /// decomposed into 'if {else}' so that declaration statements can be inserted
  /// before the condition expression.
  struct IfInfo {
    /// Info for each else-if that needs decomposing
    struct ElseIfInfo {
      /// Decls to insert before condition
      ast::StatementList cond_decls;
    };

    /// 'else if's that need to be decomposed to 'else { if }'
    std::unordered_map<const sem::ElseStatement*, ElseIfInfo> else_ifs;
  };

  // For-loops that need to be decomposed to loops.
  std::unordered_map<const sem::ForLoopStatement*, LoopInfo> loops;

  /// If statements with 'else if's that need to be decomposed to 'else { if }'
  std::unordered_map<const sem::IfStatement*, IfInfo> ifs;

  // Inserts `decl` before `sem_expr`, possibly marking a for-loop to be
  // converted to a loop, or an else-if to an else { if }..
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

  // Hoists array and structure initializers to a constant variable, declared
  // just before the statement of usage.
  bool TypeConstructorToLet(const ast::CallExpression* expr) {
    auto* ctor = ctx.src->Sem().Get(expr);
    if (!ctor->Target()->Is<sem::TypeConstructor>()) {
      return true;
    }
    auto* sem_stmt = ctor->Stmt();
    if (!sem_stmt) {
      // Expression is outside of a statement. This usually means the
      // expression is part of a global (module-scope) constant declaration.
      // These must be constexpr, and so cannot contain the type of
      // expressions that must be sanitized.
      return true;
    }

    auto* stmt = sem_stmt->Declaration();

    if (auto* src_var_decl = stmt->As<ast::VariableDeclStatement>()) {
      if (src_var_decl->variable->constructor == expr) {
        // This statement is just a variable declaration with the
        // initializer as the constructor value. This is what we're
        // attempting to transform to, and so ignore.
        return true;
      }
    }

    auto* src_ty = ctor->Type();
    if (!src_ty->IsAnyOf<sem::Array, sem::Struct>()) {
      // We only care about array and struct initializers
      return true;
    }

    // Construct the let that holds the hoisted initializer
    auto name = b.Sym();
    auto* let = b.Const(name, nullptr, ctx.Clone(expr));
    auto* let_decl = b.Decl(let);

    if (!InsertBefore(ctor, let_decl)) {
      return false;
    }

    // Replace the initializer expression with a reference to the let
    ctx.Replace(expr, b.Expr(name));
    return true;
  }

  // Extracts array and matrix values that are dynamically indexed to a
  // temporary `var` local that is then indexed.
  bool DynamicIndexToVar(const ast::IndexAccessorExpression* access_expr) {
    auto* index_expr = access_expr->index;
    auto* object_expr = access_expr->object;
    auto& sem = ctx.src->Sem();

    if (sem.Get(index_expr)->ConstantValue()) {
      // Index expression resolves to a compile time value.
      // As this isn't a dynamic index, we can ignore this.
      return true;
    }

    auto* indexed = sem.Get(object_expr);
    if (!indexed->Type()->IsAnyOf<sem::Array, sem::Matrix>()) {
      // We only care about array and matrices.
      return true;
    }

    // Construct a `var` declaration to hold the value in memory.
    // TODO(bclayton): group multiple accesses in the same object.
    // e.g. arr[i] + arr[i+1] // Don't create two vars for this
    auto var_name = b.Symbols().New("var_for_index");
    auto* var_decl = b.Decl(b.Var(var_name, nullptr, ctx.Clone(object_expr)));

    if (!InsertBefore(indexed, var_decl)) {
      return false;
    }

    // Replace the original index expression with the new `var`.
    ctx.Replace(object_expr, b.Expr(var_name));
    return true;
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
              for (auto* body_stmt : for_loop->body->statements) {
                body_stmts.emplace_back(ctx.Clone(body_stmt));
              }

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
            body_stmts.emplace_back(b.If(ctx.Clone(else_stmt->condition),
                                         ctx.Clone(else_stmt->body),
                                         next_else_stmts));

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
      auto* new_if = b.If(ctx.Clone(if_stmt->condition),
                          ctx.Clone(if_stmt->body), next_else_stmts);
      return new_if;
    });
  }

 public:
  /// Constructor
  /// @param ctx_in the CloneContext primed with the input program and
  /// @param cfg_in the transform config
  /// ProgramBuilder
  explicit State(CloneContext& ctx_in, const Config& cfg_in)
      : ctx(ctx_in), cfg(cfg_in), b(*ctx_in.dst) {}

  /// Runs the transform
  void Run() {
    // Scan the AST nodes for expressions that need to be promoted to their own
    // constant or variable declaration.

    // Note: Correct handling of nested expressions is guaranteed due to the
    // depth-first traversal of the ast::Node::Clone() methods:
    //
    // The inner-most expressions are traversed first, and they are hoisted
    // to variables declared just above the statement of use. The outer
    // expression will then be hoisted, inserting themselves between the
    // inner declaration and the statement of use. This pattern applies
    // correctly to any nested depth.
    //
    // Depth-first traversal of the AST is guaranteed because AST nodes are
    // fully immutable and require their children to be constructed first so
    // their pointer can be passed to the parent's constructor.

    for (auto* node : ctx.src->ASTNodes().Objects()) {
      if (cfg.type_ctor_to_let) {
        if (auto* call_expr = node->As<ast::CallExpression>()) {
          if (!TypeConstructorToLet(call_expr)) {
            return;
          }
        }
      }

      if (cfg.dynamic_index_to_var) {
        if (auto* access_expr = node->As<ast::IndexAccessorExpression>()) {
          if (!DynamicIndexToVar(access_expr)) {
            return;
          }
        }
      }
    }

    ForLoopsToLoops();
    ElseIfsToElseWithNestedIfs();

    ctx.Clone();
  }
};

PromoteSideEffectsToDecl::PromoteSideEffectsToDecl() = default;
PromoteSideEffectsToDecl::~PromoteSideEffectsToDecl() = default;

void PromoteSideEffectsToDecl::Run(CloneContext& ctx,
                                   const DataMap& inputs,
                                   DataMap&) {
  auto* cfg = inputs.Get<Config>();
  if (cfg == nullptr) {
    ctx.dst->Diagnostics().add_error(
        diag::System::Transform,
        "missing transform data for " + std::string(TypeInfo().name));
    return;
  }

  State state(ctx, *cfg);
  state.Run();
}

PromoteSideEffectsToDecl::Config::Config(bool type_ctor_to_let_in,
                                         bool dynamic_index_to_var_in)
    : type_ctor_to_let(type_ctor_to_let_in),
      dynamic_index_to_var(dynamic_index_to_var_in) {}

PromoteSideEffectsToDecl::Config::~Config() = default;

}  // namespace transform
}  // namespace tint
