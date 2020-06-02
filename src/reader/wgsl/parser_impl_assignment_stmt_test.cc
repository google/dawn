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

#include "gtest/gtest.h"
#include "src/ast/array_accessor_expression.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/literal.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, AssignmentStmt_Parses_ToVariable) {
  auto* p = parser("a = 123");
  auto e = p->assignment_stmt();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);

  ASSERT_TRUE(e->IsAssign());
  ASSERT_NE(e->lhs(), nullptr);
  ASSERT_NE(e->rhs(), nullptr);

  ASSERT_TRUE(e->lhs()->IsIdentifier());
  auto* ident = e->lhs()->AsIdentifier();
  EXPECT_EQ(ident->name(), "a");

  ASSERT_TRUE(e->rhs()->IsConstructor());
  ASSERT_TRUE(e->rhs()->AsConstructor()->IsScalarConstructor());

  auto* init = e->rhs()->AsConstructor()->AsScalarConstructor();
  ASSERT_NE(init->literal(), nullptr);
  ASSERT_TRUE(init->literal()->IsSint());
  EXPECT_EQ(init->literal()->AsSint()->value(), 123);
}

TEST_F(ParserImplTest, AssignmentStmt_Parses_ToMember) {
  auto* p = parser("a.b.c[2].d = 123");
  auto e = p->assignment_stmt();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);

  ASSERT_TRUE(e->IsAssign());
  ASSERT_NE(e->lhs(), nullptr);
  ASSERT_NE(e->rhs(), nullptr);

  ASSERT_TRUE(e->rhs()->IsConstructor());
  ASSERT_TRUE(e->rhs()->AsConstructor()->IsScalarConstructor());
  auto* init = e->rhs()->AsConstructor()->AsScalarConstructor();
  ASSERT_NE(init->literal(), nullptr);
  ASSERT_TRUE(init->literal()->IsSint());
  EXPECT_EQ(init->literal()->AsSint()->value(), 123);

  ASSERT_TRUE(e->lhs()->IsMemberAccessor());
  auto* mem = e->lhs()->AsMemberAccessor();

  ASSERT_TRUE(mem->member()->IsIdentifier());
  auto* ident = mem->member()->AsIdentifier();
  EXPECT_EQ(ident->name(), "d");

  ASSERT_TRUE(mem->structure()->IsArrayAccessor());
  auto* ary = mem->structure()->AsArrayAccessor();

  ASSERT_TRUE(ary->idx_expr()->IsConstructor());
  ASSERT_TRUE(ary->idx_expr()->AsConstructor()->IsScalarConstructor());
  init = ary->idx_expr()->AsConstructor()->AsScalarConstructor();
  ASSERT_NE(init->literal(), nullptr);
  ASSERT_TRUE(init->literal()->IsSint());
  EXPECT_EQ(init->literal()->AsSint()->value(), 2);

  ASSERT_TRUE(ary->array()->IsMemberAccessor());
  mem = ary->array()->AsMemberAccessor();
  ASSERT_TRUE(mem->member()->IsIdentifier());
  ident = mem->member()->AsIdentifier();
  EXPECT_EQ(ident->name(), "c");

  ASSERT_TRUE(mem->structure()->IsMemberAccessor());
  mem = mem->structure()->AsMemberAccessor();

  ASSERT_TRUE(mem->structure()->IsIdentifier());
  ident = mem->structure()->AsIdentifier();
  EXPECT_EQ(ident->name(), "a");

  ASSERT_TRUE(mem->member()->IsIdentifier());
  ident = mem->member()->AsIdentifier();
  EXPECT_EQ(ident->name(), "b");
}

TEST_F(ParserImplTest, AssignmentStmt_MissingEqual) {
  auto* p = parser("a.b.c[2].d 123");
  auto e = p->assignment_stmt();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:12: missing = for assignment");
}

TEST_F(ParserImplTest, AssignmentStmt_InvalidLHS) {
  auto* p = parser("if (true) {} = 123");
  auto e = p->assignment_stmt();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_EQ(e, nullptr);
}

TEST_F(ParserImplTest, AssignmentStmt_InvalidRHS) {
  auto* p = parser("a.b.c[2].d = if (true) {}");
  auto e = p->assignment_stmt();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:14: unable to parse right side of assignment");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
