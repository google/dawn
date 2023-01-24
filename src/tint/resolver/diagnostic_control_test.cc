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

}  // namespace
}  // namespace tint::resolver
