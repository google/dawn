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

TEST_F(ParserImplTest, DecorationDecl_EmptyStr) {
  auto p = parser("");
  auto decos = p->decoration_list();
  EXPECT_FALSE(p->has_error());
  EXPECT_FALSE(decos.errored);
  EXPECT_FALSE(decos.matched);
  EXPECT_EQ(decos.value.size(), 0u);
}

TEST_F(ParserImplTest, DecorationDecl_EmptyBlock) {
  auto p = parser("[[]]");
  auto decos = p->decoration_list();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);
  EXPECT_EQ(decos.value.size(), 0u);
  EXPECT_EQ(p->error(), "1:3: empty decoration list");
}

TEST_F(ParserImplTest, DecorationDecl_Single) {
  auto p = parser("[[size(4)]]");
  auto decos = p->decoration_list();
  EXPECT_FALSE(p->has_error());
  EXPECT_FALSE(decos.errored);
  EXPECT_TRUE(decos.matched);
  ASSERT_EQ(decos.value.size(), 1u);
  auto* deco = decos.value[0]->As<ast::Decoration>();
  ASSERT_NE(deco, nullptr);
  EXPECT_TRUE(deco->Is<ast::StructMemberSizeDecoration>());
}

TEST_F(ParserImplTest, DecorationDecl_InvalidDecoration) {
  auto p = parser("[[size(nan)]]");
  auto decos = p->decoration_list();
  EXPECT_TRUE(p->has_error()) << p->error();
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);
  EXPECT_EQ(p->error(),
            "1:8: expected signed integer literal for size decoration");
}

TEST_F(ParserImplTest, DecorationDecl_MissingClose) {
  auto p = parser("[[size(4)");
  auto decos = p->decoration_list();
  EXPECT_TRUE(p->has_error()) << p->error();
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);
  EXPECT_EQ(p->error(), "1:10: expected ']]' for decoration list");
}

TEST_F(ParserImplTest, StructMemberDecorationDecl_SizeMissingClose) {
  auto p = parser("[[size(4)");
  auto decos = p->decoration_list();
  EXPECT_TRUE(p->has_error()) << p->error();
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);
  EXPECT_EQ(p->error(), "1:10: expected ']]' for decoration list");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
