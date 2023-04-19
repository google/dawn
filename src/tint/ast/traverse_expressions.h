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

#ifndef SRC_TINT_AST_TRAVERSE_EXPRESSIONS_H_
#define SRC_TINT_AST_TRAVERSE_EXPRESSIONS_H_

#include <vector>

#include "src/tint/ast/binary_expression.h"
#include "src/tint/ast/bitcast_expression.h"
#include "src/tint/ast/call_expression.h"
#include "src/tint/ast/index_accessor_expression.h"
#include "src/tint/ast/literal_expression.h"
#include "src/tint/ast/member_accessor_expression.h"
#include "src/tint/ast/phony_expression.h"
#include "src/tint/ast/unary_op_expression.h"
#include "src/tint/switch.h"
#include "src/tint/utils/compiler_macros.h"
#include "src/tint/utils/reverse.h"
#include "src/tint/utils/vector.h"

namespace tint::ast {

/// The action to perform after calling the TraverseExpressions() callback
/// function.
enum class TraverseAction {
    /// Stop traversal immediately.
    Stop,
    /// Descend into this expression.
    Descend,
    /// Do not descend into this expression.
    Skip,
};

/// The order TraverseExpressions() will traverse expressions
enum class TraverseOrder {
    /// Expressions will be traversed from left to right
    LeftToRight,
    /// Expressions will be traversed from right to left
    RightToLeft,
};

/// TraverseExpressions performs a depth-first traversal of the expression nodes
/// from `root`, calling `callback` for each of the visited expressions that
/// match the predicate parameter type, in pre-ordering (root first).
/// @param root the root expression node
/// @param diags the diagnostics used for error messages
/// @param callback the callback function. Must be of the signature:
///        `TraverseAction(const T* expr)` or `TraverseAction(const T* expr, size_t depth)` where T
///        is an Expression type.
/// @return true on success, false on error
template <TraverseOrder ORDER = TraverseOrder::LeftToRight, typename CALLBACK>
bool TraverseExpressions(const Expression* root, diag::List& diags, CALLBACK&& callback) {
    using EXPR_TYPE = std::remove_pointer_t<utils::traits::ParameterType<CALLBACK, 0>>;
    constexpr static bool kHasDepthArg =
        utils::traits::SignatureOfT<CALLBACK>::parameter_count == 2;

    struct Pending {
        const Expression* expr;
        size_t depth;
    };

    utils::Vector<Pending, 32> to_visit{{root, 0}};

    auto push_single = [&](const Expression* expr, size_t depth) { to_visit.Push({expr, depth}); };
    auto push_pair = [&](const Expression* left, const Expression* right, size_t depth) {
        if (ORDER == TraverseOrder::LeftToRight) {
            to_visit.Push({right, depth});
            to_visit.Push({left, depth});
        } else {
            to_visit.Push({left, depth});
            to_visit.Push({right, depth});
        }
    };
    auto push_list = [&](utils::VectorRef<const Expression*> exprs, size_t depth) {
        if (ORDER == TraverseOrder::LeftToRight) {
            for (auto* expr : utils::Reverse(exprs)) {
                to_visit.Push({expr, depth});
            }
        } else {
            for (auto* expr : exprs) {
                to_visit.Push({expr, depth});
            }
        }
    };

    while (!to_visit.IsEmpty()) {
        auto p = to_visit.Pop();
        const Expression* expr = p.expr;

        if (auto* filtered = expr->template As<EXPR_TYPE>()) {
            TraverseAction result;
            if constexpr (kHasDepthArg) {
                result = callback(filtered, p.depth);
            } else {
                result = callback(filtered);
            }

            switch (result) {
                case TraverseAction::Stop:
                    return true;
                case TraverseAction::Skip:
                    continue;
                case TraverseAction::Descend:
                    break;
            }
        }

        bool ok = Switch(
            expr,
            [&](const IndexAccessorExpression* idx) {
                push_pair(idx->object, idx->index, p.depth + 1);
                return true;
            },
            [&](const BinaryExpression* bin_op) {
                push_pair(bin_op->lhs, bin_op->rhs, p.depth + 1);
                return true;
            },
            [&](const BitcastExpression* bitcast) {
                push_single(bitcast->expr, p.depth + 1);
                return true;
            },
            [&](const CallExpression* call) {
                push_list(call->args, p.depth + 1);
                return true;
            },
            [&](const MemberAccessorExpression* member) {
                push_single(member->object, p.depth + 1);
                return true;
            },
            [&](const UnaryOpExpression* unary) {
                push_single(unary->expr, p.depth + 1);
                return true;
            },
            [&](Default) {
                if (TINT_LIKELY((expr->IsAnyOf<LiteralExpression, IdentifierExpression,
                                               PhonyExpression>()))) {
                    return true;  // Leaf expression
                }
                TINT_ICE(AST, diags)
                    << "unhandled expression type: " << (expr ? expr->TypeInfo().name : "<null>");
                return false;
            });
        if (!ok) {
            return false;
        }
    }
    return true;
}

}  // namespace tint::ast

#endif  // SRC_TINT_AST_TRAVERSE_EXPRESSIONS_H_
