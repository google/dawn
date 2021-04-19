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

#include "src/type/access_control_type.h"
#include "src/type/test_helper.h"
#include "src/type/texture_type.h"

namespace tint {
namespace type {
namespace {

using VectorTest = TestHelper;

TEST_F(VectorTest, Creation) {
  I32 i32;
  Vector v{&i32, 2};
  EXPECT_EQ(v.type(), &i32);
  EXPECT_EQ(v.size(), 2u);
}

TEST_F(VectorTest, Is) {
  I32 i32;
  Vector v{&i32, 4};
  Type* ty = &v;
  EXPECT_FALSE(ty->Is<AccessControl>());
  EXPECT_FALSE(ty->Is<Alias>());
  EXPECT_FALSE(ty->Is<ArrayType>());
  EXPECT_FALSE(ty->Is<Bool>());
  EXPECT_FALSE(ty->Is<F32>());
  EXPECT_FALSE(ty->Is<I32>());
  EXPECT_FALSE(ty->Is<Matrix>());
  EXPECT_FALSE(ty->Is<Pointer>());
  EXPECT_FALSE(ty->Is<Sampler>());
  EXPECT_FALSE(ty->Is<Struct>());
  EXPECT_FALSE(ty->Is<Texture>());
  EXPECT_FALSE(ty->Is<U32>());
  EXPECT_TRUE(ty->Is<Vector>());
}

TEST_F(VectorTest, TypeName) {
  I32 i32;
  Vector v{&i32, 3};
  EXPECT_EQ(v.type_name(), "__vec_3__i32");
}

TEST_F(VectorTest, FriendlyName) {
  auto* v = ty.vec3<f32>();
  EXPECT_EQ(v->FriendlyName(Symbols()), "vec3<f32>");
}

}  // namespace
}  // namespace type
}  // namespace tint
