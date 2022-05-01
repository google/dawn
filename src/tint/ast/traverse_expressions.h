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
#include "src/tint/utils/reverse.h"

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
///        `TraverseAction(const T*)` where T is an ast::Expression type.
/// @return true on success, false on error
template <TraverseOrder ORDER = TraverseOrder::LeftToRight, typename CALLBACK>
bool TraverseExpressions(const ast::Expression* root, diag::List& diags, CALLBACK&& callback) {
    using EXPR_TYPE = std::remove_pointer_t<traits::ParameterType<CALLBACK, 0>>;
    std::vector<const ast::Expression*> to_visit{root};

    auto push_pair = [&](const ast::Expression* left, const ast::Expression* right) {
        if (ORDER == TraverseOrder::LeftToRight) {
            to_visit.push_back(right);
            to_visit.push_back(left);
        } else {
            to_visit.push_back(left);
            to_visit.push_back(right);
        }
    };
    auto push_list = [&](const std::vector<const ast::Expression*>& exprs) {
        if (ORDER == TraverseOrder::LeftToRight) {
            for (auto* expr : utils::Reverse(exprs)) {
                to_visit.push_back(expr);
            }
        } else {
            for (auto* expr : exprs) {
                to_visit.push_back(expr);
            }
        }
    };

    while (!to_visit.empty()) {
        auto* expr = to_visit.back();
        to_visit.pop_back();

        if (auto* filtered = expr->As<EXPR_TYPE>()) {
            switch (callback(filtered)) {
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
                push_pair(idx->object, idx->index);
                return true;
            },
            [&](const BinaryExpression* bin_op) {
                push_pair(bin_op->lhs, bin_op->rhs);
                return true;
            },
            [&](const BitcastExpression* bitcast) {
                to_visit.push_back(bitcast->expr);
                return true;
            },
            [&](const CallExpression* call) {
                // TODO(crbug.com/tint/1257): Resolver breaks if we actually include
                // the function name in the traversal. to_visit.push_back(call->func);
                push_list(call->args);
                return true;
            },
            [&](const MemberAccessorExpression* member) {
                // TODO(crbug.com/tint/1257): Resolver breaks if we actually include
                // the member name in the traversal. push_pair(member->structure,
                // member->member);
                to_visit.push_back(member->structure);
                return true;
            },
            [&](const UnaryOpExpression* unary) {
                to_visit.push_back(unary->expr);
                return true;
            },
            [&](Default) {
                if (expr->IsAnyOf<LiteralExpression, IdentifierExpression, PhonyExpression>()) {
                    return true;  // Leaf expression
                }
                TINT_ICE(AST, diags) << "unhandled expression type: " << expr->TypeInfo().name;
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
