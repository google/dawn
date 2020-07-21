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
#include "src/ast/call_expression.h"
#include "src/ast/call_statement.h"
#include "src/ast/identifier_expression.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, Statement_Call) {
  auto* p = parser("a();");
  auto e = p->statement();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);

  ASSERT_TRUE(e->IsCall());
  auto* c = e->AsCall()->expr();

  ASSERT_TRUE(c->func()->IsIdentifier());
  auto* func = c->func()->AsIdentifier();
  EXPECT_EQ(func->name(), "a");

  EXPECT_EQ(c->params().size(), 0u);
}

TEST_F(ParserImplTest, Statement_Call_WithParams) {
  auto* p = parser("a(1, b, 2 + 3 / b);");
  auto e = p->statement();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);

  ASSERT_TRUE(e->IsCall());
  auto* c = e->AsCall()->expr();

  ASSERT_TRUE(c->func()->IsIdentifier());
  auto* func = c->func()->AsIdentifier();
  EXPECT_EQ(func->name(), "a");

  EXPECT_EQ(c->params().size(), 3u);
  EXPECT_TRUE(c->params()[0]->IsConstructor());
  EXPECT_TRUE(c->params()[1]->IsIdentifier());
  EXPECT_TRUE(c->params()[2]->IsBinary());
}

TEST_F(ParserImplTest, Statement_Call_Missing_RightParen) {
  auto* p = parser("a(");
  auto e = p->statement();
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:3: missing ) for call statement");
}

TEST_F(ParserImplTest, Statement_Call_Missing_Semi) {
  auto* p = parser("a()");
  auto e = p->statement();
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:4: missing ;");
}

TEST_F(ParserImplTest, Statement_Call_Bad_ArgList) {
  auto* p = parser("a(b c);");
  auto e = p->statement();
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:5: missing ) for call statement");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
