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

#include "src/ast/vector.h"
#include "src/ast/access_control.h"
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
#include "src/ast/texture.h"
#include "src/ast/u32.h"

namespace tint {
namespace ast {
namespace {

using AstVectorTest = TestHelper;

TEST_F(AstVectorTest, Creation) {
  auto* i32 = create<I32>();
  auto* v = create<Vector>(i32, 2);
  EXPECT_EQ(v->type(), i32);
  EXPECT_EQ(v->size(), 2u);
}

TEST_F(AstVectorTest, Is) {
  auto* i32 = create<I32>();
  Type* ty = create<Vector>(i32, 4);
  EXPECT_FALSE(ty->Is<AccessControl>());
  EXPECT_FALSE(ty->Is<Alias>());
  EXPECT_FALSE(ty->Is<Array>());
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

TEST_F(AstVectorTest, TypeName) {
  auto* i32 = create<I32>();
  auto* v = create<Vector>(i32, 3);
  EXPECT_EQ(v->type_name(), "__vec_3__i32");
}

TEST_F(AstVectorTest, FriendlyName) {
  auto* v = ty.vec3<f32>();
  EXPECT_EQ(v->FriendlyName(Symbols()), "vec3<f32>");
}

}  // namespace
}  // namespace ast
}  // namespace tint
