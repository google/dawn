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
    void Run(ast::DiagnosticSeverity global_severity) {
        // @diagnostic(off, chromium_unreachable_code)
        // fn foo() {
        //   return;
        // }
        //
        // fn bar() {
        //   return;
        // }
        auto rule = ast::DiagnosticRule::kChromiumUnreachableCode;
        auto func_severity = ast::DiagnosticSeverity::kOff;

        auto* return_1 = Return();
        auto* return_2 = Return();
        auto* func_attr = DiagnosticAttribute(func_severity, Expr("chromium_unreachable_code"));
        auto* foo = Func("foo", {}, ty.void_(), utils::Vector{return_1}, utils::Vector{func_attr});
        auto* bar = Func("bar", {}, ty.void_(), utils::Vector{return_2});

        auto p = Build();
        EXPECT_TRUE(p.IsValid()) << p.Diagnostics().str();

        EXPECT_EQ(p.Sem().DiagnosticSeverity(foo, rule), func_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(return_1, rule), func_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(bar, rule), global_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(return_2, rule), global_severity);
    }
};

TEST_F(DiagnosticSeverityTest, WithDirective) {
    DiagnosticDirective(ast::DiagnosticSeverity::kError, Expr("chromium_unreachable_code"));
    Run(ast::DiagnosticSeverity::kError);
}

TEST_F(DiagnosticSeverityTest, WithoutDirective) {
    Run(ast::DiagnosticSeverity::kWarning);
}

}  // namespace
}  // namespace tint::sem
