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

#include "src/ast/member_accessor_expression.h"

#include <sstream>

#include "gtest/gtest.h"
#include "src/ast/identifier_expression.h"

namespace tint {
namespace ast {
namespace {

using MemberAccessorExpressionTest = testing::Test;

TEST_F(MemberAccessorExpressionTest, Creation) {
  auto str = std::make_unique<IdentifierExpression>("structure");
  auto mem = std::make_unique<IdentifierExpression>("member");

  auto* str_ptr = str.get();
  auto* mem_ptr = mem.get();

  MemberAccessorExpression stmt(std::move(str), std::move(mem));
  EXPECT_EQ(stmt.structure(), str_ptr);
  EXPECT_EQ(stmt.member(), mem_ptr);
}

TEST_F(MemberAccessorExpressionTest, Creation_WithSource) {
  auto str = std::make_unique<IdentifierExpression>("structure");
  auto mem = std::make_unique<IdentifierExpression>("member");

  MemberAccessorExpression stmt(Source{20, 2}, std::move(str), std::move(mem));
  auto src = stmt.source();
  EXPECT_EQ(src.line, 20u);
  EXPECT_EQ(src.column, 2u);
}

TEST_F(MemberAccessorExpressionTest, IsMemberAccessor) {
  MemberAccessorExpression stmt;
  EXPECT_TRUE(stmt.IsMemberAccessor());
}

TEST_F(MemberAccessorExpressionTest, IsValid) {
  auto str = std::make_unique<IdentifierExpression>("structure");
  auto mem = std::make_unique<IdentifierExpression>("member");

  MemberAccessorExpression stmt(std::move(str), std::move(mem));
  EXPECT_TRUE(stmt.IsValid());
}

TEST_F(MemberAccessorExpressionTest, IsValid_NullStruct) {
  auto mem = std::make_unique<IdentifierExpression>("member");

  MemberAccessorExpression stmt;
  stmt.set_member(std::move(mem));
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(MemberAccessorExpressionTest, IsValid_InvalidStruct) {
  auto str = std::make_unique<IdentifierExpression>("");
  auto mem = std::make_unique<IdentifierExpression>("member");

  MemberAccessorExpression stmt(std::move(str), std::move(mem));
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(MemberAccessorExpressionTest, IsValid_NullMember) {
  auto str = std::make_unique<IdentifierExpression>("structure");

  MemberAccessorExpression stmt;
  stmt.set_structure(std::move(str));
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(MemberAccessorExpressionTest, IsValid_InvalidMember) {
  auto str = std::make_unique<IdentifierExpression>("structure");
  auto mem = std::make_unique<IdentifierExpression>("");

  MemberAccessorExpression stmt(std::move(str), std::move(mem));
  EXPECT_FALSE(stmt.IsValid());
}

TEST_F(MemberAccessorExpressionTest, ToStr) {
  auto str = std::make_unique<IdentifierExpression>("structure");
  auto mem = std::make_unique<IdentifierExpression>("member");

  MemberAccessorExpression stmt(std::move(str), std::move(mem));
  std::ostringstream out;
  stmt.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  MemberAccessor{
    Identifier{structure}
    Identifier{member}
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
