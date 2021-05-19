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

#include "src/transform/inline_pointer_lets.h"

#include <memory>
#include <unordered_map>
#include <utility>

#include "src/program_builder.h"
#include "src/sem/block_statement.h"
#include "src/sem/function.h"
#include "src/sem/statement.h"
#include "src/sem/variable.h"
#include "src/utils/scoped_assignment.h"

namespace tint {
namespace transform {
namespace {

/// Traverses the expression `expr` looking for non-literal array indexing
/// expressions that would affect the computed address of a pointer expression.
/// The function-like argument `cb` is called for each found.
/// @param program the program that owns all the expression nodes
/// @param expr the expression to traverse
/// @param cb a function-like object with the signature
/// `void(const ast::Expression*)`, which is called for each array index
/// expression
template <typename F>
void CollectSavedArrayIndices(const Program* program,
                              ast::Expression* expr,
                              F&& cb) {
  if (auto* a = expr->As<ast::ArrayAccessorExpression>()) {
    CollectSavedArrayIndices(program, a->array(), cb);

    if (!a->idx_expr()->Is<ast::ScalarConstructorExpression>()) {
      cb(a->idx_expr());
    }
    return;
  }

  if (auto* m = expr->As<ast::MemberAccessorExpression>()) {
    CollectSavedArrayIndices(program, m->structure(), cb);
    return;
  }

  if (auto* u = expr->As<ast::UnaryOpExpression>()) {
    CollectSavedArrayIndices(program, u->expr(), cb);
    return;
  }

  // Note: Other ast::Expression types can be safely ignored as they cannot be
  // used to generate a reference or pointer.
  // See https://gpuweb.github.io/gpuweb/wgsl/#forming-references-and-pointers
}

// PtrLet represents a `let` declaration of a pointer type.
struct PtrLet {
  // A map of ptr-let initializer sub-expression to the name of generated
  // variable that holds the saved value of this sub-expression, when resolved
  // at the point of the ptr-let declaration.
  std::unordered_map<const ast::Expression*, Symbol> saved_vars;
};

}  // namespace

InlinePointerLets::InlinePointerLets() = default;

InlinePointerLets::~InlinePointerLets() = default;

Output InlinePointerLets::Run(const Program* in, const DataMap&) {
  ProgramBuilder out;
  CloneContext ctx(&out, in);

  // If not null, current_ptr_let is the current PtrLet being operated on.
  PtrLet* current_ptr_let = nullptr;
  // A map of the AST `let` variable to the PtrLet
  std::unordered_map<const ast::Variable*, std::unique_ptr<PtrLet>> ptr_lets;

  // Register the ast::Expression transform handler.
  // This performs two different transformations:
  // * Identifiers that resolve to the pointer-typed `let` declarations are
  // replaced with the inlined (and recursively transformed) initializer
  // expression for the `let` declaration.
  // * Sub-expressions inside the pointer-typed `let` initializer expression
  // that have been hoisted to a saved variable are replaced with the saved
  // variable identifier.
  ctx.ReplaceAll([&](ast::Expression* expr) -> ast::Expression* {
    if (current_ptr_let) {
      // We're currently processing the initializer expression of a
      // pointer-typed `let` declaration. Look to see if we need to swap this
      // Expression with a saved variable.
      auto it = current_ptr_let->saved_vars.find(expr);
      if (it != current_ptr_let->saved_vars.end()) {
        return ctx.dst->Expr(it->second);
      }
    }
    if (auto* ident = expr->As<ast::IdentifierExpression>()) {
      if (auto* vu = in->Sem().Get<sem::VariableUser>(ident)) {
        auto* var = vu->Variable()->Declaration();
        auto it = ptr_lets.find(var);
        if (it != ptr_lets.end()) {
          // We've found an identifier that resolves to a `let` declaration.
          // We need to replace this identifier with the initializer expression
          // of the `let` declaration. Clone the initializer expression to make
          // a copy. Note that this will call back into this ReplaceAll()
          // handler for sub-expressions of the initializer.
          auto* ptr_let = it->second.get();
          // TINT_SCOPED_ASSIGNMENT provides a stack of PtrLet*, this is
          // required to handle the 'chaining' of inlined `let`s.
          TINT_SCOPED_ASSIGNMENT(current_ptr_let, ptr_let);
          return ctx.Clone(var->constructor());
        }
      }
    }
    return nullptr;
  });

  // Find all the pointer-typed `let` declarations.
  // Note that these must be function-scoped, as module-scoped `let`s are not
  // permitted.
  for (auto* node : in->ASTNodes().Objects()) {
    if (auto* let = node->As<ast::VariableDeclStatement>()) {
      if (!let->variable()->is_const()) {
        continue;  // Not a `let` declaration. Ignore.
      }

      auto* var = in->Sem().Get(let->variable());
      if (!var->Type()->Is<sem::Pointer>()) {
        continue;  // Not a pointer type. Ignore.
      }

      // We're dealing with a pointer-typed `let` declaration.
      auto ptr_let = std::make_unique<PtrLet>();
      TINT_SCOPED_ASSIGNMENT(current_ptr_let, ptr_let.get());

      auto* block = ctx.src->Sem().Get(let)->Block()->Declaration();

      // Scan the initializer expression for array index expressions that need
      // to be hoist to temporary "saved" variables.
      CollectSavedArrayIndices(
          ctx.src, var->Declaration()->constructor(),
          [&](ast::Expression* idx_expr) {
            // We have a sub-expression that needs to be saved.
            // Create a new variable
            auto saved_name = ctx.dst->Symbols().New(
                ctx.src->Symbols().NameFor(var->Declaration()->symbol()) +
                "_save");
            auto* saved = ctx.dst->Decl(
                ctx.dst->Const(saved_name, nullptr, ctx.Clone(idx_expr)));
            // Place this variable after the pointer typed let. Order here is
            // important as order-of-operations needs to be preserved.
            // CollectSavedArrayIndices() visits the LHS of an array accessor
            // before the index expression.
            // Note that repeated calls to InsertAfter() with the same `after`
            // argument will result in nodes to inserted in the order the calls
            // are made (last call is inserted last).
            ctx.InsertAfter(block->statements(), let, saved);
            // Record the substitution of `idx_expr` to the saved variable with
            // the symbol `saved_name`. This will be used by the ReplaceAll()
            // handler above.
            ptr_let->saved_vars.emplace(idx_expr, saved_name);
          });

      // Record the pointer-typed `let` declaration.
      // This will be used by the ReplaceAll() handler above.
      ptr_lets.emplace(let->variable(), std::move(ptr_let));
      // As the original `let` declaration will be fully inlined, there's no
      // need for the original declaration to exist. Remove it.
      ctx.Remove(block->statements(), let);
    }
  }

  ctx.Clone();

  return Output(Program(std::move(out)));
}

}  // namespace transform
}  // namespace tint
