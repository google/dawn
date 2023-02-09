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

#include "src/tint/reader/wgsl/parser_impl_test_helper.h"
#include "src/tint/utils/string.h"

namespace tint::reader::wgsl {
namespace {

TEST_F(ParserImplTest, StructDecl_Parses) {
    auto p = parser(R"(
struct S {
  a : i32,
  b : f32,
})");
    auto s = p->struct_decl();
    EXPECT_FALSE(p->has_error());
    EXPECT_FALSE(s.errored);
    EXPECT_TRUE(s.matched);
    ASSERT_NE(s.value, nullptr);
    ASSERT_EQ(s->name->symbol, p->builder().Symbols().Register("S"));
    ASSERT_EQ(s->members.Length(), 2u);
    EXPECT_EQ(s->members[0]->name->symbol, p->builder().Symbols().Register("a"));
    EXPECT_EQ(s->members[1]->name->symbol, p->builder().Symbols().Register("b"));
}

TEST_F(ParserImplTest, StructDecl_Unicode_Parses) {
    const std::string struct_ident =  // "𝓼𝓽𝓻𝓾𝓬𝓽𝓾𝓻𝓮"
        "\xf0\x9d\x93\xbc\xf0\x9d\x93\xbd\xf0\x9d\x93\xbb\xf0\x9d\x93\xbe\xf0\x9d"
        "\x93\xac\xf0\x9d\x93\xbd\xf0\x9d\x93\xbe\xf0\x9d\x93\xbb\xf0\x9d\x93"
        "\xae";
    const std::string member_a_ident =  // "𝕞𝕖𝕞𝕓𝕖𝕣_𝕒"
        "\xf0\x9d\x95\x9e\xf0\x9d\x95\x96\xf0\x9d\x95\x9e\xf0\x9d\x95\x93\xf0\x9d"
        "\x95\x96\xf0\x9d\x95\xa3\x5f\xf0\x9d\x95\x92";
    const std::string member_b_ident =  // "𝔪𝔢𝔪𝔟𝔢𝔯_𝔟"
        "\xf0\x9d\x94\xaa\xf0\x9d\x94\xa2\xf0\x9d\x94\xaa\xf0\x9d\x94\x9f\xf0\x9d"
        "\x94\xa2\xf0\x9d\x94\xaf\x5f\xf0\x9d\x94\x9f";

    std::string src = R"(
struct $struct {
  $member_a : i32,
  $member_b : f32,
})";
    src = utils::ReplaceAll(src, "$struct", struct_ident);
    src = utils::ReplaceAll(src, "$member_a", member_a_ident);
    src = utils::ReplaceAll(src, "$member_b", member_b_ident);

    auto p = parser(src);

    auto s = p->struct_decl();
    EXPECT_FALSE(p->has_error());
    EXPECT_FALSE(s.errored);
    EXPECT_TRUE(s.matched);
    ASSERT_NE(s.value, nullptr);
    ASSERT_EQ(s->name->symbol, p->builder().Symbols().Register(struct_ident));
    ASSERT_EQ(s->members.Length(), 2u);
    EXPECT_EQ(s->members[0]->name->symbol, p->builder().Symbols().Register(member_a_ident));
    EXPECT_EQ(s->members[1]->name->symbol, p->builder().Symbols().Register(member_b_ident));
}

TEST_F(ParserImplTest, StructDecl_EmptyMembers) {
    auto p = parser("struct S {}");

    auto s = p->struct_decl();
    EXPECT_FALSE(p->has_error());
    EXPECT_FALSE(s.errored);
    EXPECT_TRUE(s.matched);
    ASSERT_NE(s.value, nullptr);
    ASSERT_EQ(s->members.Length(), 0u);
}

TEST_F(ParserImplTest, StructDecl_MissingIdent) {
    auto p = parser("struct {}");

    auto s = p->struct_decl();
    EXPECT_TRUE(s.errored);
    EXPECT_FALSE(s.matched);
    EXPECT_EQ(s.value, nullptr);

    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:8: expected identifier for struct declaration");
}

TEST_F(ParserImplTest, StructDecl_MissingBracketLeft) {
    auto p = parser("struct S }");

    auto s = p->struct_decl();
    EXPECT_TRUE(s.errored);
    EXPECT_FALSE(s.matched);
    EXPECT_EQ(s.value, nullptr);

    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:10: expected '{' for struct declaration");
}

}  // namespace
}  // namespace tint::reader::wgsl
