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
#include "src/ast/bool_literal.h"
#include "src/ast/float_literal.h"
#include "src/ast/sint_literal.h"
#include "src/ast/uint_literal.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, ConstLiteral_Int) {
  auto* p = parser("-234");
  auto c = p->const_literal();
  ASSERT_FALSE(p->has_error());
  ASSERT_NE(c, nullptr);
  ASSERT_TRUE(c->IsSint());
  EXPECT_EQ(c->AsSint()->value(), -234);
}

TEST_F(ParserImplTest, ConstLiteral_Uint) {
  auto* p = parser("234u");
  auto c = p->const_literal();
  ASSERT_FALSE(p->has_error());
  ASSERT_NE(c, nullptr);
  ASSERT_TRUE(c->IsUint());
  EXPECT_EQ(c->AsUint()->value(), 234u);
}

TEST_F(ParserImplTest, ConstLiteral_Float) {
  auto* p = parser("234.e12");
  auto c = p->const_literal();
  ASSERT_FALSE(p->has_error());
  ASSERT_NE(c, nullptr);
  ASSERT_TRUE(c->IsFloat());
  EXPECT_FLOAT_EQ(c->AsFloat()->value(), 234e12f);
}

TEST_F(ParserImplTest, ConstLiteral_InvalidFloat) {
  auto* p = parser("1.2e+256");
  auto c = p->const_literal();
  ASSERT_EQ(c, nullptr);
}

TEST_F(ParserImplTest, ConstLiteral_True) {
  auto* p = parser("true");
  auto c = p->const_literal();
  ASSERT_FALSE(p->has_error());
  ASSERT_NE(c, nullptr);
  ASSERT_TRUE(c->IsBool());
  EXPECT_TRUE(c->AsBool()->IsTrue());
}

TEST_F(ParserImplTest, ConstLiteral_False) {
  auto* p = parser("false");
  auto c = p->const_literal();
  ASSERT_FALSE(p->has_error());
  ASSERT_NE(c, nullptr);
  ASSERT_TRUE(c->IsBool());
  EXPECT_TRUE(c->AsBool()->IsFalse());
}

TEST_F(ParserImplTest, ConstLiteral_NoMatch) {
  auto* p = parser("another-token");
  auto c = p->const_literal();
  ASSERT_FALSE(p->has_error());
  ASSERT_EQ(c, nullptr);
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
