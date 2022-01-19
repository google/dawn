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

TEST_F(ParserImplTest, DecorationList_Parses) {
  auto p = parser(R"(@location(4) @builtin(position))");
  auto decos = p->decoration_list();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(decos.errored);
  ASSERT_TRUE(decos.matched);
  ASSERT_EQ(decos.value.size(), 2u);

  auto* deco_0 = decos.value[0]->As<ast::Decoration>();
  auto* deco_1 = decos.value[1]->As<ast::Decoration>();
  ASSERT_NE(deco_0, nullptr);
  ASSERT_NE(deco_1, nullptr);

  ASSERT_TRUE(deco_0->Is<ast::LocationDecoration>());
  EXPECT_EQ(deco_0->As<ast::LocationDecoration>()->value, 4u);
  ASSERT_TRUE(deco_1->Is<ast::BuiltinDecoration>());
  EXPECT_EQ(deco_1->As<ast::BuiltinDecoration>()->builtin,
            ast::Builtin::kPosition);
}

TEST_F(ParserImplTest, DecorationList_Invalid) {
  auto p = parser(R"(@invalid)");
  auto decos = p->decoration_list();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);
  EXPECT_TRUE(decos.value.empty());
  EXPECT_EQ(p->error(), R"(1:2: expected decoration)");
}

TEST_F(ParserImplTest, DecorationList_InvalidValue) {
  auto p = parser("@builtin(invalid)");
  auto decos = p->decoration_list();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);
  EXPECT_TRUE(decos.value.empty());
  EXPECT_EQ(p->error(), "1:10: invalid value for builtin decoration");
}

// TODO(crbug.com/tint/1382): Remove
TEST_F(ParserImplTest, DEPRECATED_DecorationList_Empty) {
  auto p = parser(R"([[]])");
  auto decos = p->decoration_list();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);
  EXPECT_TRUE(decos.value.empty());
  EXPECT_EQ(
      p->error(),
      R"(1:1: use of deprecated language feature: [[decoration]] style decorations have been replaced with @decoration style
1:3: empty decoration list)");
}

// TODO(crbug.com/tint/1382): Remove
TEST_F(ParserImplTest, DEPRECATED_DecorationList_Invalid) {
  auto p = parser(R"([[invalid]])");
  auto decos = p->decoration_list();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);
  EXPECT_TRUE(decos.value.empty());
  EXPECT_EQ(
      p->error(),
      R"(1:1: use of deprecated language feature: [[decoration]] style decorations have been replaced with @decoration style
1:3: expected decoration)");
}

// TODO(crbug.com/tint/1382): Remove
TEST_F(ParserImplTest, DEPRECATED_DecorationList_ExtraComma) {
  auto p = parser(R"([[builtin(position), ]])");
  auto decos = p->decoration_list();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);
  EXPECT_TRUE(decos.value.empty());
  EXPECT_EQ(
      p->error(),
      R"(1:1: use of deprecated language feature: [[decoration]] style decorations have been replaced with @decoration style
1:22: expected decoration)");
}

// TODO(crbug.com/tint/1382): Remove
TEST_F(ParserImplTest, DEPRECATED_DecorationList_MissingComma) {
  auto p = parser(R"([[binding(4) location(5)]])");
  auto decos = p->decoration_list();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);
  EXPECT_TRUE(decos.value.empty());
  EXPECT_EQ(
      p->error(),
      R"(1:1: use of deprecated language feature: [[decoration]] style decorations have been replaced with @decoration style
1:14: expected ',' for decoration list)");
}

// TODO(crbug.com/tint/1382): Remove
TEST_F(ParserImplTest, DEPRECATED_DecorationList_InvalidValue) {
  auto p = parser("[[builtin(invalid)]]");
  auto decos = p->decoration_list();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);
  EXPECT_TRUE(decos.value.empty());
  EXPECT_EQ(
      p->error(),
      R"(1:1: use of deprecated language feature: [[decoration]] style decorations have been replaced with @decoration style
1:11: invalid value for builtin decoration)");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
