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

#include "src/tint/ast/test_helper.h"
#include "src/tint/reader/wgsl/parser_impl_test_helper.h"

namespace tint::reader::wgsl {
namespace {

TEST_F(ParserImplTest, TypeDecl_ParsesType) {
    auto p = parser("alias a = i32");

    auto t = p->type_alias_decl();
    EXPECT_FALSE(p->has_error());
    EXPECT_FALSE(t.errored);
    EXPECT_TRUE(t.matched);
    ASSERT_NE(t.value, nullptr);
    ASSERT_TRUE(t->Is<ast::Alias>());
    auto* alias = t->As<ast::Alias>();
    ast::CheckIdentifier(alias->type, "i32");
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 14u}}));
}

TEST_F(ParserImplTest, TypeDecl_Parses_Ident) {
    auto p = parser("alias a = B");

    auto t = p->type_alias_decl();
    EXPECT_FALSE(p->has_error());
    EXPECT_FALSE(t.errored);
    EXPECT_TRUE(t.matched);
    ASSERT_NE(t.value, nullptr);
    ASSERT_TRUE(t.value->Is<ast::Alias>());
    auto* alias = t.value->As<ast::Alias>();
    ast::CheckIdentifier(alias->name, "a");
    ast::CheckIdentifier(alias->type, "B");
    EXPECT_EQ(alias->source.range, (Source::Range{{1u, 1u}, {1u, 12u}}));
}

TEST_F(ParserImplTest, TypeDecl_Unicode_Parses_Ident) {
    const std::string ident =  // "𝓶𝔂_𝓽𝔂𝓹𝓮"
        "\xf0\x9d\x93\xb6\xf0\x9d\x94\x82\x5f\xf0\x9d\x93\xbd\xf0\x9d\x94\x82\xf0"
        "\x9d\x93\xb9\xf0\x9d\x93\xae";

    auto p = parser("alias " + ident + " = i32");

    auto t = p->type_alias_decl();
    EXPECT_FALSE(p->has_error());
    EXPECT_FALSE(t.errored);
    EXPECT_TRUE(t.matched);
    ASSERT_NE(t.value, nullptr);
    ASSERT_TRUE(t.value->Is<ast::Alias>());
    auto* alias = t.value->As<ast::Alias>();
    ast::CheckIdentifier(alias->name, ident);
    ast::CheckIdentifier(alias->type, "i32");
    EXPECT_EQ(alias->source.range, (Source::Range{{1u, 1u}, {1u, 38u}}));
}

TEST_F(ParserImplTest, TypeDecl_MissingIdent) {
    auto p = parser("alias = i32");
    auto t = p->type_alias_decl();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(t.value, nullptr);
    EXPECT_EQ(p->error(), R"(1:7: expected identifier for type alias)");
}

TEST_F(ParserImplTest, TypeDecl_InvalidIdent) {
    auto p = parser("alias 123 = i32");
    auto t = p->type_alias_decl();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(t.value, nullptr);
    EXPECT_EQ(p->error(), R"(1:7: expected identifier for type alias)");
}

TEST_F(ParserImplTest, TypeDecl_MissingEqual) {
    auto p = parser("alias a i32");
    auto t = p->type_alias_decl();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(t.value, nullptr);
    EXPECT_EQ(p->error(), R"(1:9: expected '=' for type alias)");
}

}  // namespace
}  // namespace tint::reader::wgsl
