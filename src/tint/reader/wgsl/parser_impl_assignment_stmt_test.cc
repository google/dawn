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

namespace tint::reader::wgsl {
namespace {

TEST_F(ParserImplTest, AssignmentStmt_Parses_ToVariable) {
    auto p = parser("a = 123");
    auto e = p->variable_updating_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    auto* a = e->As<ast::AssignmentStatement>();
    ASSERT_NE(a, nullptr);
    ASSERT_NE(a->lhs, nullptr);
    ASSERT_NE(a->rhs, nullptr);

    EXPECT_EQ(a->source.range.begin.line, 1u);
    EXPECT_EQ(a->source.range.begin.column, 3u);
    EXPECT_EQ(a->source.range.end.line, 1u);
    EXPECT_EQ(a->source.range.end.column, 4u);

    ASSERT_TRUE(a->lhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = a->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(a->rhs->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(a->rhs->As<ast::IntLiteralExpression>()->value, 123);
    EXPECT_EQ(a->rhs->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(ParserImplTest, AssignmentStmt_Parses_ToMember) {
    auto p = parser("a.b.c[2].d = 123");
    auto e = p->variable_updating_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    auto* a = e->As<ast::AssignmentStatement>();
    ASSERT_NE(a, nullptr);
    ASSERT_NE(a->lhs, nullptr);
    ASSERT_NE(a->rhs, nullptr);

    EXPECT_EQ(a->source.range.begin.line, 1u);
    EXPECT_EQ(a->source.range.begin.column, 12u);
    EXPECT_EQ(a->source.range.end.line, 1u);
    EXPECT_EQ(a->source.range.end.column, 13u);

    ASSERT_TRUE(a->rhs->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(a->rhs->As<ast::IntLiteralExpression>()->value, 123);
    EXPECT_EQ(a->rhs->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    ASSERT_TRUE(a->lhs->Is<ast::MemberAccessorExpression>());
    auto* mem = a->lhs->As<ast::MemberAccessorExpression>();

    EXPECT_EQ(mem->member->symbol, p->builder().Symbols().Get("d"));

    ASSERT_TRUE(mem->object->Is<ast::IndexAccessorExpression>());
    auto* idx = mem->object->As<ast::IndexAccessorExpression>();

    ASSERT_NE(idx->index, nullptr);
    ASSERT_TRUE(idx->index->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(idx->index->As<ast::IntLiteralExpression>()->value, 2);

    ASSERT_TRUE(idx->object->Is<ast::MemberAccessorExpression>());
    mem = idx->object->As<ast::MemberAccessorExpression>();
    EXPECT_EQ(mem->member->symbol, p->builder().Symbols().Get("c"));

    ASSERT_TRUE(mem->object->Is<ast::MemberAccessorExpression>());

    mem = mem->object->As<ast::MemberAccessorExpression>();
    ASSERT_TRUE(mem->object->Is<ast::IdentifierExpression>());
    auto* ident_expr = mem->object->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));
    EXPECT_EQ(mem->member->symbol, p->builder().Symbols().Get("b"));
}

TEST_F(ParserImplTest, AssignmentStmt_Parses_ToPhony) {
    auto p = parser("_ = 123i");
    auto e = p->variable_updating_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    auto* a = e->As<ast::AssignmentStatement>();
    ASSERT_NE(a, nullptr);
    ASSERT_NE(a->lhs, nullptr);
    ASSERT_NE(a->rhs, nullptr);

    EXPECT_EQ(a->source.range.begin.line, 1u);
    EXPECT_EQ(a->source.range.begin.column, 3u);
    EXPECT_EQ(a->source.range.end.line, 1u);
    EXPECT_EQ(a->source.range.end.column, 4u);

    ASSERT_TRUE(a->rhs->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(a->rhs->As<ast::IntLiteralExpression>()->value, 123);
    EXPECT_EQ(a->rhs->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kI);

    ASSERT_TRUE(a->lhs->Is<ast::PhonyExpression>());
}

TEST_F(ParserImplTest, AssignmentStmt_Phony_CompoundOpFails) {
    auto p = parser("_ += 123i");
    auto e = p->variable_updating_statement();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:3: expected '=' for assignment");
}

TEST_F(ParserImplTest, AssignmentStmt_Phony_IncrementFails) {
    auto p = parser("_ ++");
    auto e = p->variable_updating_statement();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:3: expected '=' for assignment");
}

TEST_F(ParserImplTest, AssignmentStmt_Phony_EqualIncrementFails) {
    auto p = parser("_ = ++");
    auto e = p->variable_updating_statement();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(
        p->error(),
        "1:5: prefix increment and decrement operators are reserved for a future WGSL version");
}

struct CompoundData {
    std::string str;
    ast::BinaryOp op;
};
using CompoundOpTest = ParserImplTestWithParam<CompoundData>;
TEST_P(CompoundOpTest, CompoundOp) {
    auto params = GetParam();
    auto p = parser("a " + params.str + " 123u");
    auto e = p->variable_updating_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    auto* a = e->As<ast::CompoundAssignmentStatement>();
    ASSERT_NE(a, nullptr);
    ASSERT_NE(a->lhs, nullptr);
    ASSERT_NE(a->rhs, nullptr);
    EXPECT_EQ(a->op, params.op);

    EXPECT_EQ(a->source.range.begin.line, 1u);
    EXPECT_EQ(a->source.range.begin.column, 3u);
    EXPECT_EQ(a->source.range.end.line, 1u);
    EXPECT_EQ(a->source.range.end.column, 3u + params.str.length());

    ASSERT_TRUE(a->lhs->Is<ast::IdentifierExpression>());
    auto* ident_expr = a->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident_expr->identifier->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(a->rhs->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(a->rhs->As<ast::IntLiteralExpression>()->value, 123);
    EXPECT_EQ(a->rhs->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kU);
}
INSTANTIATE_TEST_SUITE_P(ParserImplTest,
                         CompoundOpTest,
                         testing::Values(CompoundData{"+=", ast::BinaryOp::kAdd},
                                         CompoundData{"-=", ast::BinaryOp::kSubtract},
                                         CompoundData{"*=", ast::BinaryOp::kMultiply},
                                         CompoundData{"/=", ast::BinaryOp::kDivide},
                                         CompoundData{"%=", ast::BinaryOp::kModulo},
                                         CompoundData{"&=", ast::BinaryOp::kAnd},
                                         CompoundData{"|=", ast::BinaryOp::kOr},
                                         CompoundData{"^=", ast::BinaryOp::kXor},
                                         CompoundData{">>=", ast::BinaryOp::kShiftRight},
                                         CompoundData{"<<=", ast::BinaryOp::kShiftLeft}));

TEST_F(ParserImplTest, AssignmentStmt_MissingEqual) {
    auto p = parser("a.b.c[2].d 123");
    auto e = p->variable_updating_statement();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:12: expected '=' for assignment");
}

TEST_F(ParserImplTest, AssignmentStmt_Compound_MissingEqual) {
    auto p = parser("a + 123");
    auto e = p->variable_updating_statement();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(e.value, nullptr);
    EXPECT_EQ(p->error(), "1:3: expected '=' for assignment");
}

TEST_F(ParserImplTest, AssignmentStmt_InvalidLHS) {
    auto p = parser("if (true) {} = 123");
    auto e = p->variable_updating_statement();
    EXPECT_FALSE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_EQ(e.value, nullptr);
}

TEST_F(ParserImplTest, AssignmentStmt_InvalidRHS) {
    auto p = parser("a.b.c[2].d = if (true) {}");
    auto e = p->variable_updating_statement();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:14: unable to parse right side of assignment");
}

TEST_F(ParserImplTest, AssignmentStmt_InvalidCompoundOp) {
    auto p = parser("a &&= true");
    auto e = p->variable_updating_statement();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:3: expected '=' for assignment");
}

}  // namespace
}  // namespace tint::reader::wgsl
