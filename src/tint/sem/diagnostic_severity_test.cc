// Copyright 2023 The Tint Authors.
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

#include "src/tint/sem/test_helper.h"

#include "src/tint/sem/module.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::sem {
namespace {

class DiagnosticSeverityTest : public TestHelper {
  protected:
    /// Create a program with two functions, setting the severity for "chromium_unreachable_code"
    /// using an attribute. Test that we correctly track the severity of the filter for the
    /// functions and the statements with them.
    /// @param global_severity the global severity of the "chromium_unreachable_code" filter
    void Run(builtin::DiagnosticSeverity global_severity) {
        // @diagnostic(off, chromium_unreachable_code)
        // fn foo() {
        //   @diagnostic(info, chromium_unreachable_code) {
        //     @diagnostic(error, chromium_unreachable_code)
        //     if (true) @diagnostic(warning, chromium_unreachable_code) {
        //       return;
        //     } else if (false) {
        //       return;
        //     } else @diagnostic(info, chromium_unreachable_code) {
        //       return;
        //     }
        //     return;
        //   }
        // }
        //
        // fn bar() {
        //   return;
        // }
        auto rule = builtin::DiagnosticRule::kChromiumUnreachableCode;
        auto func_severity = builtin::DiagnosticSeverity::kOff;
        auto block_severity = builtin::DiagnosticSeverity::kInfo;
        auto if_severity = builtin::DiagnosticSeverity::kError;
        auto if_body_severity = builtin::DiagnosticSeverity::kWarning;
        auto else_body_severity = builtin::DiagnosticSeverity::kInfo;
        auto attr = [&](auto severity) {
            return utils::Vector{DiagnosticAttribute(severity, "chromium_unreachable_code")};
        };

        auto* return_foo_if = Return();
        auto* return_foo_elseif = Return();
        auto* return_foo_else = Return();
        auto* return_foo_block = Return();
        auto* else_stmt = Block(utils::Vector{return_foo_else}, attr(else_body_severity));
        auto* elseif = If(Expr(false), Block(return_foo_elseif), Else(else_stmt));
        auto* if_foo = If(Expr(true), Block(utils::Vector{return_foo_if}, attr(if_body_severity)),
                          Else(elseif), attr(if_severity));
        auto* block_1 = Block(utils::Vector{if_foo, return_foo_block}, attr(block_severity));
        auto* func_attr = DiagnosticAttribute(func_severity, "chromium_unreachable_code");
        auto* foo = Func("foo", {}, ty.void_(), utils::Vector{block_1}, utils::Vector{func_attr});

        auto* return_bar = Return();
        auto* bar = Func("bar", {}, ty.void_(), utils::Vector{return_bar});

        auto p = Build();
        EXPECT_TRUE(p.IsValid()) << p.Diagnostics().str();

        EXPECT_EQ(p.Sem().DiagnosticSeverity(foo, rule), func_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(block_1, rule), block_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(if_foo, rule), if_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(if_foo->condition, rule), if_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(if_foo->body, rule), if_body_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(return_foo_if, rule), if_body_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(elseif, rule), if_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(elseif->condition, rule), if_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(elseif->body, rule), if_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(return_foo_elseif, rule), if_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(else_stmt, rule), else_body_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(return_foo_else, rule), else_body_severity);

        EXPECT_EQ(p.Sem().DiagnosticSeverity(bar, rule), global_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(return_bar, rule), global_severity);
    }
};

TEST_F(DiagnosticSeverityTest, WithDirective) {
    DiagnosticDirective(builtin::DiagnosticSeverity::kError, "chromium_unreachable_code");
    Run(builtin::DiagnosticSeverity::kError);
}

TEST_F(DiagnosticSeverityTest, WithoutDirective) {
    Run(builtin::DiagnosticSeverity::kWarning);
}

}  // namespace
}  // namespace tint::sem
