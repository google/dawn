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

TEST_F(AppendVectorTest, Vec2i32_u32) {
  auto* scalar_1 = Expr(1);
  auto* scalar_2 = Expr(2);
  auto* scalar_3 = Expr(3u);
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
  auto* u32_to_i32 = vec_123->values()[2]->As<ast::TypeConstructorExpression>();
  ASSERT_NE(u32_to_i32, nullptr);
  EXPECT_TRUE(u32_to_i32->type()->Is<ast::I32>());
  ASSERT_EQ(u32_to_i32->values().size(), 1u);
  EXPECT_EQ(u32_to_i32->values()[0], scalar_3);
}

TEST_F(AppendVectorTest, Vec2i32FromVec2u32_u32) {
  auto* scalar_1 = Expr(1u);
  auto* scalar_2 = Expr(2u);
  auto* scalar_3 = Expr(3u);
  auto* uvec_12 = vec2<u32>(scalar_1, scalar_2);
  auto* vec_12 = vec2<i32>(uvec_12);
  WrapInFunction(vec_12, scalar_3);

  resolver::Resolver resolver(this);
  ASSERT_TRUE(resolver.Resolve()) << resolver.error();

  auto* vec_123 = AppendVector(this, vec_12, scalar_3)
                      ->As<ast::TypeConstructorExpression>();
  ASSERT_NE(vec_123, nullptr);
  ASSERT_EQ(vec_123->values().size(), 2u);
  auto* v2u32_to_v2i32 =
      vec_123->values()[0]->As<ast::TypeConstructorExpression>();
  ASSERT_NE(v2u32_to_v2i32, nullptr);
  EXPECT_TRUE(v2u32_to_v2i32->type()->is_signed_integer_vector());
  EXPECT_EQ(v2u32_to_v2i32->values().size(), 1u);
  EXPECT_EQ(v2u32_to_v2i32->values()[0], uvec_12);

  auto* u32_to_i32 = vec_123->values()[1]->As<ast::TypeConstructorExpression>();
  ASSERT_NE(u32_to_i32, nullptr);
  EXPECT_TRUE(u32_to_i32->type()->Is<ast::I32>());
  ASSERT_EQ(u32_to_i32->values().size(), 1u);
  EXPECT_EQ(u32_to_i32->values()[0], scalar_3);
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
  EXPECT_TRUE(f32_to_i32->type()->Is<ast::I32>());
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
  Global("vec_12", ty.vec2<i32>(), ast::StorageClass::kPrivate);
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
  Global("scalar_3", ty.i32(), ast::StorageClass::kPrivate);
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
  Global("vec_12", ty.vec2<i32>(), ast::StorageClass::kPrivate);
  Global("scalar_3", ty.i32(), ast::StorageClass::kPrivate);
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
  Global("vec_12", ty.vec2<i32>(), ast::StorageClass::kPrivate);
  Global("scalar_3", ty.f32(), ast::StorageClass::kPrivate);
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
  EXPECT_TRUE(f32_to_i32->type()->Is<ast::I32>());
  ASSERT_EQ(f32_to_i32->values().size(), 1u);
  EXPECT_EQ(f32_to_i32->values()[0], scalar_3);
}

TEST_F(AppendVectorTest, Vec2boolVar_boolVar) {
  Global("vec_12", ty.vec2<bool>(), ast::StorageClass::kPrivate);
  Global("scalar_3", ty.bool_(), ast::StorageClass::kPrivate);
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

TEST_F(AppendVectorTest, ZeroVec3i32_i32) {
  auto* scalar = Expr(4);
  auto* vec000 = vec3<i32>();
  WrapInFunction(vec000, scalar);

  resolver::Resolver resolver(this);
  ASSERT_TRUE(resolver.Resolve()) << resolver.error();

  auto* vec_0004 =
      AppendVector(this, vec000, scalar)->As<ast::TypeConstructorExpression>();
  ASSERT_NE(vec_0004, nullptr);
  ASSERT_EQ(vec_0004->values().size(), 4u);
  for (size_t i = 0; i < 3; i++) {
    auto* ctor = vec_0004->values()[i]->As<ast::ScalarConstructorExpression>();
    ASSERT_NE(ctor, nullptr);
    auto* literal = As<ast::SintLiteral>(ctor->literal());
    ASSERT_NE(literal, nullptr);
    EXPECT_EQ(literal->value(), 0);
  }
  EXPECT_EQ(vec_0004->values()[3], scalar);
}

}  // namespace
}  // namespace writer
}  // namespace tint
