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
#include "src/ast/return_statement.h"
#include "src/ast/statement.h"
#include "src/reader/wgsl/parser_impl.h"

namespace tint {
namespace reader {
namespace wgsl {

using ParserImplTest = testing::Test;

TEST_F(ParserImplTest, Statement) {
  ParserImpl p{"return;"};
  auto e = p.statement();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_NE(e, nullptr);
  EXPECT_TRUE(e->IsReturn());
}

TEST_F(ParserImplTest, Statement_Semicolon) {
  ParserImpl p{";"};
  auto e = p.statement();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_EQ(e, nullptr);
}

TEST_F(ParserImplTest, Statement_Return_NoValue) {
  ParserImpl p{"return;"};
  auto e = p.statement();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_NE(e, nullptr);

  ASSERT_TRUE(e->IsReturn());
  auto ret = e->AsReturn();
  ASSERT_EQ(ret->value(), nullptr);
}

TEST_F(ParserImplTest, Statement_Return_Value) {
  ParserImpl p{"return a + b * (.1 - .2);"};
  auto e = p.statement();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_NE(e, nullptr);

  ASSERT_TRUE(e->IsReturn());
  auto ret = e->AsReturn();
  ASSERT_NE(ret->value(), nullptr);
  EXPECT_TRUE(ret->value()->IsRelational());
}

TEST_F(ParserImplTest, Statement_Return_MissingSemi) {
  ParserImpl p{"return"};
  auto e = p.statement();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:7: missing ;");
}

TEST_F(ParserImplTest, Statement_Return_Invalid) {
  ParserImpl p{"return if(a) {};"};
  auto e = p.statement();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:8: missing ;");
}

TEST_F(ParserImplTest, Statement_If) {
  ParserImpl p{"if (a) {}"};
  auto e = p.statement();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsIf());
}

TEST_F(ParserImplTest, Statement_If_Invalid) {
  ParserImpl p{"if (a) { fn main() -> {}}"};
  auto e = p.statement();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:10: missing }");
}

TEST_F(ParserImplTest, Statement_Unless) {
  ParserImpl p{"unless (a) {}"};
  auto e = p.statement();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsUnless());
}

TEST_F(ParserImplTest, Statement_Unless_Invalid) {
  ParserImpl p{"unless () {}"};
  auto e = p.statement();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:9: unable to parse expression");
}

TEST_F(ParserImplTest, Statement_Regardless) {
  ParserImpl p{"regardless (a) {}"};
  auto e = p.statement();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsRegardless());
}

TEST_F(ParserImplTest, Statement_Regardless_Invalid) {
  ParserImpl p{"regardless () {}"};
  auto e = p.statement();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:13: unable to parse expression");
}

TEST_F(ParserImplTest, Statement_Variable) {
  ParserImpl p{"var a : i32 = 1;"};
  auto e = p.statement();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsVariable());
}

TEST_F(ParserImplTest, Statement_Variable_Invalid) {
  ParserImpl p{"var a : i32 =;"};
  auto e = p.statement();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:14: missing initializer for variable declaration");
}

TEST_F(ParserImplTest, Statement_Variable_MissingSemicolon) {
  ParserImpl p{"var a : i32"};
  auto e = p.statement();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:12: missing ;");
}

TEST_F(ParserImplTest, Statement_Switch) {
  ParserImpl p{"switch (a) {}"};
  auto e = p.statement();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsSwitch());
}

TEST_F(ParserImplTest, Statement_Switch_Invalid) {
  ParserImpl p{"switch (a) { case: {}}"};
  auto e = p.statement();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:18: unable to parse case conditional");
}

TEST_F(ParserImplTest, Statement_Loop) {
  ParserImpl p{"loop {}"};
  auto e = p.statement();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsLoop());
}

TEST_F(ParserImplTest, Statement_Loop_Invalid) {
  ParserImpl p{"loop kill; }"};
  auto e = p.statement();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:6: missing { for loop");
}

TEST_F(ParserImplTest, Statement_Assignment) {
  ParserImpl p{"a = b;"};
  auto e = p.statement();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_NE(e, nullptr);
  EXPECT_TRUE(e->IsAssign());
}

TEST_F(ParserImplTest, Statement_Assignment_Invalid) {
  ParserImpl p{"a = if(b) {};"};
  auto e = p.statement();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:5: unable to parse right side of assignment");
}

TEST_F(ParserImplTest, Statement_Assignment_MissingSemicolon) {
  ParserImpl p{"a = b"};
  auto e = p.statement();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:6: missing ;");
}

TEST_F(ParserImplTest, Statement_Break) {
  ParserImpl p{"break;"};
  auto e = p.statement();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_NE(e, nullptr);
  EXPECT_TRUE(e->IsBreak());
}

TEST_F(ParserImplTest, Statement_Break_Invalid) {
  ParserImpl p{"break if (a = b);"};
  auto e = p.statement();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:13: expected )");
}

TEST_F(ParserImplTest, Statement_Break_MissingSemicolon) {
  ParserImpl p{"break if (a == b)"};
  auto e = p.statement();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:18: missing ;");
}

TEST_F(ParserImplTest, Statement_Continue) {
  ParserImpl p{"continue;"};
  auto e = p.statement();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_NE(e, nullptr);
  EXPECT_TRUE(e->IsContinue());
}

TEST_F(ParserImplTest, Statement_Continue_Invalid) {
  ParserImpl p{"continue if (a = b);"};
  auto e = p.statement();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:16: expected )");
}

TEST_F(ParserImplTest, Statement_Continue_MissingSemicolon) {
  ParserImpl p{"continue if (a == b)"};
  auto e = p.statement();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:21: missing ;");
}

TEST_F(ParserImplTest, Statement_Kill) {
  ParserImpl p{"kill;"};
  auto e = p.statement();
  ASSERT_FALSE(p.has_error()) << p.error();
  EXPECT_NE(e, nullptr);
  ASSERT_TRUE(e->IsKill());
}

TEST_F(ParserImplTest, Statement_Kill_MissingSemicolon) {
  ParserImpl p{"kill"};
  auto e = p.statement();
  ASSERT_TRUE(p.has_error());
  EXPECT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:5: missing ;");
}

TEST_F(ParserImplTest, Statement_Nop) {
  ParserImpl p{"nop;"};
  auto e = p.statement();
  ASSERT_FALSE(p.has_error()) << p.error();
  EXPECT_NE(e, nullptr);
  ASSERT_TRUE(e->IsNop());
}

TEST_F(ParserImplTest, Statement_Nop_MissingSemicolon) {
  ParserImpl p{"nop"};
  auto e = p.statement();
  ASSERT_TRUE(p.has_error());
  EXPECT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:4: missing ;");
}

}  // namespace wgsl
}  // namespace reader
}  // namespace tint
