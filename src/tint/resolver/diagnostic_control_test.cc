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

#include "src/tint/resolver/resolver.h"

#include "src/tint/resolver/resolver_test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ResolverDiagnosticControlTest = ResolverTest;

TEST_F(ResolverDiagnosticControlTest, UnreachableCode_DefaultSeverity) {
    auto stmts = utils::Vector{Return(), Return()};
    Func("foo", {}, ty.void_(), stmts);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), R"(warning: code is unreachable)");
}

TEST_F(ResolverDiagnosticControlTest, UnreachableCode_ErrorViaDirective) {
    DiagnosticDirective(ast::DiagnosticSeverity::kError, Expr("chromium_unreachable_code"));

    auto stmts = utils::Vector{Return(), Return()};
    Func("foo", {}, ty.void_(), stmts);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: code is unreachable)");
}

TEST_F(ResolverDiagnosticControlTest, UnreachableCode_WarningViaDirective) {
    DiagnosticDirective(ast::DiagnosticSeverity::kWarning, Expr("chromium_unreachable_code"));

    auto stmts = utils::Vector{Return(), Return()};
    Func("foo", {}, ty.void_(), stmts);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), R"(warning: code is unreachable)");
}

TEST_F(ResolverDiagnosticControlTest, UnreachableCode_InfoViaDirective) {
    DiagnosticDirective(ast::DiagnosticSeverity::kInfo, Expr("chromium_unreachable_code"));

    auto stmts = utils::Vector{Return(), Return()};
    Func("foo", {}, ty.void_(), stmts);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), R"(note: code is unreachable)");
}

TEST_F(ResolverDiagnosticControlTest, UnreachableCode_OffViaDirective) {
    DiagnosticDirective(ast::DiagnosticSeverity::kOff, Expr("chromium_unreachable_code"));

    auto stmts = utils::Vector{Return(), Return()};
    Func("foo", {}, ty.void_(), stmts);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(r()->error().empty());
}

TEST_F(ResolverDiagnosticControlTest, UnreachableCode_ErrorViaAttribute) {
    auto* attr =
        DiagnosticAttribute(ast::DiagnosticSeverity::kError, Expr("chromium_unreachable_code"));

    auto stmts = utils::Vector{Return(), Return()};
    Func("foo", {}, ty.void_(), stmts, utils::Vector{attr});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: code is unreachable)");
}

TEST_F(ResolverDiagnosticControlTest, UnreachableCode_WarningViaAttribute) {
    auto* attr =
        DiagnosticAttribute(ast::DiagnosticSeverity::kWarning, Expr("chromium_unreachable_code"));

    auto stmts = utils::Vector{Return(), Return()};
    Func("foo", {}, ty.void_(), stmts, utils::Vector{attr});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), R"(warning: code is unreachable)");
}

TEST_F(ResolverDiagnosticControlTest, UnreachableCode_InfoViaAttribute) {
    auto* attr =
        DiagnosticAttribute(ast::DiagnosticSeverity::kInfo, Expr("chromium_unreachable_code"));

    auto stmts = utils::Vector{Return(), Return()};
    Func("foo", {}, ty.void_(), stmts, utils::Vector{attr});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), R"(note: code is unreachable)");
}

TEST_F(ResolverDiagnosticControlTest, UnreachableCode_OffViaAttribute) {
    auto* attr =
        DiagnosticAttribute(ast::DiagnosticSeverity::kOff, Expr("chromium_unreachable_code"));

    auto stmts = utils::Vector{Return(), Return()};
    Func("foo", {}, ty.void_(), stmts, utils::Vector{attr});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(r()->error().empty());
}

TEST_F(ResolverDiagnosticControlTest, UnreachableCode_ErrorViaDirective_OverriddenViaAttribute) {
    // diagnostic(error, chromium_unreachable_code);
    //
    // @diagnostic(off, chromium_unreachable_code) fn foo() {
    //   return;
    //   return; // Should produce a warning
    // }
    DiagnosticDirective(ast::DiagnosticSeverity::kError, Expr("chromium_unreachable_code"));
    auto* attr =
        DiagnosticAttribute(ast::DiagnosticSeverity::kWarning, Expr("chromium_unreachable_code"));

    auto stmts = utils::Vector{Return(), Return()};
    Func("foo", {}, ty.void_(), stmts, utils::Vector{attr});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), R"(warning: code is unreachable)");
}

TEST_F(ResolverDiagnosticControlTest, FunctionAttributeScope) {
    // @diagnostic(off, chromium_unreachable_code) fn foo() {
    //   return;
    //   return; // Should not produce a diagnostic
    // }
    //
    // fn zoo() {
    //   return;
    //   return; // Should produce a warning (default severity)
    // }
    //
    // @diagnostic(info, chromium_unreachable_code) fn bar() {
    //   return;
    //   return; // Should produce an info
    // }
    {
        auto* attr =
            DiagnosticAttribute(ast::DiagnosticSeverity::kOff, Expr("chromium_unreachable_code"));
        Func("foo", {}, ty.void_(),
             utils::Vector{
                 Return(),
                 Return(Source{{12, 34}}),
             },
             utils::Vector{attr});
    }
    {
        Func("bar", {}, ty.void_(),
             utils::Vector{
                 Return(),
                 Return(Source{{45, 67}}),
             });
    }
    {
        auto* attr =
            DiagnosticAttribute(ast::DiagnosticSeverity::kInfo, Expr("chromium_unreachable_code"));
        Func("zoo", {}, ty.void_(),
             utils::Vector{
                 Return(),
                 Return(Source{{89, 10}}),
             },
             utils::Vector{attr});
    }

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), R"(45:67 warning: code is unreachable
89:10 note: code is unreachable)");
}

TEST_F(ResolverDiagnosticControlTest, UnrecognizedRuleName_Directive) {
    DiagnosticDirective(ast::DiagnosticSeverity::kError,
                        Expr(Source{{12, 34}}, "chromium_unreachable_cod"));
    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              R"(12:34 warning: unrecognized diagnostic rule 'chromium_unreachable_cod'
Did you mean 'chromium_unreachable_code'?
Possible values: 'chromium_unreachable_code', 'derivative_uniformity')");
}

TEST_F(ResolverDiagnosticControlTest, UnrecognizedRuleName_Attribute) {
    auto* attr = DiagnosticAttribute(ast::DiagnosticSeverity::kError,
                                     Expr(Source{{12, 34}}, "chromium_unreachable_cod"));
    Func("foo", {}, ty.void_(), {}, utils::Vector{attr});
    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              R"(12:34 warning: unrecognized diagnostic rule 'chromium_unreachable_cod'
Did you mean 'chromium_unreachable_code'?
Possible values: 'chromium_unreachable_code', 'derivative_uniformity')");
}

}  // namespace
}  // namespace tint::resolver
