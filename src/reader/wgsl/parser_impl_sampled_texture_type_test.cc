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
#include "src/ast/type/texture_type.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, SampledTextureType_Invalid) {
  auto* p = parser("1234");
  auto t = p->sampled_texture_type();
  EXPECT_EQ(t, ast::type::TextureDimension::kNone);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, SampledTextureType_1d) {
  auto* p = parser("texture_sampled_1d");
  auto t = p->sampled_texture_type();
  EXPECT_EQ(t, ast::type::TextureDimension::k1d);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, SampledTextureType_1dArray) {
  auto* p = parser("texture_sampled_1d_array");
  auto t = p->sampled_texture_type();
  EXPECT_EQ(t, ast::type::TextureDimension::k1dArray);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, SampledTextureType_2d) {
  auto* p = parser("texture_sampled_2d");
  auto t = p->sampled_texture_type();
  EXPECT_EQ(t, ast::type::TextureDimension::k2d);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, SampledTextureType_2dArray) {
  auto* p = parser("texture_sampled_2d_array");
  auto t = p->sampled_texture_type();
  EXPECT_EQ(t, ast::type::TextureDimension::k2dArray);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, SampledTextureType_2dMs) {
  auto* p = parser("texture_sampled_2d_ms");
  auto t = p->sampled_texture_type();
  EXPECT_EQ(t, ast::type::TextureDimension::k2dMs);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, SampledTextureType_2dMsArray) {
  auto* p = parser("texture_sampled_2d_ms_array");
  auto t = p->sampled_texture_type();
  EXPECT_EQ(t, ast::type::TextureDimension::k2dMsArray);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, SampledTextureType_3d) {
  auto* p = parser("texture_sampled_3d");
  auto t = p->sampled_texture_type();
  EXPECT_EQ(t, ast::type::TextureDimension::k3d);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, SampledTextureType_Cube) {
  auto* p = parser("texture_sampled_cube");
  auto t = p->sampled_texture_type();
  EXPECT_EQ(t, ast::type::TextureDimension::kCube);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, SampledTextureType_kCubeArray) {
  auto* p = parser("texture_sampled_cube_array");
  auto t = p->sampled_texture_type();
  EXPECT_EQ(t, ast::type::TextureDimension::kCubeArray);
  EXPECT_FALSE(p->has_error());
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
