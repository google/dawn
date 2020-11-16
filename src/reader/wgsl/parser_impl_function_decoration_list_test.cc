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
#include "src/ast/workgroup_decoration.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, FunctionDecorationList_Parses) {
  auto* p = parser("[[workgroup_size(2), workgroup_size(3, 4, 5)]]");
  auto decos = p->decoration_list();
  EXPECT_FALSE(p->has_error()) << p->error();
  EXPECT_FALSE(decos.errored);
  EXPECT_TRUE(decos.matched);
  ASSERT_EQ(decos.value.size(), 2u);

  auto* deco_0 = ast::As<ast::FunctionDecoration>(std::move(decos.value[0]));
  auto* deco_1 = ast::As<ast::FunctionDecoration>(std::move(decos.value[1]));
  ASSERT_NE(deco_0, nullptr);
  ASSERT_NE(deco_1, nullptr);

  uint32_t x = 0;
  uint32_t y = 0;
  uint32_t z = 0;
  ASSERT_TRUE(deco_0->IsWorkgroup());
  std::tie(x, y, z) = deco_0->AsWorkgroup()->values();
  EXPECT_EQ(x, 2u);

  ASSERT_TRUE(deco_1->IsWorkgroup());
  std::tie(x, y, z) = deco_1->AsWorkgroup()->values();
  EXPECT_EQ(x, 3u);
  EXPECT_EQ(y, 4u);
  EXPECT_EQ(z, 5u);
}

TEST_F(ParserImplTest, FunctionDecorationList_Empty) {
  auto* p = parser("[[]]");
  auto decos = p->decoration_list();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);
  EXPECT_EQ(p->error(), "1:3: empty decoration list");
}

TEST_F(ParserImplTest, FunctionDecorationList_Invalid) {
  auto* p = parser("[[invalid]]");
  auto decos = p->decoration_list();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);
  EXPECT_TRUE(decos.value.empty());
  EXPECT_EQ(p->error(), "1:3: expected decoration");
}

TEST_F(ParserImplTest, FunctionDecorationList_ExtraComma) {
  auto* p = parser("[[workgroup_size(2), ]]");
  auto decos = p->decoration_list();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);
  EXPECT_EQ(p->error(), "1:22: expected decoration");
}

TEST_F(ParserImplTest, FunctionDecorationList_MissingComma) {
  auto* p = parser("[[workgroup_size(2) workgroup_size(2)]]");
  auto decos = p->decoration_list();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);
  EXPECT_EQ(p->error(), "1:21: expected ',' for decoration list");
}

TEST_F(ParserImplTest, FunctionDecorationList_BadDecoration) {
  auto* p = parser("[[workgroup_size()]]");
  auto decos = p->decoration_list();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);
  EXPECT_EQ(
      p->error(),
      "1:18: expected signed integer literal for workgroup_size x parameter");
}

TEST_F(ParserImplTest, FunctionDecorationList_MissingRightAttr) {
  auto* p = parser("[[workgroup_size(2), workgroup_size(3, 4, 5)");
  auto decos = p->decoration_list();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);
  EXPECT_EQ(p->error(), "1:45: expected ']]' for decoration list");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
