// Copyright 2020 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/wgsl/ast/helper_test.h"
#include "src/tint/lang/wgsl/ast/interpolated_string_expression.h"
#include "src/tint/lang/wgsl/ast/string_literal_expression.h"
#include "src/tint/lang/wgsl/reader/parser/helper_test.h"

namespace tint::wgsl::reader {
namespace {

TEST_F(WGSLParserTest, PrimaryExpression_Ident) {
    auto p = parser("a");
    auto e = p->primary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::IdentifierExpression>());
    ast::CheckIdentifier(e.value, "a");
}

TEST_F(WGSLParserTest, PrimaryExpression_TypeDecl) {
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

TEST_F(WGSLParserTest, PrimaryExpression_TypeDecl_ZeroInitializer) {
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

TEST_F(WGSLParserTest, PrimaryExpression_TypeDecl_MissingRightParen) {
    auto p = parser("vec4<f32>(2., 3., 4., 5.");
    auto e = p->primary_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:25: expected ',' for function call");
}

TEST_F(WGSLParserTest, PrimaryExpression_TypeDecl_InvalidValue) {
    auto p = parser("i32(if(a) {})");
    auto e = p->primary_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:5: expected expression for function call");
}

TEST_F(WGSLParserTest, PrimaryExpression_TypeDecl_StructInitializer_Empty) {
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

TEST_F(WGSLParserTest, PrimaryExpression_TypeDecl_StructInitializer_NotEmpty) {
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
    EXPECT_EQ(call->args[1]->As<ast::FloatLiteralExpression>()->value, 2.0);
}

TEST_F(WGSLParserTest, PrimaryExpression_ConstLiteral_True) {
    auto p = parser("true");
    auto e = p->primary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::BoolLiteralExpression>());
    EXPECT_TRUE(e->As<ast::BoolLiteralExpression>()->value);
}

TEST_F(WGSLParserTest, PrimaryExpression_ParenExpr) {
    auto p = parser("(a == b)");
    auto e = p->primary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
}

TEST_F(WGSLParserTest, PrimaryExpression_ParenExpr_MissingRightParen) {
    auto p = parser("(a == b");
    auto e = p->primary_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:8: expected ')'");
}

TEST_F(WGSLParserTest, PrimaryExpression_ParenExpr_MissingExpr) {
    auto p = parser("()");
    auto e = p->primary_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:2: unable to parse expression");
}

TEST_F(WGSLParserTest, PrimaryExpression_ParenExpr_InvalidExpr) {
    auto p = parser("(if (a) {})");
    auto e = p->primary_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:2: unable to parse expression");
}

TEST_F(WGSLParserTest, PrimaryExpression_Cast) {
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

TEST_F(WGSLParserTest, PrimaryExpression_StringLiteral) {
    auto p = parser("`hello world`");
    auto e = p->primary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::StringLiteralExpression>());
    EXPECT_EQ(e->As<ast::StringLiteralExpression>()->value, "hello world");
}

TEST_F(WGSLParserTest, PrimaryExpression_StringLiteral_Empty) {
    auto p = parser("``");
    auto e = p->primary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::StringLiteralExpression>());
    EXPECT_EQ(e->As<ast::StringLiteralExpression>()->value, "");
}

TEST_F(WGSLParserTest, PrimaryExpression_InterpolatedString_SingleInterpolation) {
    auto p = parser("`hello ${name}!`");
    auto e = p->primary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::InterpolatedStringExpression>());
    auto* tmpl = e->As<ast::InterpolatedStringExpression>();
    ASSERT_EQ(tmpl->elements.Length(), 3u);
    ASSERT_TRUE(tmpl->elements[0]->Is<ast::StringLiteralExpression>());
    EXPECT_EQ(tmpl->elements[0]->As<ast::StringLiteralExpression>()->value, "hello ");
    ASSERT_TRUE(tmpl->elements[1]->Is<ast::IdentifierExpression>());
    ast::CheckIdentifier(tmpl->elements[1], "name");
    ASSERT_TRUE(tmpl->elements[2]->Is<ast::StringLiteralExpression>());
    EXPECT_EQ(tmpl->elements[2]->As<ast::StringLiteralExpression>()->value, "!");
}

TEST_F(WGSLParserTest, PrimaryExpression_InterpolatedString_EmptyHeadAndTail) {
    auto p = parser("`${x}`");
    auto e = p->primary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::InterpolatedStringExpression>());
    auto* tmpl = e->As<ast::InterpolatedStringExpression>();
    ASSERT_EQ(tmpl->elements.Length(), 1u);
    ASSERT_TRUE(tmpl->elements[0]->Is<ast::IdentifierExpression>());
    ast::CheckIdentifier(tmpl->elements[0], "x");
}

