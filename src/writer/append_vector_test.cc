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

#include "src/writer/append_vector.h"
#include "src/program_builder.h"
#include "src/resolver/resolver.h"

#include "gtest/gtest.h"

namespace tint {
namespace writer {
namespace {

class AppendVectorTest : public ::testing::Test, public ProgramBuilder {};

TEST_F(AppendVectorTest, Vec2i32_i32) {
  auto* scalar_1 = Expr(1);
  auto* scalar_2 = Expr(2);
  auto* scalar_3 = Expr(3);
  auto* vec_12 = vec2<i32>(scalar_1, scalar_2);
  WrapInFunction(vec_12, scalar_3);

  resolver::Resolver resolver(this);
  ASSERT_TRUE(resolver.Resolve()) << resolver.error();

  auto* vec_123 = AppendVector(this, vec_12, scalar_3)
                      ->As<ast::TypeConstructorExpression>();
  ASSERT_NE(vec_123, nullptr);
  ASSERT_EQ(vec_123->values().size(), 3u);
  EXPECT_EQ(vec_123->values()[0], scalar_1);
  EXPECT_EQ(vec_123->values()[1], scalar_2);
  EXPECT_EQ(vec_123->values()[2], scalar_3);
}

TEST_F(AppendVectorTest, Vec2i32_f32) {
  auto* scalar_1 = Expr(1);
  auto* scalar_2 = Expr(2);
  auto* scalar_3 = Expr(3.0f);
  auto* vec_12 = vec2<i32>(scalar_1, scalar_2);
  WrapInFunction(vec_12, scalar_3);

  resolver::Resolver resolver(this);
  ASSERT_TRUE(resolver.Resolve()) << resolver.error();

  auto* vec_123 = AppendVector(this, vec_12, scalar_3)
                      ->As<ast::TypeConstructorExpression>();
  ASSERT_NE(vec_123, nullptr);
  ASSERT_EQ(vec_123->values().size(), 3u);
  EXPECT_EQ(vec_123->values()[0], scalar_1);
  EXPECT_EQ(vec_123->values()[1], scalar_2);
  auto* f32_to_i32 = vec_123->values()[2]->As<ast::TypeConstructorExpression>();
  ASSERT_NE(f32_to_i32, nullptr);
  ASSERT_EQ(f32_to_i32->values().size(), 1u);
  EXPECT_EQ(f32_to_i32->values()[0], scalar_3);
}

TEST_F(AppendVectorTest, Vec3i32_i32) {
  auto* scalar_1 = Expr(1);
  auto* scalar_2 = Expr(2);
  auto* scalar_3 = Expr(3);
  auto* scalar_4 = Expr(3);
  auto* vec_123 = vec3<i32>(scalar_1, scalar_2, scalar_3);
  WrapInFunction(vec_123, scalar_4);

  resolver::Resolver resolver(this);
  ASSERT_TRUE(resolver.Resolve()) << resolver.error();

  auto* vec_1234 = AppendVector(this, vec_123, scalar_4)
                       ->As<ast::TypeConstructorExpression>();
  ASSERT_NE(vec_1234, nullptr);
  ASSERT_EQ(vec_1234->values().size(), 4u);
  EXPECT_EQ(vec_1234->values()[0], scalar_1);
  EXPECT_EQ(vec_1234->values()[1], scalar_2);
  EXPECT_EQ(vec_1234->values()[2], scalar_3);
  EXPECT_EQ(vec_1234->values()[3], scalar_4);
}

TEST_F(AppendVectorTest, Vec2i32Var_i32) {
  Global("vec_12", ty.vec2<i32>(), ast::StorageClass::kInput);
  auto* vec_12 = Expr("vec_12");
  auto* scalar_3 = Expr(3);
  WrapInFunction(vec_12, scalar_3);

  resolver::Resolver resolver(this);
  ASSERT_TRUE(resolver.Resolve()) << resolver.error();

  auto* vec_123 = AppendVector(this, vec_12, scalar_3)
                      ->As<ast::TypeConstructorExpression>();
  ASSERT_NE(vec_123, nullptr);
  ASSERT_EQ(vec_123->values().size(), 2u);
  EXPECT_EQ(vec_123->values()[0], vec_12);
  EXPECT_EQ(vec_123->values()[1], scalar_3);
}

TEST_F(AppendVectorTest, Vec2i32_i32Var) {
  Global("scalar_3", ty.i32(), ast::StorageClass::kInput);
  auto* scalar_1 = Expr(1);
  auto* scalar_2 = Expr(2);
  auto* scalar_3 = Expr("scalar_3");
  auto* vec_12 = vec2<i32>(scalar_1, scalar_2);
  WrapInFunction(vec_12, scalar_3);

  resolver::Resolver resolver(this);
  ASSERT_TRUE(resolver.Resolve()) << resolver.error();

  auto* vec_123 = AppendVector(this, vec_12, scalar_3)
                      ->As<ast::TypeConstructorExpression>();
  ASSERT_NE(vec_123, nullptr);
  ASSERT_EQ(vec_123->values().size(), 3u);
  EXPECT_EQ(vec_123->values()[0], scalar_1);
  EXPECT_EQ(vec_123->values()[1], scalar_2);
  EXPECT_EQ(vec_123->values()[2], scalar_3);
}

TEST_F(AppendVectorTest, Vec2i32Var_i32Var) {
  Global("vec_12", ty.vec2<i32>(), ast::StorageClass::kInput);
  Global("scalar_3", ty.i32(), ast::StorageClass::kInput);
  auto* vec_12 = Expr("vec_12");
  auto* scalar_3 = Expr("scalar_3");
  WrapInFunction(vec_12, scalar_3);

  resolver::Resolver resolver(this);
  ASSERT_TRUE(resolver.Resolve()) << resolver.error();

  auto* vec_123 = AppendVector(this, vec_12, scalar_3)
                      ->As<ast::TypeConstructorExpression>();
  ASSERT_NE(vec_123, nullptr);
  ASSERT_EQ(vec_123->values().size(), 2u);
  EXPECT_EQ(vec_123->values()[0], vec_12);
  EXPECT_EQ(vec_123->values()[1], scalar_3);
}

TEST_F(AppendVectorTest, Vec2i32Var_f32Var) {
  Global("vec_12", ty.vec2<i32>(), ast::StorageClass::kInput);
  Global("scalar_3", ty.f32(), ast::StorageClass::kInput);
  auto* vec_12 = Expr("vec_12");
  auto* scalar_3 = Expr("scalar_3");
  WrapInFunction(vec_12, scalar_3);

  resolver::Resolver resolver(this);
  ASSERT_TRUE(resolver.Resolve()) << resolver.error();

  auto* vec_123 = AppendVector(this, vec_12, scalar_3)
                      ->As<ast::TypeConstructorExpression>();
  ASSERT_NE(vec_123, nullptr);
  ASSERT_EQ(vec_123->values().size(), 2u);
  EXPECT_EQ(vec_123->values()[0], vec_12);
  auto* f32_to_i32 = vec_123->values()[1]->As<ast::TypeConstructorExpression>();
  ASSERT_NE(f32_to_i32, nullptr);
  ASSERT_EQ(f32_to_i32->values().size(), 1u);
  EXPECT_EQ(f32_to_i32->values()[0], scalar_3);
}

}  // namespace
}  // namespace writer
}  // namespace tint
