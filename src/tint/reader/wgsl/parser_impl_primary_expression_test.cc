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

#include "src/tint/ast/bitcast_expression.h"
#include "src/tint/ast/test_helper.h"
#include "src/tint/reader/wgsl/parser_impl_test_helper.h"

namespace tint::reader::wgsl {
namespace {

TEST_F(ParserImplTest, PrimaryExpression_Ident) {
    auto p = parser("a");
    auto e = p->primary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::IdentifierExpression>());
    ast::CheckIdentifier(e.value, "a");
}

TEST_F(ParserImplTest, PrimaryExpression_TypeDecl) {
    auto p = parser("vec4<i32>(1, 2, 3, 4))");
    auto e = p->primary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::CallExpression>());
    auto* call = e->As<ast::CallExpression>();

    ASSERT_EQ(call->args.Length(), 4u);
    const auto& val = call->args;
    ASSERT_TRUE(val[0]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(val[0]->As<ast::IntLiteralExpression>()->value, 1);
    EXPECT_EQ(val[0]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    ASSERT_TRUE(val[1]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(val[1]->As<ast::IntLiteralExpression>()->value, 2);
    EXPECT_EQ(val[1]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    ASSERT_TRUE(val[2]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(val[2]->As<ast::IntLiteralExpression>()->value, 3);
    EXPECT_EQ(val[2]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    ASSERT_TRUE(val[3]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(val[3]->As<ast::IntLiteralExpression>()->value, 4);
    EXPECT_EQ(val[3]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(ParserImplTest, PrimaryExpression_TypeDecl_ZeroInitializer) {
    auto p = parser("vec4<i32>()");
    auto e = p->primary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::CallExpression>());
    auto* call = e->As<ast::CallExpression>();

    ASSERT_EQ(call->args.Length(), 0u);
}

TEST_F(ParserImplTest, PrimaryExpression_TypeDecl_MissingRightParen) {
    auto p = parser("vec4<f32>(2., 3., 4., 5.");
    auto e = p->primary_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:25: expected ')' for function call");
}

TEST_F(ParserImplTest, PrimaryExpression_TypeDecl_InvalidValue) {
    auto p = parser("i32(if(a) {})");
    auto e = p->primary_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:5: expected ')' for function call");
}

TEST_F(ParserImplTest, PrimaryExpression_TypeDecl_StructInitializer_Empty) {
    auto p = parser(R"(
  struct S { a : i32, b : f32, }
  S()
  )");

    p->global_decl();
    ASSERT_FALSE(p->has_error()) << p->error();

    auto e = p->primary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::CallExpression>());
    auto* call = e->As<ast::CallExpression>();

    ASSERT_NE(call->target, nullptr);
    ast::CheckIdentifier(call->target, "S");

    ASSERT_EQ(call->args.Length(), 0u);
}

TEST_F(ParserImplTest, PrimaryExpression_TypeDecl_StructInitializer_NotEmpty) {
    auto p = parser(R"(
  struct S { a : i32, b : f32, }
  S(1u, 2.0)
  )");

    p->global_decl();
    ASSERT_FALSE(p->has_error()) << p->error();

    auto e = p->primary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::CallExpression>());
    auto* call = e->As<ast::CallExpression>();

    ASSERT_NE(call->target, nullptr);
    ast::CheckIdentifier(call->target, "S");

    ASSERT_EQ(call->args.Length(), 2u);

    ASSERT_TRUE(call->args[0]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(call->args[0]->As<ast::IntLiteralExpression>()->value, 1u);
    EXPECT_EQ(call->args[0]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kU);

    ASSERT_TRUE(call->args[1]->Is<ast::FloatLiteralExpression>());
    EXPECT_EQ(call->args[1]->As<ast::FloatLiteralExpression>()->value, 2.f);
}

TEST_F(ParserImplTest, PrimaryExpression_ConstLiteral_True) {
    auto p = parser("true");
    auto e = p->primary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::BoolLiteralExpression>());
    EXPECT_TRUE(e->As<ast::BoolLiteralExpression>()->value);
}

TEST_F(ParserImplTest, PrimaryExpression_ParenExpr) {
    auto p = parser("(a == b)");
    auto e = p->primary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
}

TEST_F(ParserImplTest, PrimaryExpression_ParenExpr_MissingRightParen) {
    auto p = parser("(a == b");
    auto e = p->primary_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:8: expected ')'");
}

TEST_F(ParserImplTest, PrimaryExpression_ParenExpr_MissingExpr) {
    auto p = parser("()");
    auto e = p->primary_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:2: unable to parse expression");
}

TEST_F(ParserImplTest, PrimaryExpression_ParenExpr_InvalidExpr) {
    auto p = parser("(if (a) {})");
    auto e = p->primary_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:2: unable to parse expression");
}

TEST_F(ParserImplTest, PrimaryExpression_Cast) {
    auto p = parser("f32(1)");

    auto e = p->primary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::CallExpression>());
    auto* call = e->As<ast::CallExpression>();
    ast::CheckIdentifier(call->target, "f32");

    ASSERT_EQ(call->args.Length(), 1u);
    ASSERT_TRUE(call->args[0]->Is<ast::IntLiteralExpression>());
}

TEST_F(ParserImplTest, PrimaryExpression_Bitcast) {
    auto p = parser("bitcast<f32>(1)");

    auto e = p->primary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::BitcastExpression>());

    auto* c = e->As<ast::BitcastExpression>();

    ast::CheckIdentifier(c->type, "f32");

    ASSERT_TRUE(c->expr->Is<ast::IntLiteralExpression>());
}

TEST_F(ParserImplTest, PrimaryExpression_Bitcast_MissingGreaterThan) {
    auto p = parser("bitcast<f32(1)");
    auto e = p->primary_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:8: missing closing '>' for bitcast expression");
}

TEST_F(ParserImplTest, PrimaryExpression_Bitcast_MissingType) {
    auto p = parser("bitcast<>(1)");
    auto e = p->primary_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:9: invalid type for bitcast expression");
}

TEST_F(ParserImplTest, PrimaryExpression_Bitcast_MissingLeftParen) {
    auto p = parser("bitcast<f32>1)");
    auto e = p->primary_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:13: expected '('");
}

TEST_F(ParserImplTest, PrimaryExpression_Bitcast_MissingRightParen) {
    auto p = parser("bitcast<f32>(1");
    auto e = p->primary_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:15: expected ')'");
}

TEST_F(ParserImplTest, PrimaryExpression_Bitcast_MissingExpression) {
    auto p = parser("bitcast<f32>()");
    auto e = p->primary_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:14: unable to parse expression");
}

TEST_F(ParserImplTest, PrimaryExpression_bitcast_InvalidExpression) {
    auto p = parser("bitcast<f32>(if (a) {})");
    auto e = p->primary_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:14: unable to parse expression");
}

}  // namespace
}  // namespace tint::reader::wgsl
