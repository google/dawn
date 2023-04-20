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
TEST_F(ParserImplTest, VariableDecl_Parses) {
    auto p = parser("var my_var : f32");
    auto v = p->variable_decl();
    EXPECT_FALSE(p->has_error());
    EXPECT_TRUE(v.matched);
    EXPECT_FALSE(v.errored);
    ast::CheckIdentifier(v->name, "my_var");
    EXPECT_NE(v->type, nullptr);

    ast::CheckIdentifier(v->type, "f32");

    EXPECT_EQ(v->source.range, (Source::Range{{1u, 5u}, {1u, 11u}}));
    EXPECT_EQ(v->type->source.range, (Source::Range{{1u, 14u}, {1u, 17u}}));
}

TEST_F(ParserImplTest, VariableDecl_Unicode_Parses) {
    const std::string ident =  // "𝖎𝖉𝖊𝖓𝖙𝖎𝖋𝖎𝖊𝖗123"
        "\xf0\x9d\x96\x8e\xf0\x9d\x96\x89\xf0\x9d\x96\x8a\xf0\x9d\x96\x93"
        "\xf0\x9d\x96\x99\xf0\x9d\x96\x8e\xf0\x9d\x96\x8b\xf0\x9d\x96\x8e"
        "\xf0\x9d\x96\x8a\xf0\x9d\x96\x97\x31\x32\x33";

    auto p = parser("var " + ident + " : f32");
    auto v = p->variable_decl();
    EXPECT_FALSE(p->has_error());
    EXPECT_TRUE(v.matched);
    EXPECT_FALSE(v.errored);
    ast::CheckIdentifier(v->name, ident);
    EXPECT_NE(v->type, nullptr);

    ast::CheckIdentifier(v->type, "f32");

    EXPECT_EQ(v->source.range, (Source::Range{{1u, 5u}, {1u, 48u}}));
    EXPECT_EQ(v->type->source.range, (Source::Range{{1u, 51u}, {1u, 54u}}));
}

TEST_F(ParserImplTest, VariableDecl_Inferred_Parses) {
    auto p = parser("var my_var = 1.0");
    auto v = p->variable_decl();
    EXPECT_FALSE(p->has_error());
    EXPECT_TRUE(v.matched);
    EXPECT_FALSE(v.errored);
    ast::CheckIdentifier(v->name, "my_var");
    EXPECT_EQ(v->type, nullptr);

    EXPECT_EQ(v->source.range, (Source::Range{{1u, 5u}, {1u, 11u}}));
}

TEST_F(ParserImplTest, VariableDecl_MissingVar) {
    auto p = parser("my_var : f32");
    auto v = p->variable_decl();
    EXPECT_FALSE(v.matched);
    EXPECT_FALSE(v.errored);
    EXPECT_FALSE(p->has_error());

    auto& t = p->next();
    ASSERT_TRUE(t.IsIdentifier());
}

TEST_F(ParserImplTest, VariableDecl_WithAddressSpace) {
    auto p = parser("var<private> my_var : f32");
    auto v = p->variable_decl();
    EXPECT_TRUE(v.matched);
    EXPECT_FALSE(v.errored);
    EXPECT_FALSE(p->has_error());
    ast::CheckIdentifier(v->name, "my_var");

    ast::CheckIdentifier(v->type, "f32");
    ast::CheckIdentifier(v->address_space, "private");

    EXPECT_EQ(v->source.range.begin.line, 1u);
    EXPECT_EQ(v->source.range.begin.column, 14u);
    EXPECT_EQ(v->source.range.end.line, 1u);
    EXPECT_EQ(v->source.range.end.column, 20u);
}

TEST_F(ParserImplTest, VariableDecl_WithPushConstant) {
    auto p = parser("var<push_constant> my_var : f32");
    auto v = p->variable_decl();
    EXPECT_TRUE(v.matched);
    EXPECT_FALSE(v.errored);
    EXPECT_FALSE(p->has_error());
    ast::CheckIdentifier(v->name, "my_var");

    ast::CheckIdentifier(v->type, "f32");
    ast::CheckIdentifier(v->address_space, "push_constant");
}

}  // namespace
}  // namespace tint::reader::wgsl
