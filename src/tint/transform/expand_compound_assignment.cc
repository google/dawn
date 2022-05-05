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
#include "src/tint/ast/increment_decrement_statement.h"
#include "src/tint/program_builder.h"
#include "src/tint/sem/block_statement.h"
#include "src/tint/sem/expression.h"
#include "src/tint/sem/for_loop_statement.h"
#include "src/tint/sem/statement.h"
#include "src/tint/transform/utils/hoist_to_decl_before.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::ExpandCompoundAssignment);

using namespace tint::number_suffixes;  // NOLINT

namespace tint::transform {

ExpandCompoundAssignment::ExpandCompoundAssignment() = default;

ExpandCompoundAssignment::~ExpandCompoundAssignment() = default;

bool ExpandCompoundAssignment::ShouldRun(const Program* program, const DataMap&) const {
    for (auto* node : program->ASTNodes().Objects()) {
        if (node->IsAnyOf<ast::CompoundAssignmentStatement, ast::IncrementDecrementStatement>()) {
            return true;
        }
    }
    return false;
}

/// Internal class used to collect statement expansions during the transform.
class State {
  private:
    /// The clone context.
    CloneContext& ctx;

    /// The program builder.
    ProgramBuilder& b;

    /// The HoistToDeclBefore helper instance.
    HoistToDeclBefore hoist_to_decl_before;

  public:
    /// Constructor
    /// @param context the clone context
    explicit State(CloneContext& context) : ctx(context), b(*ctx.dst), hoist_to_decl_before(ctx) {}

    /// Replace `stmt` with a regular assignment statement of the form:
    ///     lhs = lhs op rhs
    /// The LHS expression will only be evaluated once, and any side effects will
    /// be hoisted to `let` declarations above the assignment statement.
    /// @param stmt the statement to replace
    /// @param lhs the lhs expression from the source statement
    /// @param rhs the rhs expression in the destination module
    /// @param op the binary operator
    void Expand(const ast::Statement* stmt,
                const ast::Expression* lhs,
                const ast::Expression* rhs,
                ast::BinaryOp op) {
        // Helper function to create the new LHS expression. This will be called
        // twice when building the non-compound assignment statement, so must
        // not produce expressions that cause side effects.
        std::function<const ast::Expression*()> new_lhs;

        // Helper function to create a variable that is a pointer to `expr`.
        auto hoist_pointer_to = [&](const ast::Expression* expr) {
            auto name = b.Sym();
            auto* ptr = b.AddressOf(ctx.Clone(expr));
            auto* decl = b.Decl(b.Let(name, nullptr, ptr));
            hoist_to_decl_before.InsertBefore(ctx.src->Sem().Get(stmt), decl);
            return name;
        };

        // Helper function to hoist `expr` to a let declaration.
        auto hoist_expr_to_let = [&](const ast::Expression* expr) {
            auto name = b.Sym();
            auto* decl = b.Decl(b.Let(name, nullptr, ctx.Clone(expr)));
            hoist_to_decl_before.InsertBefore(ctx.src->Sem().Get(stmt), decl);
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
        auto* index_accessor = lhs->As<ast::IndexAccessorExpression>();
        auto* member_accessor = lhs->As<ast::MemberAccessorExpression>();
        if (lhs->Is<ast::IdentifierExpression>() ||
            (member_accessor && member_accessor->structure->Is<ast::IdentifierExpression>())) {
            // This is the simple case with no side effects, so we can just use the
            // original LHS expression directly.
            // Before:
            //     foo.bar += rhs;
            // After:
            //     foo.bar = foo.bar + rhs;
            new_lhs = [&]() { return ctx.Clone(lhs); };
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
            new_lhs = [&, lhs_ptr, index]() { return b.IndexAccessor(b.Deref(lhs_ptr), index); };
        } else if (member_accessor && is_vec(member_accessor->structure)) {
            // This is the case for vector component via a member accessor. We just
            // need to capture a pointer to the vector.
            // Before:
            //     a[idx()].y += rhs;
            // After:
            //     let vec_ptr = &a[idx()];
            //     (*vec_ptr).y = (*vec_ptr).y + rhs;
            auto lhs_ptr = hoist_pointer_to(member_accessor->structure);
            new_lhs = [&, lhs_ptr]() {
                return b.MemberAccessor(b.Deref(lhs_ptr), ctx.Clone(member_accessor->member));
            };
        } else {
            // For all other statements that may have side-effecting expressions, we
            // just need to capture a pointer to the whole LHS.
            // Before:
            //     a[idx()] += rhs;
            // After:
            //     let lhs_ptr = &a[idx()];
            //     (*lhs_ptr) = (*lhs_ptr) + rhs;
            auto lhs_ptr = hoist_pointer_to(lhs);
            new_lhs = [&, lhs_ptr]() { return b.Deref(lhs_ptr); };
        }

        // Replace the statement with a regular assignment statement.
        auto* value = b.create<ast::BinaryExpression>(op, new_lhs(), rhs);
        ctx.Replace(stmt, b.Assign(new_lhs(), value));
    }

    /// Finalize the transformation and clone the module.
    void Finalize() {
        hoist_to_decl_before.Apply();
        ctx.Clone();
    }
};

void ExpandCompoundAssignment::Run(CloneContext& ctx, const DataMap&, DataMap&) const {
    State state(ctx);
    for (auto* node : ctx.src->ASTNodes().Objects()) {
        if (auto* assign = node->As<ast::CompoundAssignmentStatement>()) {
            state.Expand(assign, assign->lhs, ctx.Clone(assign->rhs), assign->op);
        } else if (auto* inc_dec = node->As<ast::IncrementDecrementStatement>()) {
            // For increment/decrement statements, `i++` becomes `i = i + 1`.
            // TODO(jrprice): Simplify this when we have untyped literals.
            auto* sem_lhs = ctx.src->Sem().Get(inc_dec->lhs);
            const ast::IntLiteralExpression* one =
                sem_lhs->Type()->UnwrapRef()->is_signed_integer_scalar()
                    ? ctx.dst->Expr(1_i)->As<ast::IntLiteralExpression>()
                    : ctx.dst->Expr(1_u)->As<ast::IntLiteralExpression>();
            auto op = inc_dec->increment ? ast::BinaryOp::kAdd : ast::BinaryOp::kSubtract;
            state.Expand(inc_dec, inc_dec->lhs, one, op);
        }
    }
    state.Finalize();
}

}  // namespace tint::transform
