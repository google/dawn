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
#include "src/ast/stage_decoration.h"
#include "src/ast/workgroup_decoration.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, FunctionDecoration_Workgroup) {
  auto* p = parser("workgroup_size(4)");
  auto deco = p->function_decoration();
  ASSERT_NE(deco, nullptr);
  ASSERT_FALSE(p->has_error());
  ASSERT_TRUE(deco->IsWorkgroup());

  uint32_t x = 0;
  uint32_t y = 0;
  uint32_t z = 0;
  std::tie(x, y, z) = deco->AsWorkgroup()->values();
  EXPECT_EQ(x, 4u);
  EXPECT_EQ(y, 1u);
  EXPECT_EQ(z, 1u);
}

TEST_F(ParserImplTest, FunctionDecoration_Workgroup_2Param) {
  auto* p = parser("workgroup_size(4, 5)");
  auto deco = p->function_decoration();
  ASSERT_NE(deco, nullptr) << p->error();
  ASSERT_FALSE(p->has_error());
  ASSERT_TRUE(deco->IsWorkgroup());

  uint32_t x = 0;
  uint32_t y = 0;
  uint32_t z = 0;
  std::tie(x, y, z) = deco->AsWorkgroup()->values();
  EXPECT_EQ(x, 4u);
  EXPECT_EQ(y, 5u);
  EXPECT_EQ(z, 1u);
}

TEST_F(ParserImplTest, FunctionDecoration_Workgroup_3Param) {
  auto* p = parser("workgroup_size(4, 5, 6)");
  auto deco = p->function_decoration();
  ASSERT_NE(deco, nullptr);
  ASSERT_FALSE(p->has_error());
  ASSERT_TRUE(deco->IsWorkgroup());

  uint32_t x = 0;
  uint32_t y = 0;
  uint32_t z = 0;
  std::tie(x, y, z) = deco->AsWorkgroup()->values();
  EXPECT_EQ(x, 4u);
  EXPECT_EQ(y, 5u);
  EXPECT_EQ(z, 6u);
}

TEST_F(ParserImplTest, FunctionDecoration_Workgroup_TooManyValues) {
  auto* p = parser("workgroup_size(1, 2, 3, 4)");
  auto deco = p->function_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:23: missing ) for workgroup_size");
}

TEST_F(ParserImplTest, FunctionDecoration_Workgroup_Invalid_X_Value) {
  auto* p = parser("workgroup_size(-2, 5, 6)");
  auto deco = p->function_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:16: invalid value for workgroup_size x parameter");
}

TEST_F(ParserImplTest, FunctionDecoration_Workgroup_Invalid_Y_Value) {
  auto* p = parser("workgroup_size(4, 0, 6)");
  auto deco = p->function_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:19: invalid value for workgroup_size y parameter");
}

TEST_F(ParserImplTest, FunctionDecoration_Workgroup_Invalid_Z_Value) {
  auto* p = parser("workgroup_size(4, 5, -3)");
  auto deco = p->function_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:22: invalid value for workgroup_size z parameter");
}

TEST_F(ParserImplTest, FunctionDecoration_Workgroup_MissingLeftParam) {
  auto* p = parser("workgroup_size 4, 5, 6)");
  auto deco = p->function_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:16: missing ( for workgroup_size");
}

TEST_F(ParserImplTest, FunctionDecoration_Workgroup_MissingRightParam) {
  auto* p = parser("workgroup_size(4, 5, 6");
  auto deco = p->function_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:23: missing ) for workgroup_size");
}

TEST_F(ParserImplTest, FunctionDecoration_Workgroup_MissingValues) {
  auto* p = parser("workgroup_size()");
  auto deco = p->function_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:16: missing x value for workgroup_size");
}

TEST_F(ParserImplTest, FunctionDecoration_Workgroup_Missing_X_Value) {
  auto* p = parser("workgroup_size(, 2, 3)");
  auto deco = p->function_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:16: missing x value for workgroup_size");
}

TEST_F(ParserImplTest, FunctionDecoration_Workgroup_Missing_Y_Comma) {
  auto* p = parser("workgroup_size(1 2, 3)");
  auto deco = p->function_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:18: missing ) for workgroup_size");
}

TEST_F(ParserImplTest, FunctionDecoration_Workgroup_Missing_Y_Value) {
  auto* p = parser("workgroup_size(1, , 3)");
  auto deco = p->function_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:19: missing y value for workgroup_size");
}

TEST_F(ParserImplTest, FunctionDecoration_Workgroup_Missing_Z_Comma) {
  auto* p = parser("workgroup_size(1, 2 3)");
  auto deco = p->function_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:21: missing ) for workgroup_size");
}

TEST_F(ParserImplTest, FunctionDecoration_Workgroup_Missing_Z_Value) {
  auto* p = parser("workgroup_size(1, 2, )");
  auto deco = p->function_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:22: missing z value for workgroup_size");
}

TEST_F(ParserImplTest, FunctionDecoration_Workgroup_Missing_X_Invalid) {
  auto* p = parser("workgroup_size(nan)");
  auto deco = p->function_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:16: missing x value for workgroup_size");
}

TEST_F(ParserImplTest, FunctionDecoration_Workgroup_Missing_Y_Invalid) {
  auto* p = parser("workgroup_size(2, nan)");
  auto deco = p->function_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:19: missing y value for workgroup_size");
}

TEST_F(ParserImplTest, FunctionDecoration_Workgroup_Missing_Z_Invalid) {
  auto* p = parser("workgroup_size(2, 3, nan)");
  auto deco = p->function_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:22: missing z value for workgroup_size");
}

TEST_F(ParserImplTest, FunctionDecoration_Stage) {
  auto* p = parser("stage(compute)");
  auto deco = p->function_decoration();
  ASSERT_NE(deco, nullptr);
  ASSERT_FALSE(p->has_error());
  ASSERT_TRUE(deco->IsStage());
  EXPECT_EQ(deco->AsStage()->value(), ast::PipelineStage::kCompute);
}

TEST_F(ParserImplTest, FunctionDecoration_Stage_MissingValue) {
  auto* p = parser("stage()");
  auto deco = p->function_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:7: invalid value for stage decoration");
}

TEST_F(ParserImplTest, FunctionDecoration_Stage_MissingInvalid) {
  auto* p = parser("stage(nan)");
  auto deco = p->function_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:7: invalid value for stage decoration");
}

TEST_F(ParserImplTest, FunctionDecoration_Stage_MissingLeftParen) {
  auto* p = parser("stage compute)");
  auto deco = p->function_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:7: missing ( for stage decoration");
}

TEST_F(ParserImplTest, FunctionDecoration_Stage_MissingRightParen) {
  auto* p = parser("stage(compute");
  auto deco = p->function_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:14: missing ) for stage decoration");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
