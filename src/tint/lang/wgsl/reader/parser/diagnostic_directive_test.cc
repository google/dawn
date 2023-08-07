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

#include "src/tint/lang/wgsl/reader/parser/helper_test.h"

#include "src/tint/lang/wgsl/ast/diagnostic_control.h"
#include "src/tint/lang/wgsl/ast/helper_test.h"

namespace tint::wgsl::reader {
namespace {

TEST_F(WGSLParserTest, DiagnosticDirective_Name) {
    auto p = parser("diagnostic(off, foo);");
    p->diagnostic_directive();
    EXPECT_FALSE(p->has_error()) << p->error();
    auto& ast = p->builder().AST();
    ASSERT_EQ(ast.DiagnosticDirectives().Length(), 1u);
    auto* directive = ast.DiagnosticDirectives()[0];
    EXPECT_EQ(directive->control.severity, core::DiagnosticSeverity::kOff);
    ASSERT_EQ(ast.GlobalDeclarations().Length(), 1u);
    EXPECT_EQ(ast.GlobalDeclarations()[0], directive);

    auto* r = directive->control.rule_name;
    ASSERT_NE(r, nullptr);
    EXPECT_EQ(r->category, nullptr);
    ast::CheckIdentifier(r->name, "foo");
}

TEST_F(WGSLParserTest, DiagnosticDirective_CategoryName) {
    auto p = parser("diagnostic(off, foo.bar);");
    p->diagnostic_directive();
    EXPECT_FALSE(p->has_error()) << p->error();
    auto& ast = p->builder().AST();
    ASSERT_EQ(ast.DiagnosticDirectives().Length(), 1u);
    auto* directive = ast.DiagnosticDirectives()[0];
    EXPECT_EQ(directive->control.severity, core::DiagnosticSeverity::kOff);
    ASSERT_EQ(ast.GlobalDeclarations().Length(), 1u);
    EXPECT_EQ(ast.GlobalDeclarations()[0], directive);

    auto* r = directive->control.rule_name;
    ASSERT_NE(r, nullptr);
    ast::CheckIdentifier(r->category, "foo");
    ast::CheckIdentifier(r->name, "bar");
}

TEST_F(WGSLParserTest, DiagnosticDirective_MissingSemicolon) {
    auto p = parser("diagnostic(off, foo)");
    p->translation_unit();
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:21: expected ';' for diagnostic directive");
    auto program = p->program();
    auto& ast = program.AST();
    EXPECT_EQ(ast.DiagnosticDirectives().Length(), 0u);
    EXPECT_EQ(ast.GlobalDeclarations().Length(), 0u);
}

TEST_F(WGSLParserTest, DiagnosticDirective_FollowingOtherGlobalDecl) {
    auto p = parser(R"(
var<private> t: f32 = 0f;
diagnostic(off, foo);
)");
    p->translation_unit();
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "3:1: directives must come before all global declarations");
}

TEST_F(WGSLParserTest, DiagnosticDirective_FollowingEmptySemicolon) {
    auto p = parser(R"(
;
diagnostic(off, foo);
)");
    p->translation_unit();
    // An empty semicolon is treated as a global declaration.
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "3:1: directives must come before all global declarations");
}

}  // namespace
}  // namespace tint::wgsl::reader
