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

TEST_F(ParserImplTest, TextureSamplerTypes_DepthTexture) {
  auto* p = parser("texture_depth_2d");
  auto* t = p->texture_sampler_types();
  ASSERT_NE(t, nullptr);
  ASSERT_TRUE(t->IsTexture());
  ASSERT_TRUE(t->AsTexture()->IsDepth());
  EXPECT_EQ(t->AsTexture()->dim(), ast::type::TextureDimension::k2d);
  EXPECT_FALSE(p->has_error());
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
