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

TEST_F(WGSLParserTest, StructBodyDecl_Parses) {
    auto p = parser("{a : i32}");

    auto& builder = p->builder();

    auto m = p->expect_struct_body_decl();
    ASSERT_FALSE(p->has_error());
    ASSERT_FALSE(m.errored);
    ASSERT_EQ(m.value.Length(), 1u);

    const auto* mem = m.value[0];
    EXPECT_EQ(mem->name->symbol, builder.Symbols().Get("a"));
    ast::CheckIdentifier(mem->type, "i32");
    EXPECT_EQ(mem->attributes.Length(), 0u);
}

TEST_F(WGSLParserTest, StructBodyDecl_Parses_TrailingComma) {
    auto p = parser("{a : i32,}");

    auto& builder = p->builder();

    auto m = p->expect_struct_body_decl();
    ASSERT_FALSE(p->has_error());
    ASSERT_FALSE(m.errored);
    ASSERT_EQ(m.value.Length(), 1u);

    const auto* mem = m.value[0];
    EXPECT_EQ(mem->name->symbol, builder.Symbols().Get("a"));
    ast::CheckIdentifier(mem->type, "i32");
    EXPECT_EQ(mem->attributes.Length(), 0u);
}

TEST_F(WGSLParserTest, StructBodyDecl_ParsesEmpty) {
    auto p = parser("{}");
    auto m = p->expect_struct_body_decl();
    ASSERT_FALSE(p->has_error());
    ASSERT_FALSE(m.errored);
    ASSERT_EQ(m.value.Length(), 0u);
}

TEST_F(WGSLParserTest, StructBodyDecl_InvalidAlign) {
    auto p = parser(R"(
{
  @align(if) a : i32,
})");
    auto m = p->expect_struct_body_decl();
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(m.errored);
    EXPECT_EQ(p->error(), "3:10: expected expression for align");
}

TEST_F(WGSLParserTest, StructBodyDecl_InvalidSize) {
    auto p = parser(R"(
{
  @size(if) a : i32,
})");
    auto m = p->expect_struct_body_decl();
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(m.errored);
    EXPECT_EQ(p->error(), "3:9: expected expression for size");
}

TEST_F(WGSLParserTest, StructBodyDecl_MissingClosingBracket) {
    auto p = parser("{a : i32,");
    auto m = p->expect_struct_body_decl();
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(m.errored);
    EXPECT_EQ(p->error(), "1:10: expected '}' for struct declaration");
}

TEST_F(WGSLParserTest, StructBodyDecl_InvalidToken) {
    auto p = parser(R"(
{
  a : i32,
  1.23
} )");
    auto m = p->expect_struct_body_decl();
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(m.errored);
    EXPECT_EQ(p->error(), "4:3: expected '}' for struct declaration");
}

}  // namespace
}  // namespace tint::wgsl::reader
