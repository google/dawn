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

#include "src/tint/resolver/resolver.h"

#include "gtest/gtest.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/sem/expression.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using Scalar = sem::Constant::Scalar;

using ResolverConstantsTest = ResolverTest;

TEST_F(ResolverConstantsTest, Scalar_i32) {
    auto* expr = Expr(99_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Type()->Is<sem::I32>());
    EXPECT_EQ(sem->ConstantValue().Type(), sem->Type());
    EXPECT_EQ(sem->ConstantValue().ElementType(), sem->Type());
    ASSERT_EQ(sem->ConstantValue().Elements().size(), 1u);
    EXPECT_EQ(sem->ConstantValue().Element<AInt>(0).value, 99);
}

TEST_F(ResolverConstantsTest, Scalar_u32) {
    auto* expr = Expr(99_u);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Type()->Is<sem::U32>());
    EXPECT_EQ(sem->ConstantValue().Type(), sem->Type());
    EXPECT_EQ(sem->ConstantValue().ElementType(), sem->Type());
    ASSERT_EQ(sem->ConstantValue().Elements().size(), 1u);
    EXPECT_EQ(sem->ConstantValue().Element<AInt>(0).value, 99u);
}

TEST_F(ResolverConstantsTest, Scalar_f32) {
    auto* expr = Expr(9.9_f);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Type()->Is<sem::F32>());
    EXPECT_EQ(sem->ConstantValue().Type(), sem->Type());
    EXPECT_EQ(sem->ConstantValue().ElementType(), sem->Type());
    ASSERT_EQ(sem->ConstantValue().Elements().size(), 1u);
    EXPECT_EQ(sem->ConstantValue().Element<AFloat>(0).value, 9.9f);
}

TEST_F(ResolverConstantsTest, Scalar_bool) {
    auto* expr = Expr(true);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Type()->Is<sem::Bool>());
    EXPECT_EQ(sem->ConstantValue().Type(), sem->Type());
    EXPECT_EQ(sem->ConstantValue().ElementType(), sem->Type());
    ASSERT_EQ(sem->ConstantValue().Elements().size(), 1u);
    EXPECT_EQ(sem->ConstantValue().Element<bool>(0), true);
}

