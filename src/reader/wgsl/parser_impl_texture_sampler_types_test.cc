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
#include "src/ast/type/sampled_texture_type.h"
#include "src/ast/type/sampler_type.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, TextureSamplerTypes_Invalid) {
  auto* p = parser("1234");
  auto* t = p->texture_sampler_types();
  EXPECT_EQ(t, nullptr);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, TextureSamplerTypes_Sampler) {
  auto* p = parser("sampler");
  auto* t = p->texture_sampler_types();
  ASSERT_NE(t, nullptr);
  ASSERT_TRUE(t->IsSampler());
  ASSERT_FALSE(t->AsSampler()->IsComparison());
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, TextureSamplerTypes_SamplerComparison) {
  auto* p = parser("sampler_comparison");
  auto* t = p->texture_sampler_types();
  ASSERT_NE(t, nullptr);
  ASSERT_TRUE(t->IsSampler());
  ASSERT_TRUE(t->AsSampler()->IsComparison());
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, TextureSamplerTypes_DepthTexture) {
  auto* p = parser("texture_depth_2d");
  auto* t = p->texture_sampler_types();
  ASSERT_NE(t, nullptr);
  ASSERT_TRUE(t->IsTexture());
  ASSERT_TRUE(t->AsTexture()->IsDepth());
  EXPECT_EQ(t->AsTexture()->dim(), ast::type::TextureDimension::k2d);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, TextureSamplerTypes_SampledTexture_F32) {
  auto* p = parser("texture_sampled_1d<f32>");
  auto* t = p->texture_sampler_types();
  ASSERT_NE(t, nullptr);
  ASSERT_TRUE(t->IsTexture());
  ASSERT_TRUE(t->AsTexture()->IsSampled());
  ASSERT_TRUE(t->AsTexture()->AsSampled()->type()->IsF32());
  EXPECT_EQ(t->AsTexture()->dim(), ast::type::TextureDimension::k1d);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, TextureSamplerTypes_SampledTexture_I32) {
  auto* p = parser("texture_sampled_2d<i32>");
  auto* t = p->texture_sampler_types();
  ASSERT_NE(t, nullptr);
  ASSERT_TRUE(t->IsTexture());
  ASSERT_TRUE(t->AsTexture()->IsSampled());
  ASSERT_TRUE(t->AsTexture()->AsSampled()->type()->IsI32());
  EXPECT_EQ(t->AsTexture()->dim(), ast::type::TextureDimension::k2d);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, TextureSamplerTypes_SampledTexture_U32) {
  auto* p = parser("texture_sampled_3d<u32>");
  auto* t = p->texture_sampler_types();
  ASSERT_NE(t, nullptr);
  ASSERT_TRUE(t->IsTexture());
  ASSERT_TRUE(t->AsTexture()->IsSampled());
  ASSERT_TRUE(t->AsTexture()->AsSampled()->type()->IsU32());
  EXPECT_EQ(t->AsTexture()->dim(), ast::type::TextureDimension::k3d);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, TextureSamplerTypes_SampledTexture_Invalid) {
  auto* p = parser("texture_sampled_1d<abc>");
  auto* t = p->texture_sampler_types();
  EXPECT_EQ(t, nullptr);
  EXPECT_EQ(p->error(), "1:20: unknown type alias 'abc'");
}

TEST_F(ParserImplTest, TextureSamplerTypes_SampledTexture_MissingType) {
  auto* p = parser("texture_sampled_1d<>");
  auto* t = p->texture_sampler_types();
  EXPECT_EQ(t, nullptr);
  EXPECT_EQ(p->error(), "1:20: invalid subtype for sampled texture type");
}

TEST_F(ParserImplTest, TextureSamplerTypes_SampledTexture_MissingLessThan) {
  auto* p = parser("texture_sampled_1d");
  auto* t = p->texture_sampler_types();
  EXPECT_EQ(t, nullptr);
  EXPECT_EQ(p->error(), "1:19: missing '<' for sampled texture type");
}

TEST_F(ParserImplTest, TextureSamplerTypes_SampledTexture_MissingGreaterThan) {
  auto* p = parser("texture_sampled_1d<u32");
  auto* t = p->texture_sampler_types();
  EXPECT_EQ(t, nullptr);
  EXPECT_EQ(p->error(), "1:23: missing '>' for sampled texture type");
}

TEST_F(ParserImplTest, TextureSamplerTypes_StorageTexture_Readonly1dR8Unorm) {
  auto* p = parser("texture_ro_1d<r8unorm>");
  auto* t = p->texture_sampler_types();
  ASSERT_NE(t, nullptr);
  ASSERT_TRUE(t->IsTexture());
  ASSERT_TRUE(t->AsTexture()->IsStorage());
  EXPECT_EQ(t->AsTexture()->AsStorage()->image_format(),
            ast::type::ImageFormat::kR8Unorm);
  EXPECT_EQ(t->AsTexture()->AsStorage()->access(),
            ast::type::StorageAccess::kRead);
  EXPECT_EQ(t->AsTexture()->dim(), ast::type::TextureDimension::k1d);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, TextureSamplerTypes_StorageTexture_Writeonly2dR16Float) {
  auto* p = parser("texture_wo_2d<r16float>");
  auto* t = p->texture_sampler_types();
  ASSERT_NE(t, nullptr);
  ASSERT_TRUE(t->IsTexture());
  ASSERT_TRUE(t->AsTexture()->IsStorage());
  EXPECT_EQ(t->AsTexture()->AsStorage()->image_format(),
            ast::type::ImageFormat::kR16Float);
  EXPECT_EQ(t->AsTexture()->AsStorage()->access(),
            ast::type::StorageAccess::kWrite);
  EXPECT_EQ(t->AsTexture()->dim(), ast::type::TextureDimension::k2d);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, TextureSamplerTypes_StorageTexture_InvalidType) {
  auto* p = parser("texture_ro_1d<abc>");
  auto* t = p->texture_sampler_types();
  EXPECT_EQ(t, nullptr);
  EXPECT_EQ(p->error(), "1:15: invalid format for storage texture type");
}

TEST_F(ParserImplTest, TextureSamplerTypes_StorageTexture_MissingType) {
  auto* p = parser("texture_wo_1d<>");
  auto* t = p->texture_sampler_types();
  EXPECT_EQ(t, nullptr);
  EXPECT_EQ(p->error(), "1:15: invalid format for storage texture type");
}

TEST_F(ParserImplTest, TextureSamplerTypes_StorageTexture_MissingLessThan) {
  auto* p = parser("texture_ro_1d");
  auto* t = p->texture_sampler_types();
  EXPECT_EQ(t, nullptr);
  EXPECT_EQ(p->error(), "1:14: missing '<' for storage texture type");
}

TEST_F(ParserImplTest, TextureSamplerTypes_StorageTexture_MissingGreaterThan) {
  auto* p = parser("texture_wo_1d<r8unorm");
  auto* t = p->texture_sampler_types();
  EXPECT_EQ(t, nullptr);
  EXPECT_EQ(p->error(), "1:22: missing '>' for storage texture type");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
