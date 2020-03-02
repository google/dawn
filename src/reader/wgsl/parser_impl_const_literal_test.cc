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
#include "src/ast/int_literal.h"
#include "src/ast/uint_literal.h"
#include "src/reader/wgsl/parser_impl.h"

namespace tint {
namespace reader {
namespace wgsl {

using ParserImplTest = testing::Test;

TEST_F(ParserImplTest, ConstLiteral_Int) {
  ParserImpl p{"-234"};
  auto c = p.const_literal();
  ASSERT_FALSE(p.has_error());
  ASSERT_NE(c, nullptr);
  ASSERT_TRUE(c->IsInt());
  EXPECT_EQ(c->AsInt()->value(), -234);
}

TEST_F(ParserImplTest, ConstLiteral_Uint) {
  ParserImpl p{"234u"};
  auto c = p.const_literal();
  ASSERT_FALSE(p.has_error());
  ASSERT_NE(c, nullptr);
  ASSERT_TRUE(c->IsUint());
  EXPECT_EQ(c->AsUint()->value(), 234u);
}

TEST_F(ParserImplTest, ConstLiteral_Float) {
  ParserImpl p{"234.e12"};
  auto c = p.const_literal();
  ASSERT_FALSE(p.has_error());
  ASSERT_NE(c, nullptr);
  ASSERT_TRUE(c->IsFloat());
  EXPECT_FLOAT_EQ(c->AsFloat()->value(), 234e12);
}

TEST_F(ParserImplTest, ConstLiteral_InvalidFloat) {
  ParserImpl p{"1.2e+256"};
  auto c = p.const_literal();
  ASSERT_EQ(c, nullptr);
}

TEST_F(ParserImplTest, ConstLiteral_True) {
  ParserImpl p{"true"};
  auto c = p.const_literal();
  ASSERT_FALSE(p.has_error());
  ASSERT_NE(c, nullptr);
  ASSERT_TRUE(c->IsBool());
  EXPECT_TRUE(c->AsBool()->IsTrue());
}

TEST_F(ParserImplTest, ConstLiteral_False) {
  ParserImpl p{"false"};
  auto c = p.const_literal();
  ASSERT_FALSE(p.has_error());
  ASSERT_NE(c, nullptr);
  ASSERT_TRUE(c->IsBool());
  EXPECT_TRUE(c->AsBool()->IsFalse());
}

TEST_F(ParserImplTest, ConstLiteral_NoMatch) {
  ParserImpl p{"another-token"};
  auto c = p.const_literal();
  ASSERT_FALSE(p.has_error());
  ASSERT_EQ(c, nullptr);
}

}  // namespace wgsl
}  // namespace reader
}  // namespace tint
