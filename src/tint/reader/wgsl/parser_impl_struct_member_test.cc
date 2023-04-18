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

TEST_F(ParserImplTest, StructMember_Parses) {
    auto p = parser("a : i32,");

    auto m = p->expect_struct_member();
    ASSERT_FALSE(p->has_error());
    ASSERT_FALSE(m.errored);
    ASSERT_NE(m.value, nullptr);

    ast::CheckIdentifier(m->name, "a");
    ast::CheckIdentifier(m->type, "i32");
    EXPECT_EQ(m->attributes.Length(), 0u);

    EXPECT_EQ(m->source.range, (Source::Range{{1u, 1u}, {1u, 2u}}));
    EXPECT_EQ(m->type->source.range, (Source::Range{{1u, 5u}, {1u, 8u}}));
}

TEST_F(ParserImplTest, StructMember_ParsesWithAlignAttribute) {
    auto p = parser("@align(2) a : i32,");

    auto m = p->expect_struct_member();
    ASSERT_FALSE(p->has_error());
    ASSERT_FALSE(m.errored);
    ASSERT_NE(m.value, nullptr);

    ast::CheckIdentifier(m->name, "a");
    ast::CheckIdentifier(m->type, "i32");
    EXPECT_EQ(m->attributes.Length(), 1u);
    EXPECT_TRUE(m->attributes[0]->Is<ast::StructMemberAlignAttribute>());

    auto* attr = m->attributes[0]->As<ast::StructMemberAlignAttribute>();
    ASSERT_TRUE(attr->expr->Is<ast::IntLiteralExpression>());
    auto* expr = attr->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(expr->value, 2);
    EXPECT_EQ(expr->suffix, ast::IntLiteralExpression::Suffix::kNone);

    EXPECT_EQ(m->source.range, (Source::Range{{1u, 11u}, {1u, 12u}}));
    EXPECT_EQ(m->type->source.range, (Source::Range{{1u, 15u}, {1u, 18u}}));
}

TEST_F(ParserImplTest, StructMember_ParsesWithSizeAttribute) {
    auto p = parser("@size(2) a : i32,");

    auto m = p->expect_struct_member();
    ASSERT_FALSE(p->has_error());
    ASSERT_FALSE(m.errored);
    ASSERT_NE(m.value, nullptr);

    ast::CheckIdentifier(m->name, "a");
    ast::CheckIdentifier(m->type, "i32");
    EXPECT_EQ(m->attributes.Length(), 1u);
    ASSERT_TRUE(m->attributes[0]->Is<ast::StructMemberSizeAttribute>());
    auto* s = m->attributes[0]->As<ast::StructMemberSizeAttribute>();

    ASSERT_TRUE(s->expr->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(s->expr->As<ast::IntLiteralExpression>()->value, 2u);

    EXPECT_EQ(m->source.range, (Source::Range{{1u, 10u}, {1u, 11u}}));
    EXPECT_EQ(m->type->source.range, (Source::Range{{1u, 14u}, {1u, 17u}}));
}

TEST_F(ParserImplTest, StructMember_ParsesWithMultipleattributes) {
    auto p = parser(R"(@size(2)
@align(4) a : i32,)");

    auto m = p->expect_struct_member();
    ASSERT_FALSE(p->has_error());
    ASSERT_FALSE(m.errored);
    ASSERT_NE(m.value, nullptr);

    ast::CheckIdentifier(m->name, "a");
    ast::CheckIdentifier(m->type, "i32");
    EXPECT_EQ(m->attributes.Length(), 2u);
    ASSERT_TRUE(m->attributes[0]->Is<ast::StructMemberSizeAttribute>());
    auto* size_attr = m->attributes[0]->As<ast::StructMemberSizeAttribute>();
    ASSERT_TRUE(size_attr->expr->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(size_attr->expr->As<ast::IntLiteralExpression>()->value, 2u);

    ASSERT_TRUE(m->attributes[1]->Is<ast::StructMemberAlignAttribute>());
    auto* attr = m->attributes[1]->As<ast::StructMemberAlignAttribute>();

    ASSERT_TRUE(attr->expr->Is<ast::IntLiteralExpression>());
    auto* expr = attr->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(expr->value, 4);
    EXPECT_EQ(expr->suffix, ast::IntLiteralExpression::Suffix::kNone);

    EXPECT_EQ(m->source.range, (Source::Range{{2u, 11u}, {2u, 12u}}));
    EXPECT_EQ(m->type->source.range, (Source::Range{{2u, 15u}, {2u, 18u}}));
}

TEST_F(ParserImplTest, StructMember_InvalidAttribute) {
    auto p = parser("@size(if) a : i32,");

    auto m = p->expect_struct_member();
    ASSERT_TRUE(m.errored);
    ASSERT_EQ(m.value, nullptr);

    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:7: expected expression for size");
}

}  // namespace
}  // namespace tint::reader::wgsl
