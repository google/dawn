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

#include "src/ast/type/depth_texture_type.h"

#include "gtest/gtest.h"

namespace tint {
namespace ast {
namespace type {
namespace {

using DepthTextureTypeTest = testing::Test;

TEST_F(DepthTextureTypeTest, Is) {
  DepthTextureType d(TextureDimension::kCube);
  EXPECT_FALSE(d.IsAlias());
  EXPECT_FALSE(d.IsArray());
  EXPECT_FALSE(d.IsBool());
  EXPECT_FALSE(d.IsF32());
  EXPECT_FALSE(d.IsI32());
  EXPECT_FALSE(d.IsMatrix());
  EXPECT_FALSE(d.IsPointer());
  EXPECT_FALSE(d.IsSampler());
  EXPECT_FALSE(d.IsStruct());
  EXPECT_TRUE(d.IsTexture());
  EXPECT_FALSE(d.IsU32());
  EXPECT_FALSE(d.IsVector());
}

TEST_F(DepthTextureTypeTest, IsTextureType) {
  DepthTextureType d(TextureDimension::kCube);
  EXPECT_TRUE(d.IsDepth());
  EXPECT_FALSE(d.IsSampled());
  EXPECT_FALSE(d.IsStorage());
}

TEST_F(DepthTextureTypeTest, Dim) {
  DepthTextureType d(TextureDimension::kCube);
  EXPECT_EQ(d.dim(), TextureDimension::kCube);
}

TEST_F(DepthTextureTypeTest, TypeName) {
  DepthTextureType d(TextureDimension::kCube);
  EXPECT_EQ(d.type_name(), "__depth_texture_cube");
}

}  // namespace
}  // namespace type
}  // namespace ast
}  // namespace tint
