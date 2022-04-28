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

#include "src/tint/transform/simplify_pointers.h"

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/tint/program_builder.h"
#include "src/tint/sem/block_statement.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/variable.h"
#include "src/tint/transform/unshadow.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::SimplifyPointers);

namespace tint::transform {

namespace {

/// PointerOp describes either possible indirection or address-of action on an
/// expression.
struct PointerOp {
  /// Positive: Number of times the `expr` was dereferenced (*expr)
  /// Negative: Number of times the `expr` was 'addressed-of' (&expr)
  /// Zero: no pointer op on `expr`
  int indirections = 0;
  /// The expression being operated on
  const ast::Expression* expr = nullptr;
};

}  // namespace

/// The PIMPL state for the SimplifyPointers transform
struct SimplifyPointers::State {
  /// The clone context
  CloneContext& ctx;

  /// Constructor
  /// @param context the clone context
  explicit State(CloneContext& context) : ctx(context) {}

  /// Traverses the expression `expr` looking for non-literal array indexing
  /// expressions that would affect the computed address of a pointer
  /// expression. The function-like argument `cb` is called for each found.
  /// @param expr the expression to traverse
  /// @param cb a function-like object with the signature
  /// `void(const ast::Expression*)`, which is called for each array index
  /// expression
  template <typename F>
  static void CollectSavedArrayIndices(const ast::Expression* expr, F&& cb) {
    if (auto* a = expr->As<ast::IndexAccessorExpression>()) {
      CollectSavedArrayIndices(a->object, cb);
      if (!a->index->Is<ast::LiteralExpression>()) {
        cb(a->index);
      }
      return;
    }

    if (auto* m = expr->As<ast::MemberAccessorExpression>()) {
      CollectSavedArrayIndices(m->structure, cb);
      return;
    }

    if (auto* u = expr->As<ast::UnaryOpExpression>()) {
      CollectSavedArrayIndices(u->expr, cb);
      return;
    }

    // Note: Other ast::Expression types can be safely ignored as they cannot be
    // used to generate a reference or pointer.
    // See https://gpuweb.github.io/gpuweb/wgsl/#forming-references-and-pointers
  }

  /// Reduce walks the expression chain, collapsing all address-of and
  /// indirection ops into a PointerOp.
  /// @param in the expression to walk
  /// @returns the reduced PointerOp
  PointerOp Reduce(const ast::Expression* in) const {
    PointerOp op{0, in};
    while (true) {
      if (auto* unary = op.expr->As<ast::UnaryOpExpression>()) {
        switch (unary->op) {
          case ast::UnaryOp::kIndirection:
            op.indirections++;
            op.expr = unary->expr;
            continue;
          case ast::UnaryOp::kAddressOf:
            op.indirections--;
            op.expr = unary->expr;
            continue;
          default:
            break;
        }
      }
      if (auto* user = ctx.src->Sem().Get<sem::VariableUser>(op.expr)) {
        auto* var = user->Variable();
        if (var->Is<sem::LocalVariable>() &&  //
            var->Declaration()->is_const &&   //
            var->Type()->Is<sem::Pointer>()) {
          op.expr = var->Declaration()->constructor;
          continue;
        }
      }
      return op;
    }
  }

  /// Performs the transformation
  void Run() {
    // A map of saved expressions to their saved variable name
    std::unordered_map<const ast::Expression*, Symbol> saved_vars;

    // Register the ast::Expression transform handler.
    // This performs two different transformations:
    // * Identifiers that resolve to the pointer-typed `let` declarations are
    // replaced with the recursively inlined initializer expression for the
    // `let` declaration.
    // * Sub-expressions inside the pointer-typed `let` initializer expression
    // that have been hoisted to a saved variable are replaced with the saved
    // variable identifier.
    ctx.ReplaceAll([&](const ast::Expression* expr) -> const ast::Expression* {
      // Look to see if we need to swap this Expression with a saved variable.
      auto it = saved_vars.find(expr);
      if (it != saved_vars.end()) {
        return ctx.dst->Expr(it->second);
      }

      // Reduce the expression, folding away chains of address-of / indirections
      auto op = Reduce(expr);

      // Clone the reduced root expression
      expr = ctx.CloneWithoutTransform(op.expr);

      // And reapply the minimum number of address-of / indirections
      for (int i = 0; i < op.indirections; i++) {
        expr = ctx.dst->Deref(expr);
      }
      for (int i = 0; i > op.indirections; i--) {
        expr = ctx.dst->AddressOf(expr);
      }
      return expr;
    });

    // Find all the pointer-typed `let` declarations.
    // Note that these must be function-scoped, as module-scoped `let`s are not
    // permitted.
    for (auto* node : ctx.src->ASTNodes().Objects()) {
      if (auto* let = node->As<ast::VariableDeclStatement>()) {
        if (!let->variable->is_const) {
          continue;  // Not a `let` declaration. Ignore.
        }

        auto* var = ctx.src->Sem().Get(let->variable);
        if (!var->Type()->Is<sem::Pointer>()) {
          continue;  // Not a pointer type. Ignore.
        }

        // We're dealing with a pointer-typed `let` declaration.

        // Scan the initializer expression for array index expressions that need
        // to be hoist to temporary "saved" variables.
        std::vector<const ast::VariableDeclStatement*> saved;
        CollectSavedArrayIndices(
            var->Declaration()->constructor,
            [&](const ast::Expression* idx_expr) {
              // We have a sub-expression that needs to be saved.
              // Create a new variable
              auto saved_name = ctx.dst->Symbols().New(
                  ctx.src->Symbols().NameFor(var->Declaration()->symbol) +
                  "_save");
              auto* decl = ctx.dst->Decl(
                  ctx.dst->Let(saved_name, nullptr, ctx.Clone(idx_expr)));
              saved.emplace_back(decl);
              // Record the substitution of `idx_expr` to the saved variable
              // with the symbol `saved_name`. This will be used by the
              // ReplaceAll() handler above.
              saved_vars.emplace(idx_expr, saved_name);
            });

        // Find the place to insert the saved declarations.
        // Special care needs to be made for lets declared as the initializer
        // part of for-loops. In this case the block will hold the for-loop
        // statement, not the let.
        if (!saved.empty()) {
          auto* stmt = ctx.src->Sem().Get(let);
          auto* block = stmt->Block();
          // Find the statement owned by the block (either the let decl or a
          // for-loop)
          while (block != stmt->Parent()) {
            stmt = stmt->Parent();
          }
          // Declare the stored variables just before stmt. Order here is
          // important as order-of-operations needs to be preserved.
          // CollectSavedArrayIndices() visits the LHS of an index accessor
          // before the index expression.
          for (auto* decl : saved) {
            // Note that repeated calls to InsertBefore() with the same `before`
            // argument will result in nodes to inserted in the order the
            // calls are made (last call is inserted last).
            ctx.InsertBefore(block->Declaration()->statements,
                             stmt->Declaration(), decl);
          }
        }

        // As the original `let` declaration will be fully inlined, there's no
        // need for the original declaration to exist. Remove it.
        RemoveStatement(ctx, let);
      }
    }
    ctx.Clone();
  }
};

SimplifyPointers::SimplifyPointers() = default;

SimplifyPointers::~SimplifyPointers() = default;

void SimplifyPointers::Run(CloneContext& ctx, const DataMap&, DataMap&) const {
  State(ctx).Run();
}

}  // namespace tint::transform