TEST_F(ResolverConstantsTest, Vec3_ZeroInit_i32) {
    auto* expr = vec3<i32>();
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    ASSERT_TRUE(sem->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->Type()->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_EQ(sem->Type()->As<sem::Vector>()->Width(), 3u);
    EXPECT_EQ(sem->ConstantValue().Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue().ElementType()->Is<sem::I32>());
    ASSERT_EQ(sem->ConstantValue().Elements().size(), 3u);
    EXPECT_EQ(sem->ConstantValue().Element<AInt>(0).value, 0);
    EXPECT_EQ(sem->ConstantValue().Element<AInt>(1).value, 0);
    EXPECT_EQ(sem->ConstantValue().Element<AInt>(2).value, 0);
}

TEST_F(ResolverConstantsTest, Vec3_ZeroInit_u32) {
    auto* expr = vec3<u32>();
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    ASSERT_TRUE(sem->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->Type()->As<sem::Vector>()->type()->Is<sem::U32>());
    EXPECT_EQ(sem->Type()->As<sem::Vector>()->Width(), 3u);
    EXPECT_EQ(sem->ConstantValue().Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue().ElementType()->Is<sem::U32>());
    ASSERT_EQ(sem->ConstantValue().Elements().size(), 3u);
    EXPECT_EQ(sem->ConstantValue().Element<AInt>(0).value, 0u);
    EXPECT_EQ(sem->ConstantValue().Element<AInt>(1).value, 0u);
    EXPECT_EQ(sem->ConstantValue().Element<AInt>(2).value, 0u);
}

TEST_F(ResolverConstantsTest, Vec3_ZeroInit_f32) {
    auto* expr = vec3<f32>();
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    ASSERT_TRUE(sem->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->Type()->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(sem->Type()->As<sem::Vector>()->Width(), 3u);
    EXPECT_EQ(sem->ConstantValue().Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue().ElementType()->Is<sem::F32>());
    ASSERT_EQ(sem->ConstantValue().Elements().size(), 3u);
    EXPECT_EQ(sem->ConstantValue().Element<AFloat>(0).value, 0.0);
    EXPECT_EQ(sem->ConstantValue().Element<AFloat>(1).value, 0.0);
    EXPECT_EQ(sem->ConstantValue().Element<AFloat>(2).value, 0.0);
}

TEST_F(ResolverConstantsTest, Vec3_ZeroInit_bool) {
    auto* expr = vec3<bool>();
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    ASSERT_TRUE(sem->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->Type()->As<sem::Vector>()->type()->Is<sem::Bool>());
    EXPECT_EQ(sem->Type()->As<sem::Vector>()->Width(), 3u);
    EXPECT_EQ(sem->ConstantValue().Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue().ElementType()->Is<sem::Bool>());
    ASSERT_EQ(sem->ConstantValue().Elements().size(), 3u);
    EXPECT_EQ(sem->ConstantValue().Element<bool>(0), false);
    EXPECT_EQ(sem->ConstantValue().Element<bool>(1), false);
    EXPECT_EQ(sem->ConstantValue().Element<bool>(2), false);
}

TEST_F(ResolverConstantsTest, Vec3_Splat_i32) {
    auto* expr = vec3<i32>(99_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    ASSERT_TRUE(sem->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->Type()->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_EQ(sem->Type()->As<sem::Vector>()->Width(), 3u);
    EXPECT_EQ(sem->ConstantValue().Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue().ElementType()->Is<sem::I32>());
    ASSERT_EQ(sem->ConstantValue().Elements().size(), 3u);
    EXPECT_EQ(sem->ConstantValue().Element<AInt>(0).value, 99);
    EXPECT_EQ(sem->ConstantValue().Element<AInt>(1).value, 99);
    EXPECT_EQ(sem->ConstantValue().Element<AInt>(2).value, 99);
}

TEST_F(ResolverConstantsTest, Vec3_Splat_u32) {
    auto* expr = vec3<u32>(99_u);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    ASSERT_TRUE(sem->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->Type()->As<sem::Vector>()->type()->Is<sem::U32>());
    EXPECT_EQ(sem->Type()->As<sem::Vector>()->Width(), 3u);
    EXPECT_EQ(sem->ConstantValue().Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue().ElementType()->Is<sem::U32>());
    ASSERT_EQ(sem->ConstantValue().Elements().size(), 3u);
    EXPECT_EQ(sem->ConstantValue().Element<AInt>(0).value, 99u);
    EXPECT_EQ(sem->ConstantValue().Element<AInt>(1).value, 99u);
    EXPECT_EQ(sem->ConstantValue().Element<AInt>(2).value, 99u);
}

TEST_F(ResolverConstantsTest, Vec3_Splat_f32) {
    auto* expr = vec3<f32>(9.9_f);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    ASSERT_TRUE(sem->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->Type()->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(sem->Type()->As<sem::Vector>()->Width(), 3u);
    EXPECT_EQ(sem->ConstantValue().Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue().ElementType()->Is<sem::F32>());
    ASSERT_EQ(sem->ConstantValue().Elements().size(), 3u);
    EXPECT_EQ(sem->ConstantValue().Element<AFloat>(0).value, 9.9f);
    EXPECT_EQ(sem->ConstantValue().Element<AFloat>(1).value, 9.9f);
    EXPECT_EQ(sem->ConstantValue().Element<AFloat>(2).value, 9.9f);
}

TEST_F(ResolverConstantsTest, Vec3_Splat_bool) {
    auto* expr = vec3<bool>(true);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    ASSERT_TRUE(sem->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->Type()->As<sem::Vector>()->type()->Is<sem::Bool>());
    EXPECT_EQ(sem->Type()->As<sem::Vector>()->Width(), 3u);
    EXPECT_EQ(sem->ConstantValue().Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue().ElementType()->Is<sem::Bool>());
    ASSERT_EQ(sem->ConstantValue().Elements().size(), 3u);
    EXPECT_EQ(sem->ConstantValue().Element<bool>(0), true);
    EXPECT_EQ(sem->ConstantValue().Element<bool>(1), true);
    EXPECT_EQ(sem->ConstantValue().Element<bool>(2), true);
}

TEST_F(ResolverConstantsTest, Vec3_FullConstruct_i32) {
    auto* expr = vec3<i32>(1_i, 2_i, 3_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    ASSERT_TRUE(sem->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->Type()->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_EQ(sem->Type()->As<sem::Vector>()->Width(), 3u);
    EXPECT_EQ(sem->ConstantValue().Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue().ElementType()->Is<sem::I32>());
    ASSERT_EQ(sem->ConstantValue().Elements().size(), 3u);
    EXPECT_EQ(sem->ConstantValue().Element<AInt>(0).value, 1);
    EXPECT_EQ(sem->ConstantValue().Element<AInt>(1).value, 2);
    EXPECT_EQ(sem->ConstantValue().Element<AInt>(2).value, 3);
}

TEST_F(ResolverConstantsTest, Vec3_FullConstruct_u32) {
    auto* expr = vec3<u32>(1_u, 2_u, 3_u);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    ASSERT_TRUE(sem->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->Type()->As<sem::Vector>()->type()->Is<sem::U32>());
    EXPECT_EQ(sem->Type()->As<sem::Vector>()->Width(), 3u);
    EXPECT_EQ(sem->ConstantValue().Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue().ElementType()->Is<sem::U32>());
    ASSERT_EQ(sem->ConstantValue().Elements().size(), 3u);
    EXPECT_EQ(sem->ConstantValue().Element<AInt>(0).value, 1);
    EXPECT_EQ(sem->ConstantValue().Element<AInt>(1).value, 2);
    EXPECT_EQ(sem->ConstantValue().Element<AInt>(2).value, 3);
}

TEST_F(ResolverConstantsTest, Vec3_FullConstruct_f32) {
    auto* expr = vec3<f32>(1_f, 2_f, 3_f);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    ASSERT_TRUE(sem->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->Type()->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(sem->Type()->As<sem::Vector>()->Width(), 3u);
    EXPECT_EQ(sem->ConstantValue().Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue().ElementType()->Is<sem::F32>());
    ASSERT_EQ(sem->ConstantValue().Elements().size(), 3u);
    EXPECT_EQ(sem->ConstantValue().Element<AFloat>(0).value, 1.f);
    EXPECT_EQ(sem->ConstantValue().Element<AFloat>(1).value, 2.f);
    EXPECT_EQ(sem->ConstantValue().Element<AFloat>(2).value, 3.f);
}

TEST_F(ResolverConstantsTest, Vec3_FullConstruct_bool) {
    auto* expr = vec3<bool>(true, false, true);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    ASSERT_TRUE(sem->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->Type()->As<sem::Vector>()->type()->Is<sem::Bool>());
    EXPECT_EQ(sem->Type()->As<sem::Vector>()->Width(), 3u);
    EXPECT_EQ(sem->ConstantValue().Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue().ElementType()->Is<sem::Bool>());
    ASSERT_EQ(sem->ConstantValue().Elements().size(), 3u);
    EXPECT_EQ(sem->ConstantValue().Element<bool>(0), true);
    EXPECT_EQ(sem->ConstantValue().Element<bool>(1), false);
    EXPECT_EQ(sem->ConstantValue().Element<bool>(2), true);
}

TEST_F(ResolverConstantsTest, Vec3_MixConstruct_i32) {
    auto* expr = vec3<i32>(1_i, vec2<i32>(2_i, 3_i));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    ASSERT_TRUE(sem->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->Type()->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_EQ(sem->Type()->As<sem::Vector>()->Width(), 3u);
    EXPECT_EQ(sem->ConstantValue().Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue().ElementType()->Is<sem::I32>());
    ASSERT_EQ(sem->ConstantValue().Elements().size(), 3u);
    EXPECT_EQ(sem->ConstantValue().Element<AInt>(0).value, 1);
    EXPECT_EQ(sem->ConstantValue().Element<AInt>(1).value, 2);
    EXPECT_EQ(sem->ConstantValue().Element<AInt>(2).value, 3);
}

TEST_F(ResolverConstantsTest, Vec3_MixConstruct_u32) {
    auto* expr = vec3<u32>(vec2<u32>(1_u, 2_u), 3_u);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    ASSERT_TRUE(sem->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->Type()->As<sem::Vector>()->type()->Is<sem::U32>());
    EXPECT_EQ(sem->Type()->As<sem::Vector>()->Width(), 3u);
    EXPECT_EQ(sem->ConstantValue().Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue().ElementType()->Is<sem::U32>());
    ASSERT_EQ(sem->ConstantValue().Elements().size(), 3u);
    EXPECT_EQ(sem->ConstantValue().Element<AInt>(0).value, 1);
    EXPECT_EQ(sem->ConstantValue().Element<AInt>(1).value, 2);
    EXPECT_EQ(sem->ConstantValue().Element<AInt>(2).value, 3);
}

TEST_F(ResolverConstantsTest, Vec3_MixConstruct_f32) {
    auto* expr = vec3<f32>(1_f, vec2<f32>(2_f, 3_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    ASSERT_TRUE(sem->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->Type()->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(sem->Type()->As<sem::Vector>()->Width(), 3u);
    EXPECT_EQ(sem->ConstantValue().Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue().ElementType()->Is<sem::F32>());
    ASSERT_EQ(sem->ConstantValue().Elements().size(), 3u);
    EXPECT_EQ(sem->ConstantValue().Element<AFloat>(0).value, 1.f);
    EXPECT_EQ(sem->ConstantValue().Element<AFloat>(1).value, 2.f);
    EXPECT_EQ(sem->ConstantValue().Element<AFloat>(2).value, 3.f);
}

TEST_F(ResolverConstantsTest, Vec3_MixConstruct_bool) {
    auto* expr = vec3<bool>(vec2<bool>(true, false), true);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    ASSERT_TRUE(sem->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->Type()->As<sem::Vector>()->type()->Is<sem::Bool>());
    EXPECT_EQ(sem->Type()->As<sem::Vector>()->Width(), 3u);
    EXPECT_EQ(sem->ConstantValue().Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue().ElementType()->Is<sem::Bool>());
    ASSERT_EQ(sem->ConstantValue().Elements().size(), 3u);
    EXPECT_EQ(sem->ConstantValue().Element<bool>(0), true);
    EXPECT_EQ(sem->ConstantValue().Element<bool>(1), false);
    EXPECT_EQ(sem->ConstantValue().Element<bool>(2), true);
}

TEST_F(ResolverConstantsTest, Vec3_Cast_f32_to_32) {
    auto* expr = vec3<i32>(vec3<f32>(1.1_f, 2.2_f, 3.3_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    ASSERT_TRUE(sem->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->Type()->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_EQ(sem->Type()->As<sem::Vector>()->Width(), 3u);
    EXPECT_EQ(sem->ConstantValue().Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue().ElementType()->Is<sem::I32>());
    ASSERT_EQ(sem->ConstantValue().Elements().size(), 3u);
    EXPECT_EQ(sem->ConstantValue().Element<AInt>(0).value, 1);
    EXPECT_EQ(sem->ConstantValue().Element<AInt>(1).value, 2);
    EXPECT_EQ(sem->ConstantValue().Element<AInt>(2).value, 3);
}

TEST_F(ResolverConstantsTest, Vec3_Cast_u32_to_f32) {
    auto* expr = vec3<f32>(vec3<u32>(10_u, 20_u, 30_u));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    ASSERT_TRUE(sem->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->Type()->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(sem->Type()->As<sem::Vector>()->Width(), 3u);
    EXPECT_EQ(sem->ConstantValue().Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue().ElementType()->Is<sem::F32>());
    ASSERT_EQ(sem->ConstantValue().Elements().size(), 3u);
    EXPECT_EQ(sem->ConstantValue().Element<AFloat>(0).value, 10.f);
    EXPECT_EQ(sem->ConstantValue().Element<AFloat>(1).value, 20.f);
    EXPECT_EQ(sem->ConstantValue().Element<AFloat>(2).value, 30.f);
}

}  // namespace
}  // namespace tint::resolver
