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

#include "src/tint/lang/wgsl/ast/transform/expand_compound_assignment.h"

#include <utility>

#include "src/tint/lang/wgsl/ast/compound_assignment_statement.h"
#include "src/tint/lang/wgsl/ast/increment_decrement_statement.h"
#include "src/tint/lang/wgsl/ast/transform/utils/hoist_to_decl_before.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/sem/block_statement.h"
#include "src/tint/lang/wgsl/sem/for_loop_statement.h"
#include "src/tint/lang/wgsl/sem/statement.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::ExpandCompoundAssignment);

using namespace tint::number_suffixes;  // NOLINT

namespace tint::ast::transform {

namespace {

bool ShouldRun(const Program* program) {
    for (auto* node : program->ASTNodes().Objects()) {
        if (node->IsAnyOf<CompoundAssignmentStatement, IncrementDecrementStatement>()) {
            return true;
        }
    }
    return false;
}

}  // namespace

/// PIMPL state for the transform
struct ExpandCompoundAssignment::State {
    /// Constructor
    /// @param context the clone context
    explicit State(program::CloneContext& context)
        : ctx(context), b(*ctx.dst), hoist_to_decl_before(ctx) {}

    /// Replace `stmt` with a regular assignment statement of the form:
    ///     lhs = lhs op rhs
    /// The LHS expression will only be evaluated once, and any side effects will
    /// be hoisted to `let` declarations above the assignment statement.
    /// @param stmt the statement to replace
    /// @param lhs the lhs expression from the source statement
    /// @param rhs the rhs expression in the destination module
    /// @param op the binary operator
    void Expand(const Statement* stmt, const Expression* lhs, const Expression* rhs, BinaryOp op) {
        // Helper function to create the new LHS expression. This will be called
        // twice when building the non-compound assignment statement, so must
        // not produce expressions that cause side effects.
        std::function<const Expression*()> new_lhs;

        // Helper function to create a variable that is a pointer to `expr`.
        auto hoist_pointer_to = [&](const Expression* expr) {
            auto name = b.Sym();
            auto* ptr = b.AddressOf(ctx.Clone(expr));
            auto* decl = b.Decl(b.Let(name, ptr));
            hoist_to_decl_before.InsertBefore(ctx.src->Sem().Get(stmt), decl);
            return name;
        };

        // Helper function to hoist `expr` to a let declaration.
        auto hoist_expr_to_let = [&](const Expression* expr) {
            auto name = b.Sym();
            auto* decl = b.Decl(b.Let(name, ctx.Clone(expr)));
            hoist_to_decl_before.InsertBefore(ctx.src->Sem().Get(stmt), decl);
            return name;
        };

        // Helper function that returns `true` if the type of `expr` is a vector.
        auto is_vec = [&](const Expression* expr) {
            if (auto* val_expr = ctx.src->Sem().GetVal(expr)) {
                return val_expr->Type()->UnwrapRef()->Is<type::Vector>();
            }
            return false;
        };

        // Hoist the LHS expression subtree into local constants to produce a new
        // LHS that we can evaluate twice.
        // We need to special case compound assignments to vector components since
        // we cannot take the address of a vector component.
        auto* index_accessor = lhs->As<IndexAccessorExpression>();
        auto* member_accessor = lhs->As<MemberAccessorExpression>();
        if (lhs->Is<IdentifierExpression>() ||
            (member_accessor && member_accessor->object->Is<IdentifierExpression>())) {
            // This is the simple case with no side effects, so we can just use the
            // original LHS expression directly.
            // Before:
            //     foo.bar += rhs;
            // After:
            //     foo.bar = foo.bar + rhs;
            new_lhs = [&] { return ctx.Clone(lhs); };
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
            new_lhs = [&, lhs_ptr, index] { return b.IndexAccessor(b.Deref(lhs_ptr), index); };
        } else if (member_accessor && is_vec(member_accessor->object)) {
            // This is the case for vector component via a member accessor. We just
            // need to capture a pointer to the vector.
            // Before:
            //     a[idx()].y += rhs;
            // After:
            //     let vec_ptr = &a[idx()];
            //     (*vec_ptr).y = (*vec_ptr).y + rhs;
            auto lhs_ptr = hoist_pointer_to(member_accessor->object);
            new_lhs = [&, lhs_ptr] {
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
            new_lhs = [&, lhs_ptr] { return b.Deref(lhs_ptr); };
        }

        // Replace the statement with a regular assignment statement.
        auto* value = b.create<BinaryExpression>(op, new_lhs(), rhs);
        ctx.Replace(stmt, b.Assign(new_lhs(), value));
    }

  private:
    /// The clone context.
    program::CloneContext& ctx;

    /// The AST builder.
    ast::Builder& b;

    /// The HoistToDeclBefore helper instance.
    HoistToDeclBefore hoist_to_decl_before;
};

ExpandCompoundAssignment::ExpandCompoundAssignment() = default;

ExpandCompoundAssignment::~ExpandCompoundAssignment() = default;

Transform::ApplyResult ExpandCompoundAssignment::Apply(const Program* src,
                                                       const DataMap&,
                                                       DataMap&) const {
    if (!ShouldRun(src)) {
        return SkipTransform;
    }

    ProgramBuilder b;
    program::CloneContext ctx{&b, src, /* auto_clone_symbols */ true};
    State state(ctx);
    for (auto* node : src->ASTNodes().Objects()) {
        if (auto* assign = node->As<CompoundAssignmentStatement>()) {
            state.Expand(assign, assign->lhs, ctx.Clone(assign->rhs), assign->op);
        } else if (auto* inc_dec = node->As<IncrementDecrementStatement>()) {
            // For increment/decrement statements, `i++` becomes `i = i + 1`.
            auto op = inc_dec->increment ? BinaryOp::kAdd : BinaryOp::kSubtract;
            state.Expand(inc_dec, inc_dec->lhs, ctx.dst->Expr(1_a), op);
        }
    }

    ctx.Clone();
    return Program(std::move(b));
}

}  // namespace tint::ast::transform
