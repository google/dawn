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

#include "src/ast/workgroup_decoration.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, DecorationList_Parses) {
  auto p = parser("[[workgroup_size(2), stage(compute)]]");
  auto decos = p->decoration_list();
  EXPECT_FALSE(p->has_error()) << p->error();
  EXPECT_FALSE(decos.errored);
  EXPECT_TRUE(decos.matched);
  ASSERT_EQ(decos.value.size(), 2u);

  auto* deco_0 = decos.value[0]->As<ast::Decoration>();
  auto* deco_1 = decos.value[1]->As<ast::Decoration>();
  ASSERT_NE(deco_0, nullptr);
  ASSERT_NE(deco_1, nullptr);

  ASSERT_TRUE(deco_0->Is<ast::WorkgroupDecoration>());
  ast::Expression* x = deco_0->As<ast::WorkgroupDecoration>()->values()[0];
  ASSERT_NE(x, nullptr);
  auto* x_scalar = x->As<ast::ScalarConstructorExpression>();
  ASSERT_NE(x_scalar, nullptr);
  ASSERT_TRUE(x_scalar->literal()->Is<ast::IntLiteral>());
  EXPECT_EQ(x_scalar->literal()->As<ast::IntLiteral>()->value_as_u32(), 2u);

  ASSERT_TRUE(deco_1->Is<ast::StageDecoration>());
  EXPECT_EQ(deco_1->As<ast::StageDecoration>()->value(),
            ast::PipelineStage::kCompute);
}

TEST_F(ParserImplTest, DecorationList_Empty) {
  auto p = parser("[[]]");
  auto decos = p->decoration_list();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);
  EXPECT_EQ(p->error(), "1:3: empty decoration list");
}

TEST_F(ParserImplTest, DecorationList_Invalid) {
  auto p = parser("[[invalid]]");
  auto decos = p->decoration_list();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);
  EXPECT_TRUE(decos.value.empty());
  EXPECT_EQ(p->error(), "1:3: expected decoration");
}

TEST_F(ParserImplTest, DecorationList_ExtraComma) {
  auto p = parser("[[workgroup_size(2), ]]");
  auto decos = p->decoration_list();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);
  EXPECT_EQ(p->error(), "1:22: expected decoration");
}

TEST_F(ParserImplTest, DecorationList_MissingComma) {
  auto p = parser("[[workgroup_size(2) workgroup_size(2)]]");
  auto decos = p->decoration_list();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);
  EXPECT_EQ(p->error(), "1:21: expected ',' for decoration list");
}

TEST_F(ParserImplTest, DecorationList_BadDecoration) {
  auto p = parser("[[stage()]]");
  auto decos = p->decoration_list();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);
  EXPECT_EQ(p->error(), "1:9: invalid value for stage decoration");
}

TEST_F(ParserImplTest, DecorationList_MissingRightAttr) {
  auto p = parser("[[workgroup_size(2), workgroup_size(3, 4, 5)");
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
