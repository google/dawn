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

#include "src/ast/stage_decoration.h"
#include "src/ast/workgroup_decoration.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, Decoration_Workgroup) {
  auto p = parser("workgroup_size(4)");
  auto deco = p->decoration();
  EXPECT_TRUE(deco.matched);
  EXPECT_FALSE(deco.errored);
  ASSERT_NE(deco.value, nullptr) << p->error();
  ASSERT_FALSE(p->has_error());
  auto* func_deco = deco.value->As<ast::Decoration>();
  ASSERT_NE(func_deco, nullptr);
  ASSERT_TRUE(func_deco->Is<ast::WorkgroupDecoration>());

  auto values = func_deco->As<ast::WorkgroupDecoration>()->values();

  ASSERT_NE(values[0], nullptr);
  auto* x_scalar = values[0]->As<ast::ScalarConstructorExpression>();
  ASSERT_NE(x_scalar, nullptr);
  ASSERT_TRUE(x_scalar->literal()->Is<ast::IntLiteral>());
  EXPECT_EQ(x_scalar->literal()->As<ast::IntLiteral>()->value_as_u32(), 4u);

  EXPECT_EQ(values[1], nullptr);
  EXPECT_EQ(values[2], nullptr);
}

TEST_F(ParserImplTest, Decoration_Workgroup_2Param) {
  auto p = parser("workgroup_size(4, 5)");
  auto deco = p->decoration();
  EXPECT_TRUE(deco.matched);
  EXPECT_FALSE(deco.errored);
  ASSERT_NE(deco.value, nullptr) << p->error();
  ASSERT_FALSE(p->has_error());
  auto* func_deco = deco.value->As<ast::Decoration>();
  ASSERT_NE(func_deco, nullptr) << p->error();
  ASSERT_TRUE(func_deco->Is<ast::WorkgroupDecoration>());

  auto values = func_deco->As<ast::WorkgroupDecoration>()->values();

  ASSERT_NE(values[0], nullptr);
  auto* x_scalar = values[0]->As<ast::ScalarConstructorExpression>();
  ASSERT_NE(x_scalar, nullptr);
  ASSERT_TRUE(x_scalar->literal()->Is<ast::IntLiteral>());
  EXPECT_EQ(x_scalar->literal()->As<ast::IntLiteral>()->value_as_u32(), 4u);

  ASSERT_NE(values[1], nullptr);
  auto* y_scalar = values[1]->As<ast::ScalarConstructorExpression>();
  ASSERT_NE(y_scalar, nullptr);
  ASSERT_TRUE(y_scalar->literal()->Is<ast::IntLiteral>());
  EXPECT_EQ(y_scalar->literal()->As<ast::IntLiteral>()->value_as_u32(), 5u);

  EXPECT_EQ(values[2], nullptr);
}

TEST_F(ParserImplTest, Decoration_Workgroup_3Param) {
  auto p = parser("workgroup_size(4, 5, 6)");
  auto deco = p->decoration();
  EXPECT_TRUE(deco.matched);
  EXPECT_FALSE(deco.errored);
  ASSERT_NE(deco.value, nullptr) << p->error();
  ASSERT_FALSE(p->has_error());
  auto* func_deco = deco.value->As<ast::Decoration>();
  ASSERT_NE(func_deco, nullptr);
  ASSERT_TRUE(func_deco->Is<ast::WorkgroupDecoration>());

  auto values = func_deco->As<ast::WorkgroupDecoration>()->values();

  ASSERT_NE(values[0], nullptr);
  auto* x_scalar = values[0]->As<ast::ScalarConstructorExpression>();
  ASSERT_NE(x_scalar, nullptr);
  ASSERT_TRUE(x_scalar->literal()->Is<ast::IntLiteral>());
  EXPECT_EQ(x_scalar->literal()->As<ast::IntLiteral>()->value_as_u32(), 4u);

  ASSERT_NE(values[1], nullptr);
  auto* y_scalar = values[1]->As<ast::ScalarConstructorExpression>();
  ASSERT_NE(y_scalar, nullptr);
  ASSERT_TRUE(y_scalar->literal()->Is<ast::IntLiteral>());
  EXPECT_EQ(y_scalar->literal()->As<ast::IntLiteral>()->value_as_u32(), 5u);

  ASSERT_NE(values[2], nullptr);
  auto* z_scalar = values[2]->As<ast::ScalarConstructorExpression>();
  ASSERT_NE(z_scalar, nullptr);
  ASSERT_TRUE(z_scalar->literal()->Is<ast::IntLiteral>());
  EXPECT_EQ(z_scalar->literal()->As<ast::IntLiteral>()->value_as_u32(), 6u);
}

