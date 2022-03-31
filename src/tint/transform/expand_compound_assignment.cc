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

#include "src/tint/transform/expand_compound_assignment.h"

#include <utility>

#include "src/tint/ast/compound_assignment_statement.h"
#include "src/tint/program_builder.h"
#include "src/tint/sem/block_statement.h"
#include "src/tint/sem/expression.h"
#include "src/tint/sem/for_loop_statement.h"
#include "src/tint/sem/statement.h"
#include "src/tint/transform/utils/hoist_to_decl_before.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::ExpandCompoundAssignment);

namespace tint {
namespace transform {

ExpandCompoundAssignment::ExpandCompoundAssignment() = default;

ExpandCompoundAssignment::~ExpandCompoundAssignment() = default;

bool ExpandCompoundAssignment::ShouldRun(const Program* program,
                                         const DataMap&) const {
  for (auto* node : program->ASTNodes().Objects()) {
    if (node->Is<ast::CompoundAssignmentStatement>()) {
      return true;
    }
  }
  return false;
}

void ExpandCompoundAssignment::Run(CloneContext& ctx,
                                   const DataMap&,
                                   DataMap&) const {
  HoistToDeclBefore hoist_to_decl_before(ctx);

  for (auto* node : ctx.src->ASTNodes().Objects()) {
    if (auto* assign = node->As<ast::CompoundAssignmentStatement>()) {
      auto* sem_assign = ctx.src->Sem().Get(assign);

      // Helper function to create the LHS expression. This will be called twice
      // when building the non-compound assignment statement, so must not
      // produce expressions that cause side effects.
      std::function<const ast::Expression*()> lhs;

      // Helper function to create a variable that is a pointer to `expr`.
      auto hoist_pointer_to = [&](const ast::Expression* expr) {
        auto name = ctx.dst->Sym();
        auto* ptr = ctx.dst->AddressOf(ctx.Clone(expr));
        auto* decl = ctx.dst->Decl(ctx.dst->Const(name, nullptr, ptr));
        hoist_to_decl_before.InsertBefore(sem_assign, decl);
        return name;
      };

      // Helper function to hoist `expr` to a let declaration.
      auto hoist_expr_to_let = [&](const ast::Expression* expr) {
        auto name = ctx.dst->Sym();
        auto* decl =
            ctx.dst->Decl(ctx.dst->Const(name, nullptr, ctx.Clone(expr)));
        hoist_to_decl_before.InsertBefore(sem_assign, decl);
        return name;
      };

      // Helper function that returns `true` if the type of `expr` is a vector.
      auto is_vec = [&](const ast::Expression* expr) {
        return ctx.src->Sem().Get(expr)->Type()->UnwrapRef()->Is<sem::Vector>();
      };

      // Hoist the LHS expression subtree into local constants to produce a new
      // LHS that we can evaluate twice.
      // We need to special case compound assignments to vector components since
      // we cannot take the address of a vector component.
      auto* index_accessor = assign->lhs->As<ast::IndexAccessorExpression>();
      auto* member_accessor = assign->lhs->As<ast::MemberAccessorExpression>();
      if (assign->lhs->Is<ast::IdentifierExpression>() ||
          (member_accessor &&
           member_accessor->structure->Is<ast::IdentifierExpression>())) {
        // This is the simple case with no side effects, so we can just use the
        // original LHS expression directly.
        // Before:
        //     foo.bar += rhs;
        // After:
        //     foo.bar = foo.bar + rhs;
        lhs = [&]() { return ctx.Clone(assign->lhs); };
      } else if (index_accessor && is_vec(index_accessor->object)) {
        // This is the case for vector component via an array accessor. We need
        // to capture a pointer to the vector and also the index value.
        // Before:
        //     v[idx()] += rhs;
        // After:
        //     let vec_ptr = &v;
        //     let index = idx();
        //     (*vec_ptr)[index] = (*vec_ptr)[index] + rhs;
        auto lhs_ptr = hoist_pointer_to(index_accessor->object);
        auto index = hoist_expr_to_let(index_accessor->index);
        lhs = [&, lhs_ptr, index]() {
          return ctx.dst->IndexAccessor(ctx.dst->Deref(lhs_ptr), index);
        };
      } else if (member_accessor && is_vec(member_accessor->structure)) {
        // This is the case for vector component via a member accessor. We just
        // need to capture a pointer to the vector.
        // Before:
        //     a[idx()].y += rhs;
        // After:
        //     let vec_ptr = &a[idx()];
        //     (*vec_ptr).y = (*vec_ptr).y + rhs;
        auto lhs_ptr = hoist_pointer_to(member_accessor->structure);
        lhs = [&, lhs_ptr]() {
          return ctx.dst->MemberAccessor(ctx.dst->Deref(lhs_ptr),
                                         ctx.Clone(member_accessor->member));
        };
      } else {
        // For all other statements that may have side-effecting expressions, we
        // just need to capture a pointer to the whole LHS.
        // Before:
        //     a[idx()] += rhs;
        // After:
        //     let lhs_ptr = &a[idx()];
        //     (*lhs_ptr) = (*lhs_ptr) + rhs;
        auto lhs_ptr = hoist_pointer_to(assign->lhs);
        lhs = [&, lhs_ptr]() { return ctx.dst->Deref(lhs_ptr); };
      }

      // Replace the compound assignment with a regular assignment.
      auto* rhs = ctx.dst->create<ast::BinaryExpression>(
          assign->op, lhs(), ctx.Clone(assign->rhs));
      ctx.Replace(assign, ctx.dst->Assign(lhs(), rhs));
    }
  }
  hoist_to_decl_before.Apply();
  ctx.Clone();
}

}  // namespace transform
}  // namespace tint
