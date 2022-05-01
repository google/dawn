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

#include "src/tint/reader/wgsl/parser_impl_test_helper.h"

#include "src/tint/ast/enable.h"

namespace tint::reader::wgsl {
namespace {

using EnableDirectiveTest = ParserImplTest;

// Test a valid enable directive.
TEST_F(EnableDirectiveTest, Valid) {
    auto p = parser("enable InternalExtensionForTesting;");
    p->enable_directive();
    EXPECT_FALSE(p->has_error()) << p->error();
    auto program = p->program();
    auto& ast = program.AST();
    EXPECT_EQ(ast.Extensions(),
              ast::ExtensionSet{ast::Enable::ExtensionKind::kInternalExtensionForTesting});
    EXPECT_EQ(ast.GlobalDeclarations().size(), 1u);
    auto node = ast.GlobalDeclarations()[0]->As<ast::Enable>();
    EXPECT_TRUE(node != nullptr);
    EXPECT_EQ(node->name, "InternalExtensionForTesting");
    EXPECT_EQ(node->kind, ast::Enable::ExtensionKind::kInternalExtensionForTesting);
}

// Test multiple enable directives for a same extension.
TEST_F(EnableDirectiveTest, EnableMultipleTime) {
    auto p = parser(R"(
enable InternalExtensionForTesting;
enable InternalExtensionForTesting;
)");
    p->translation_unit();
    EXPECT_FALSE(p->has_error()) << p->error();
    auto program = p->program();
    auto& ast = program.AST();
    EXPECT_EQ(ast.Extensions(),
              ast::ExtensionSet{ast::Enable::ExtensionKind::kInternalExtensionForTesting});
    EXPECT_EQ(ast.GlobalDeclarations().size(), 2u);
    auto node1 = ast.GlobalDeclarations()[0]->As<ast::Enable>();
    EXPECT_TRUE(node1 != nullptr);
    EXPECT_EQ(node1->name, "InternalExtensionForTesting");
    EXPECT_EQ(node1->kind, ast::Enable::ExtensionKind::kInternalExtensionForTesting);
    auto node2 = ast.GlobalDeclarations()[1]->As<ast::Enable>();
    EXPECT_TRUE(node2 != nullptr);
    EXPECT_EQ(node2->name, "InternalExtensionForTesting");
    EXPECT_EQ(node2->kind, ast::Enable::ExtensionKind::kInternalExtensionForTesting);
}

// Test an unknown extension identifier.
TEST_F(EnableDirectiveTest, InvalidIdentifier) {
    auto p = parser("enable NotAValidExtensionName;");
    p->enable_directive();
    // Error when unknown extension found
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:8: unsupported extension: 'NotAValidExtensionName'");
    auto program = p->program();
    auto& ast = program.AST();
    EXPECT_EQ(ast.Extensions().size(), 0u);
    EXPECT_EQ(ast.GlobalDeclarations().size(), 0u);
}

// Test an enable directive missing ending semiclon.
TEST_F(EnableDirectiveTest, MissingEndingSemiclon) {
    auto p = parser("enable InternalExtensionForTesting");
    p->translation_unit();
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:35: expected ';' for enable directive");
    auto program = p->program();
    auto& ast = program.AST();
    EXPECT_EQ(ast.Extensions().size(), 0u);
    EXPECT_EQ(ast.GlobalDeclarations().size(), 0u);
}

// Test using invalid tokens in an enable directive.
TEST_F(EnableDirectiveTest, InvalidTokens) {
    {
        auto p = parser("enable InternalExtensionForTesting<;");
        p->translation_unit();
        EXPECT_TRUE(p->has_error());
        EXPECT_EQ(p->error(), "1:35: expected ';' for enable directive");
        auto program = p->program();
        auto& ast = program.AST();
        EXPECT_EQ(ast.Extensions().size(), 0u);
        EXPECT_EQ(ast.GlobalDeclarations().size(), 0u);
    }
    {
        auto p = parser("enable <InternalExtensionForTesting;");
        p->translation_unit();
        EXPECT_TRUE(p->has_error());
        EXPECT_EQ(p->error(), "1:8: invalid extension name");
        auto program = p->program();
        auto& ast = program.AST();
        EXPECT_EQ(ast.Extensions().size(), 0u);
        EXPECT_EQ(ast.GlobalDeclarations().size(), 0u);
    }
    {
        auto p = parser("enable =;");
        p->translation_unit();
        EXPECT_TRUE(p->has_error());
        EXPECT_EQ(p->error(), "1:8: invalid extension name");
        auto program = p->program();
        auto& ast = program.AST();
        EXPECT_EQ(ast.Extensions().size(), 0u);
        EXPECT_EQ(ast.GlobalDeclarations().size(), 0u);
    }
    {
        auto p = parser("enable vec4;");
        p->translation_unit();
        EXPECT_TRUE(p->has_error());
        EXPECT_EQ(p->error(), "1:8: invalid extension name");
        auto program = p->program();
        auto& ast = program.AST();
        EXPECT_EQ(ast.Extensions().size(), 0u);
        EXPECT_EQ(ast.GlobalDeclarations().size(), 0u);
    }
}

// Test an enable directive go after other global declarations.
TEST_F(EnableDirectiveTest, FollowingOtherGlobalDecl) {
    auto p = parser(R"(
var<private> t: f32 = 0f;
enable InternalExtensionForTesting;
)");
    p->translation_unit();
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "3:1: enable directives must come before all global declarations");
    auto program = p->program();
    auto& ast = program.AST();
    // Accept the enable directive although it cause an error
    EXPECT_EQ(ast.Extensions(),
              ast::ExtensionSet{ast::Enable::ExtensionKind::kInternalExtensionForTesting});
    EXPECT_EQ(ast.GlobalDeclarations().size(), 2u);
}

// Test an enable directive go after an empty semiclon.
TEST_F(EnableDirectiveTest, FollowingEmptySemiclon) {
    auto p = parser(R"(
;
enable InternalExtensionForTesting;
)");
    p->translation_unit();
    // An empty semiclon is treated as a global declaration
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "3:1: enable directives must come before all global declarations");
    auto program = p->program();
    auto& ast = program.AST();
    // Accept the enable directive although it cause an error
    EXPECT_EQ(ast.Extensions(),
              ast::ExtensionSet{ast::Enable::ExtensionKind::kInternalExtensionForTesting});
    EXPECT_EQ(ast.GlobalDeclarations().size(), 1u);
}

}  // namespace
}  // namespace tint::reader::wgsl
