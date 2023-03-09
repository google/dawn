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

#include "src/tint/sem/info.h"

#include "src/tint/sem/function.h"
#include "src/tint/sem/module.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/value_expression.h"
#include "src/tint/switch.h"

namespace tint::sem {

Info::Info() = default;

Info::Info(Info&&) = default;

Info::~Info() = default;

Info& Info::operator=(Info&&) = default;

builtin::DiagnosticSeverity Info::DiagnosticSeverity(const ast::Node* ast_node,
                                                     builtin::DiagnosticRule rule) const {
    // Get the diagnostic severity modification for a node.
    auto check = [&](auto* node) {
        auto& severities = node->DiagnosticSeverities();
        auto itr = severities.find(rule);
        if (itr != severities.end()) {
            return itr->second;
        }
        return builtin::DiagnosticSeverity::kUndefined;
    };

    // Get the diagnostic severity modification for a function.
    auto check_func = [&](const sem::Function* func) {
        auto severity = check(func);
        if (severity != builtin::DiagnosticSeverity::kUndefined) {
            return severity;
        }

        // No severity set on the function, so check the module instead.
        return check(module_);
    };

    // Get the diagnostic severity modification for a statement.
    auto check_stmt = [&](const sem::Statement* stmt) {
        // Walk up the statement hierarchy, checking for diagnostic severity modifications.
        while (true) {
            auto severity = check(stmt);
            if (severity != builtin::DiagnosticSeverity::kUndefined) {
                return severity;
            }
            if (!stmt->Parent()) {
                break;
            }
            stmt = stmt->Parent();
        }

        // No severity set on the statement, so check the function instead.
        return check_func(stmt->Function());
    };

    // Query the diagnostic severity from the semantic node that corresponds to the AST node.
    auto* sem = Get(ast_node);
    TINT_ASSERT(Resolver, sem != nullptr);
    auto severity = Switch(
        sem,  //
        [&](const sem::ValueExpression* expr) { return check_stmt(expr->Stmt()); },
        [&](const sem::Statement* stmt) { return check_stmt(stmt); },
        [&](const sem::Function* func) { return check_func(func); },
        [&](Default) {
            // Use the global severity set on the module.
            return check(module_);
        });
    TINT_ASSERT(Resolver, severity != builtin::DiagnosticSeverity::kUndefined);
    return severity;
}

}  // namespace tint::sem