TEST_F(ParserImplTest, Decoration_Workgroup_WithIdent) {
  auto p = parser("workgroup_size(4, height)");
  auto deco = p->decoration();
  EXPECT_TRUE(deco.matched);
  EXPECT_FALSE(deco.errored);
  ASSERT_NE(deco.value, nullptr) << p->error();
  ASSERT_FALSE(p->has_error());
  auto* func_deco = deco.value->As<ast::Decoration>();
  ASSERT_NE(func_deco, nullptr);
  ASSERT_TRUE(func_deco->Is<ast::WorkgroupDecoration>());

  auto values = func_deco->As<ast::WorkgroupDecoration>()->values();

  ASSERT_NE(values[0], nullptr);
  auto* x_scalar = values[0]->As<ast::ScalarConstructorExpression>();
  ASSERT_NE(x_scalar, nullptr);
  ASSERT_TRUE(x_scalar->literal()->Is<ast::IntLiteral>());
  EXPECT_EQ(x_scalar->literal()->As<ast::IntLiteral>()->value_as_u32(), 4u);

  ASSERT_NE(values[1], nullptr);
  auto* y_ident = values[1]->As<ast::IdentifierExpression>();
  ASSERT_NE(y_ident, nullptr);
  EXPECT_EQ(p->builder().Symbols().NameFor(y_ident->symbol()), "height");

  ASSERT_EQ(values[2], nullptr);
}

TEST_F(ParserImplTest, Decoration_Workgroup_TooManyValues) {
  auto p = parser("workgroup_size(1, 2, 3, 4)");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:23: expected ')' for workgroup_size decoration");
}

TEST_F(ParserImplTest, Decoration_Workgroup_MissingLeftParam) {
  auto p = parser("workgroup_size 4, 5, 6)");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:16: expected '(' for workgroup_size decoration");
}

TEST_F(ParserImplTest, Decoration_Workgroup_MissingRightParam) {
  auto p = parser("workgroup_size(4, 5, 6");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:23: expected ')' for workgroup_size decoration");
}

TEST_F(ParserImplTest, Decoration_Workgroup_MissingValues) {
  auto p = parser("workgroup_size()");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:16: expected workgroup_size x parameter");
}

TEST_F(ParserImplTest, Decoration_Workgroup_Missing_X_Value) {
  auto p = parser("workgroup_size(, 2, 3)");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:16: expected workgroup_size x parameter");
}

TEST_F(ParserImplTest, Decoration_Workgroup_Missing_Y_Comma) {
  auto p = parser("workgroup_size(1 2, 3)");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:18: expected ')' for workgroup_size decoration");
}

TEST_F(ParserImplTest, Decoration_Workgroup_Missing_Y_Value) {
  auto p = parser("workgroup_size(1, , 3)");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:19: expected workgroup_size y parameter");
}

TEST_F(ParserImplTest, Decoration_Workgroup_Missing_Z_Comma) {
  auto p = parser("workgroup_size(1, 2 3)");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:21: expected ')' for workgroup_size decoration");
}

TEST_F(ParserImplTest, Decoration_Workgroup_Missing_Z_Value) {
  auto p = parser("workgroup_size(1, 2, )");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:22: expected workgroup_size z parameter");
}

TEST_F(ParserImplTest, Decoration_Stage) {
  auto p = parser("stage(compute)");
  auto deco = p->decoration();
  EXPECT_TRUE(deco.matched);
  EXPECT_FALSE(deco.errored);
  ASSERT_NE(deco.value, nullptr) << p->error();
  ASSERT_FALSE(p->has_error());
  auto* func_deco = deco.value->As<ast::Decoration>();
  ASSERT_NE(func_deco, nullptr);
  ASSERT_TRUE(func_deco->Is<ast::StageDecoration>());
  EXPECT_EQ(func_deco->As<ast::StageDecoration>()->value(),
            ast::PipelineStage::kCompute);
}

TEST_F(ParserImplTest, Decoration_Stage_MissingValue) {
  auto p = parser("stage()");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:7: invalid value for stage decoration");
}

TEST_F(ParserImplTest, Decoration_Stage_MissingInvalid) {
  auto p = parser("stage(nan)");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:7: invalid value for stage decoration");
}

TEST_F(ParserImplTest, Decoration_Stage_MissingLeftParen) {
  auto p = parser("stage compute)");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:7: expected '(' for stage decoration");
}

TEST_F(ParserImplTest, Decoration_Stage_MissingRightParen) {
  auto p = parser("stage(compute");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:14: expected ')' for stage decoration");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
