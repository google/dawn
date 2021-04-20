// Copyright 2021 The Tint Authors.
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

#include "src/ast/texture.h"

#include "src/ast/alias.h"
#include "src/ast/array.h"
#include "src/ast/bool.h"
#include "src/ast/f32.h"
#include "src/ast/i32.h"
#include "src/ast/matrix.h"
#include "src/ast/pointer.h"
#include "src/ast/sampler.h"
#include "src/ast/struct.h"
#include "src/ast/test_helper.h"
#include "src/ast/u32.h"
#include "src/ast/vector.h"

namespace tint {
namespace ast {
namespace {

using AstTextureTypeTest = TestHelper;

TEST_F(AstTextureTypeTest, IsTextureArray) {
  EXPECT_EQ(false, IsTextureArray(TextureDimension::kNone));
  EXPECT_EQ(false, IsTextureArray(TextureDimension::k1d));
  EXPECT_EQ(false, IsTextureArray(TextureDimension::k2d));
  EXPECT_EQ(true, IsTextureArray(TextureDimension::k2dArray));
  EXPECT_EQ(false, IsTextureArray(TextureDimension::k3d));
  EXPECT_EQ(false, IsTextureArray(TextureDimension::kCube));
  EXPECT_EQ(true, IsTextureArray(TextureDimension::kCubeArray));
}

TEST_F(AstTextureTypeTest, NumCoordinateAxes) {
  EXPECT_EQ(0, NumCoordinateAxes(TextureDimension::kNone));
  EXPECT_EQ(1, NumCoordinateAxes(TextureDimension::k1d));
  EXPECT_EQ(2, NumCoordinateAxes(TextureDimension::k2d));
  EXPECT_EQ(2, NumCoordinateAxes(TextureDimension::k2dArray));
  EXPECT_EQ(3, NumCoordinateAxes(TextureDimension::k3d));
  EXPECT_EQ(3, NumCoordinateAxes(TextureDimension::kCube));
  EXPECT_EQ(3, NumCoordinateAxes(TextureDimension::kCubeArray));
}

}  // namespace
}  // namespace ast
}  // namespace tint
