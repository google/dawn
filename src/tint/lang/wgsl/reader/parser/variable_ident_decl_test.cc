// Copyright 2020 The Tint Authors.
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

#include "src/tint/lang/wgsl/ast/helper_test.h"
#include "src/tint/lang/wgsl/reader/parser/helper_test.h"

namespace tint::wgsl::reader {
namespace {

TEST_F(WGSLParserTest, VariableIdentDecl_Parses) {
    auto p = parser("my_var : f32");
    auto decl = p->expect_ident_with_type_specifier("test");
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(decl.errored);
    ast::CheckIdentifier(decl->name, "my_var");
    ASSERT_NE(decl->type, nullptr);
    ast::CheckIdentifier(decl->type, "f32");

    EXPECT_EQ(decl->name->source.range, (Source::Range{{1u, 1u}, {1u, 7u}}));
    EXPECT_EQ(decl->type->source.range, (Source::Range{{1u, 10u}, {1u, 13u}}));
}

TEST_F(WGSLParserTest, VariableIdentDecl_Parses_AllowInferredType) {
    auto p = parser("my_var : f32");
    auto decl = p->expect_optionally_typed_ident("test");
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(decl.errored);
    ast::CheckIdentifier(decl->name, "my_var");
    ASSERT_NE(decl->type, nullptr);
    ast::CheckIdentifier(decl->type, "f32");

    EXPECT_EQ(decl->name->source.range, (Source::Range{{1u, 1u}, {1u, 7u}}));
    EXPECT_EQ(decl->type->source.range, (Source::Range{{1u, 10u}, {1u, 13u}}));
}

TEST_F(WGSLParserTest, VariableIdentDecl_Inferred_Parse_Failure) {
    auto p = parser("my_var = 1.0");
    auto decl = p->expect_ident_with_type_specifier("test");
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:8: expected ':' for test");
}

TEST_F(WGSLParserTest, VariableIdentDecl_Inferred_Parses_AllowInferredType) {
    auto p = parser("my_var = 1.0");
    auto decl = p->expect_optionally_typed_ident("test");
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(decl.errored);
    ast::CheckIdentifier(decl->name, "my_var");
    ASSERT_EQ(decl->type, nullptr);

    EXPECT_EQ(decl->name->source.range, (Source::Range{{1u, 1u}, {1u, 7u}}));
}

TEST_F(WGSLParserTest, VariableIdentDecl_MissingIdent) {
    auto p = parser(": f32");
    auto decl = p->expect_ident_with_type_specifier("test");
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(decl.errored);
    ASSERT_EQ(p->error(), "1:1: expected identifier for test");
}

TEST_F(WGSLParserTest, VariableIdentDecl_MissingIdent_AllowInferredType) {
    auto p = parser(": f32");
    auto decl = p->expect_optionally_typed_ident("test");
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(decl.errored);
    ASSERT_EQ(p->error(), "1:1: expected identifier for test");
}

TEST_F(WGSLParserTest, VariableIdentDecl_MissingType) {
    auto p = parser("my_var :");
    auto decl = p->expect_ident_with_type_specifier("test");
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(decl.errored);
    ASSERT_EQ(p->error(), "1:9: invalid type for test");
}

TEST_F(WGSLParserTest, VariableIdentDecl_MissingType_AllowInferredType) {
    auto p = parser("my_var :");
    auto decl = p->expect_optionally_typed_ident("test");
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(decl.errored);
    ASSERT_EQ(p->error(), "1:9: invalid type for test");
}

TEST_F(WGSLParserTest, VariableIdentDecl_InvalidIdent) {
    auto p = parser("123 : f32");
    auto decl = p->expect_ident_with_type_specifier("test");
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(decl.errored);
    ASSERT_EQ(p->error(), "1:1: expected identifier for test");
}

TEST_F(WGSLParserTest, VariableIdentDecl_InvalidIdent_AllowInferredType) {
    auto p = parser("123 : f32");
    auto decl = p->expect_optionally_typed_ident("test");
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(decl.errored);
    ASSERT_EQ(p->error(), "1:1: expected identifier for test");
}

}  // namespace
}  // namespace tint::wgsl::reader
