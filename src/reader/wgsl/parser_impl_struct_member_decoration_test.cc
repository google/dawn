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

TEST_F(ParserImplTest, Decoration_Offset) {
  auto p = parser("offset(4)");
  auto deco = p->decoration();
  EXPECT_TRUE(deco.matched);
  EXPECT_FALSE(deco.errored);
  ASSERT_NE(deco.value, nullptr);
  ASSERT_FALSE(p->has_error());

  auto* member_deco = deco.value->As<ast::Decoration>();
  ASSERT_NE(member_deco, nullptr);
  ASSERT_TRUE(member_deco->Is<ast::StructMemberOffsetDecoration>());

  auto* o = member_deco->As<ast::StructMemberOffsetDecoration>();
  EXPECT_EQ(o->offset(), 4u);
}

TEST_F(ParserImplTest, Decoration_Offset_MissingLeftParen) {
  auto p = parser("offset 4)");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(
      p->error(),
      R"(1:1: use of deprecated language feature: [[offset]] has been replaced with [[size]] and [[align]]
1:8: expected '(' for offset decoration)");
}

TEST_F(ParserImplTest, Decoration_Offset_MissingRightParen) {
  auto p = parser("offset(4");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(
      p->error(),
      R"(1:1: use of deprecated language feature: [[offset]] has been replaced with [[size]] and [[align]]
1:9: expected ')' for offset decoration)");
}

TEST_F(ParserImplTest, Decoration_Offset_MissingValue) {
  auto p = parser("offset()");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(
      p->error(),
      R"(1:1: use of deprecated language feature: [[offset]] has been replaced with [[size]] and [[align]]
1:8: expected signed integer literal for offset decoration)");
}

TEST_F(ParserImplTest, Decoration_Offset_MissingInvalid) {
  auto p = parser("offset(nan)");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(
      p->error(),
      R"(1:1: use of deprecated language feature: [[offset]] has been replaced with [[size]] and [[align]]
1:8: expected signed integer literal for offset decoration)");
}

TEST_F(ParserImplTest, Decoration_Size) {
  auto p = parser("size(4)");
  auto deco = p->decoration();
  EXPECT_TRUE(deco.matched);
  EXPECT_FALSE(deco.errored);
  ASSERT_NE(deco.value, nullptr);
  ASSERT_FALSE(p->has_error());

  auto* member_deco = deco.value->As<ast::Decoration>();
  ASSERT_NE(member_deco, nullptr);
  ASSERT_TRUE(member_deco->Is<ast::StructMemberSizeDecoration>());

  auto* o = member_deco->As<ast::StructMemberSizeDecoration>();
  EXPECT_EQ(o->size(), 4u);
}

TEST_F(ParserImplTest, Decoration_Size_MissingLeftParen) {
  auto p = parser("size 4)");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:6: expected '(' for size decoration");
}

TEST_F(ParserImplTest, Decoration_Size_MissingRightParen) {
  auto p = parser("size(4");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:7: expected ')' for size decoration");
}

TEST_F(ParserImplTest, Decoration_Size_MissingValue) {
  auto p = parser("size()");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(),
            "1:6: expected signed integer literal for size decoration");
}

TEST_F(ParserImplTest, Decoration_Size_MissingInvalid) {
  auto p = parser("size(nan)");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(),
            "1:6: expected signed integer literal for size decoration");
}

TEST_F(ParserImplTest, Decoration_Align) {
  auto p = parser("align(4)");
  auto deco = p->decoration();
  EXPECT_TRUE(deco.matched);
  EXPECT_FALSE(deco.errored);
  ASSERT_NE(deco.value, nullptr);
  ASSERT_FALSE(p->has_error());

  auto* member_deco = deco.value->As<ast::Decoration>();
  ASSERT_NE(member_deco, nullptr);
  ASSERT_TRUE(member_deco->Is<ast::StructMemberAlignDecoration>());

  auto* o = member_deco->As<ast::StructMemberAlignDecoration>();
  EXPECT_EQ(o->align(), 4u);
}

TEST_F(ParserImplTest, Decoration_Align_MissingLeftParen) {
  auto p = parser("align 4)");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:7: expected '(' for align decoration");
}

TEST_F(ParserImplTest, Decoration_Align_MissingRightParen) {
  auto p = parser("align(4");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:8: expected ')' for align decoration");
}

TEST_F(ParserImplTest, Decoration_Align_MissingValue) {
  auto p = parser("align()");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(),
            "1:7: expected signed integer literal for align decoration");
}

TEST_F(ParserImplTest, Decoration_Align_MissingInvalid) {
  auto p = parser("align(nan)");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(),
            "1:7: expected signed integer literal for align decoration");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
