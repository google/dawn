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

#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, ConstLiteral_Int) {
  auto p = parser("-234");
  auto c = p->const_literal();
  EXPECT_TRUE(c.matched);
  EXPECT_FALSE(c.errored);
  EXPECT_FALSE(p->has_error());
  ASSERT_NE(c.value, nullptr);
  ASSERT_TRUE(c->Is<ast::SintLiteral>());
  EXPECT_EQ(c->As<ast::SintLiteral>()->value(), -234);
  EXPECT_EQ(c->source().range, (Source::Range{{1u, 1u}, {1u, 5u}}));
}

TEST_F(ParserImplTest, ConstLiteral_Uint) {
  auto p = parser("234u");
  auto c = p->const_literal();
  EXPECT_TRUE(c.matched);
  EXPECT_FALSE(c.errored);
  EXPECT_FALSE(p->has_error());
  ASSERT_NE(c.value, nullptr);
  ASSERT_TRUE(c->Is<ast::UintLiteral>());
  EXPECT_EQ(c->As<ast::UintLiteral>()->value(), 234u);
  EXPECT_EQ(c->source().range, (Source::Range{{1u, 1u}, {1u, 5u}}));
}

TEST_F(ParserImplTest, ConstLiteral_Float) {
  auto p = parser("234.e12");
  auto c = p->const_literal();
  EXPECT_TRUE(c.matched);
  EXPECT_FALSE(c.errored);
  EXPECT_FALSE(p->has_error());
  ASSERT_NE(c.value, nullptr);
  ASSERT_TRUE(c->Is<ast::FloatLiteral>());
  EXPECT_FLOAT_EQ(c->As<ast::FloatLiteral>()->value(), 234e12f);
  EXPECT_EQ(c->source().range, (Source::Range{{1u, 1u}, {1u, 8u}}));
}

TEST_F(ParserImplTest, ConstLiteral_InvalidFloat) {
  auto p = parser("1.2e+256");
  auto c = p->const_literal();
  EXPECT_FALSE(c.matched);
  EXPECT_FALSE(c.errored);
  ASSERT_EQ(c.value, nullptr);
}

TEST_F(ParserImplTest, ConstLiteral_True) {
  auto p = parser("true");
  auto c = p->const_literal();
  EXPECT_TRUE(c.matched);
  EXPECT_FALSE(c.errored);
  EXPECT_FALSE(p->has_error());
  ASSERT_NE(c.value, nullptr);
  ASSERT_TRUE(c->Is<ast::BoolLiteral>());
  EXPECT_TRUE(c->As<ast::BoolLiteral>()->IsTrue());
  EXPECT_EQ(c->source().range, (Source::Range{{1u, 1u}, {1u, 5u}}));
}

TEST_F(ParserImplTest, ConstLiteral_False) {
  auto p = parser("false");
  auto c = p->const_literal();
  EXPECT_TRUE(c.matched);
  EXPECT_FALSE(c.errored);
  EXPECT_FALSE(p->has_error());
  ASSERT_NE(c.value, nullptr);
  ASSERT_TRUE(c->Is<ast::BoolLiteral>());
  EXPECT_TRUE(c->As<ast::BoolLiteral>()->IsFalse());
  EXPECT_EQ(c->source().range, (Source::Range{{1u, 1u}, {1u, 6u}}));
}

TEST_F(ParserImplTest, ConstLiteral_NoMatch) {
  auto p = parser("another-token");
  auto c = p->const_literal();
  EXPECT_FALSE(c.matched);
  EXPECT_FALSE(c.errored);
  EXPECT_FALSE(p->has_error());
  ASSERT_EQ(c.value, nullptr);
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
