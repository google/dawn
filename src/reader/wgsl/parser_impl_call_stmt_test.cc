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

#include "src/ast/call_statement.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, Statement_Call) {
  auto p = parser("a();");
  auto e = p->statement();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);

  ASSERT_TRUE(e->Is<ast::CallStatement>());
  auto* c = e->As<ast::CallStatement>()->expr();

  ASSERT_TRUE(c->func()->Is<ast::IdentifierExpression>());
  auto* ident = c->func()->As<ast::IdentifierExpression>();
  EXPECT_EQ(ident->symbol(), p->builder().Symbols().Get("a"));

  EXPECT_EQ(c->params().size(), 0u);
}

TEST_F(ParserImplTest, Statement_Call_WithParams) {
  auto p = parser("a(1, b, 2 + 3 / b);");
  auto e = p->statement();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);

  ASSERT_TRUE(e->Is<ast::CallStatement>());
  auto* c = e->As<ast::CallStatement>()->expr();

  ASSERT_TRUE(c->func()->Is<ast::IdentifierExpression>());
  auto* ident = c->func()->As<ast::IdentifierExpression>();
  EXPECT_EQ(ident->symbol(), p->builder().Symbols().Get("a"));

  EXPECT_EQ(c->params().size(), 3u);
  EXPECT_TRUE(c->params()[0]->Is<ast::ConstructorExpression>());
  EXPECT_TRUE(c->params()[1]->Is<ast::IdentifierExpression>());
  EXPECT_TRUE(c->params()[2]->Is<ast::BinaryExpression>());
}

TEST_F(ParserImplTest, Statement_Call_WithParams_TrailingComma) {
  auto p = parser("a(1, b,);");
  auto e = p->statement();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e.value, nullptr);
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);

  ASSERT_TRUE(e->Is<ast::CallStatement>());
  auto* c = e->As<ast::CallStatement>()->expr();

  ASSERT_TRUE(c->func()->Is<ast::IdentifierExpression>());
  auto* ident = c->func()->As<ast::IdentifierExpression>();
  EXPECT_EQ(ident->symbol(), p->builder().Symbols().Get("a"));

  EXPECT_EQ(c->params().size(), 2u);
  EXPECT_TRUE(c->params()[0]->Is<ast::ConstructorExpression>());
  EXPECT_TRUE(c->params()[1]->Is<ast::IdentifierExpression>());
}

TEST_F(ParserImplTest, Statement_Call_Missing_RightParen) {
  auto p = parser("a(");
  auto e = p->statement();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(e.errored);
  EXPECT_FALSE(e.matched);
  EXPECT_EQ(p->error(), "1:3: expected ')' for function call");
}

TEST_F(ParserImplTest, Statement_Call_Missing_Semi) {
  auto p = parser("a()");
  auto e = p->statement();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(e.errored);
  EXPECT_FALSE(e.matched);
  EXPECT_EQ(p->error(), "1:4: expected ';' for function call");
}

TEST_F(ParserImplTest, Statement_Call_Bad_ArgList) {
  auto p = parser("a(b c);");
  auto e = p->statement();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(e.errored);
  EXPECT_FALSE(e.matched);
  EXPECT_EQ(p->error(), "1:5: expected ')' for function call");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