TEST_F(WGSLParserTest, PrimaryExpression_InterpolatedString_MultipleInterpolations) {
    auto p = parser("`x: ${x}, y: ${y + 1}`");
    auto e = p->primary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::InterpolatedStringExpression>());
    auto* tmpl = e->As<ast::InterpolatedStringExpression>();
    ASSERT_EQ(tmpl->elements.Length(), 4u);
    ASSERT_TRUE(tmpl->elements[0]->Is<ast::StringLiteralExpression>());
    EXPECT_EQ(tmpl->elements[0]->As<ast::StringLiteralExpression>()->value, "x: ");
    ASSERT_TRUE(tmpl->elements[1]->Is<ast::IdentifierExpression>());
    ast::CheckIdentifier(tmpl->elements[1], "x");
    ASSERT_TRUE(tmpl->elements[2]->Is<ast::StringLiteralExpression>());
    EXPECT_EQ(tmpl->elements[2]->As<ast::StringLiteralExpression>()->value, ", y: ");
    ASSERT_TRUE(tmpl->elements[3]->Is<ast::BinaryExpression>());
}

TEST_F(WGSLParserTest, PrimaryExpression_InterpolatedString_ConsecutiveInterpolations) {
    auto p = parser("`${a}${b}`");
    auto e = p->primary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::InterpolatedStringExpression>());
    auto* tmpl = e->As<ast::InterpolatedStringExpression>();
    ASSERT_EQ(tmpl->elements.Length(), 2u);
    ASSERT_TRUE(tmpl->elements[0]->Is<ast::IdentifierExpression>());
    ast::CheckIdentifier(tmpl->elements[0], "a");
    ASSERT_TRUE(tmpl->elements[1]->Is<ast::IdentifierExpression>());
    ast::CheckIdentifier(tmpl->elements[1], "b");
}

TEST_F(WGSLParserTest, PrimaryExpression_InterpolatedString_Nested) {
    auto p = parser("`outer: ${`inner: ${x}`}`");
    auto e = p->primary_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);
    ASSERT_TRUE(e->Is<ast::InterpolatedStringExpression>());
    auto* tmpl = e->As<ast::InterpolatedStringExpression>();
    ASSERT_EQ(tmpl->elements.Length(), 2u);
    ASSERT_TRUE(tmpl->elements[0]->Is<ast::StringLiteralExpression>());
    EXPECT_EQ(tmpl->elements[0]->As<ast::StringLiteralExpression>()->value, "outer: ");
    ASSERT_TRUE(tmpl->elements[1]->Is<ast::InterpolatedStringExpression>());
    auto* inner = tmpl->elements[1]->As<ast::InterpolatedStringExpression>();
    ASSERT_EQ(inner->elements.Length(), 2u);
    ASSERT_TRUE(inner->elements[0]->Is<ast::StringLiteralExpression>());
    EXPECT_EQ(inner->elements[0]->As<ast::StringLiteralExpression>()->value, "inner: ");
    ASSERT_TRUE(inner->elements[1]->Is<ast::IdentifierExpression>());
    ast::CheckIdentifier(inner->elements[1], "x");
}

TEST_F(WGSLParserTest, PrimaryExpression_InterpolatedString_MissingExpression) {
    auto p = parser("`hello ${} world`");
    auto e = p->primary_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:10: expected expression for interpolated string");
}

TEST_F(WGSLParserTest, PrimaryExpression_InterpolatedString_MissingClosingBrace) {
    auto p = parser("`hello ${x y`");
    auto e = p->primary_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:12: expected '}' for interpolated string");
}

TEST_F(WGSLParserTest, PrimaryExpression_InterpolatedString_Unterminated) {
    auto p = parser("`hello ${x ");
    auto e = p->primary_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    ASSERT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:1: unterminated interpolated string literal");
}

}  // namespace
}  // namespace tint::wgsl::reader
