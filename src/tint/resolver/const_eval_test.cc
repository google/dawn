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

#include <cmath>
#include <type_traits>

#include "gtest/gtest.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/sem/builtin_type.h"
#include "src/tint/sem/expression.h"
#include "src/tint/sem/index_accessor_expression.h"
#include "src/tint/sem/member_accessor_expression.h"
#include "src/tint/sem/test_helper.h"
#include "src/tint/utils/transform.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

template <typename T>
const auto kHighest = T(T::kHighest);

template <typename T>
const auto kLowest = T(T::kLowest);

template <typename T>
const auto kNaN = T(std::numeric_limits<UnwrapNumber<T>>::quiet_NaN());

template <typename T>
const auto kInf = T(std::numeric_limits<UnwrapNumber<T>>::infinity());

template <typename T>
const auto kPi = T(UnwrapNumber<T>(3.14159265358979323846));

template <typename T>
const auto kPiOver2 = T(UnwrapNumber<T>(1.57079632679489661923));

template <typename T>
const auto kPiOver4 = T(UnwrapNumber<T>(0.785398163397448309616));

template <typename T>
const auto k3PiOver4 = T(UnwrapNumber<T>(2.356194490192344928846));

template <typename T>
constexpr auto Negate(const Number<T>& v) {
    // For signed integrals, avoid C++ UB by not negating the smallest negative number. In
    // WGSL, this operation is well defined to return the same value, see:
    // https://gpuweb.github.io/gpuweb/wgsl/#arithmetic-expr.
    if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
        if (v == std::numeric_limits<T>::min()) {
            return v;
        }
    }
    return -v;
}

template <typename T>
auto Abs(const Number<T>& v) {
    if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>) {
        return v;
    } else {
        return Number<T>(std::abs(v));
    }
}

// Concats any number of std::vectors
template <typename Vec, typename... Vecs>
auto Concat(Vec&& v1, Vecs&&... vs) {
    auto total_size = v1.size() + (vs.size() + ...);
    v1.reserve(total_size);
    (std::move(vs.begin(), vs.end(), std::back_inserter(v1)), ...);
    return std::move(v1);
}

using ResolverConstEvalTest = ResolverTest;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Construction
////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(ResolverConstEvalTest, Scalar_i32) {
    auto* expr = Expr(99_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Type()->Is<sem::I32>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    EXPECT_EQ(sem->ConstantValue()->As<AInt>(), 99);
}

TEST_F(ResolverConstEvalTest, Scalar_u32) {
    auto* expr = Expr(99_u);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Type()->Is<sem::U32>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    EXPECT_EQ(sem->ConstantValue()->As<AInt>(), 99u);
}

TEST_F(ResolverConstEvalTest, Scalar_f32) {
    auto* expr = Expr(9.9_f);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Type()->Is<sem::F32>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    EXPECT_EQ(sem->ConstantValue()->As<AFloat>().value, 9.9f);
}

TEST_F(ResolverConstEvalTest, Scalar_f16) {
    Enable(ast::Extension::kF16);

    auto* expr = Expr(9.9_h);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Type()->Is<sem::F16>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    // 9.9 is not exactly representable by f16, and should be quantized to 9.8984375
    EXPECT_EQ(sem->ConstantValue()->As<AFloat>(), 9.8984375f);
}

TEST_F(ResolverConstEvalTest, Scalar_bool) {
    auto* expr = Expr(true);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Type()->Is<sem::Bool>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    EXPECT_EQ(sem->ConstantValue()->As<bool>(), true);
}

TEST_F(ResolverConstEvalTest, Vec3_ZeroInit_i32) {
    auto* expr = vec3<i32>();
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::I32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AInt>(), 0);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AInt>(), 0);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AInt>(), 0);
}

TEST_F(ResolverConstEvalTest, Vec3_ZeroInit_u32) {
    auto* expr = vec3<u32>();
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::U32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AInt>(), 0u);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AInt>(), 0u);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AInt>(), 0u);
}

TEST_F(ResolverConstEvalTest, Vec3_ZeroInit_f32) {
    auto* expr = vec3<f32>();
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AFloat>(), 0._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AFloat>(), 0._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AFloat>(), 0._a);
}

TEST_F(ResolverConstEvalTest, Vec3_ZeroInit_f16) {
    Enable(ast::Extension::kF16);

    auto* expr = vec3<f16>();
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AFloat>(), 0._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AFloat>(), 0._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AFloat>(), 0._a);
}

TEST_F(ResolverConstEvalTest, Vec3_ZeroInit_bool) {
    auto* expr = vec3<bool>();
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::Bool>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<bool>(), false);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<bool>(), false);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<bool>(), false);
}

TEST_F(ResolverConstEvalTest, Vec3_Splat_i32) {
    auto* expr = vec3<i32>(99_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::I32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AInt>(), 99);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AInt>(), 99);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AInt>(), 99);
}

TEST_F(ResolverConstEvalTest, Vec3_Splat_u32) {
    auto* expr = vec3<u32>(99_u);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::U32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AInt>(), 99u);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AInt>(), 99u);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AInt>(), 99u);
}

TEST_F(ResolverConstEvalTest, Vec3_Splat_f32) {
    auto* expr = vec3<f32>(9.9_f);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AFloat>(), 9.9f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AFloat>(), 9.9f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AFloat>(), 9.9f);
}

TEST_F(ResolverConstEvalTest, Vec3_Splat_f16) {
    Enable(ast::Extension::kF16);

    auto* expr = vec3<f16>(9.9_h);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    // 9.9 is not exactly representable by f16, and should be quantized to 9.8984375

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AFloat>(), 9.8984375f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AFloat>(), 9.8984375f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AFloat>(), 9.8984375f);
}

TEST_F(ResolverConstEvalTest, Vec3_Splat_bool) {
    auto* expr = vec3<bool>(true);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::Bool>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<bool>(), true);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<bool>(), true);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<bool>(), true);
}

TEST_F(ResolverConstEvalTest, Vec3_FullConstruct_i32) {
    auto* expr = vec3<i32>(1_i, 2_i, 3_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::I32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AInt>(), 1);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AInt>(), 2);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AInt>(), 3);
}

TEST_F(ResolverConstEvalTest, Vec3_FullConstruct_u32) {
    auto* expr = vec3<u32>(1_u, 2_u, 3_u);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::U32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AInt>(), 1);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AInt>(), 2);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AInt>(), 3);
}

TEST_F(ResolverConstEvalTest, Vec3_FullConstruct_f32) {
    auto* expr = vec3<f32>(1_f, 2_f, 3_f);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AFloat>(), 1.f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AFloat>(), 2.f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AFloat>(), 3.f);
}

TEST_F(ResolverConstEvalTest, Vec3_FullConstruct_f16) {
    Enable(ast::Extension::kF16);

    auto* expr = vec3<f16>(1_h, 2_h, 3_h);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AFloat>(), 1.f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AFloat>(), 2.f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AFloat>(), 3.f);
}

TEST_F(ResolverConstEvalTest, Vec3_FullConstruct_bool) {
    auto* expr = vec3<bool>(true, false, true);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::Bool>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<bool>(), true);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<bool>(), false);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<bool>(), true);
}

TEST_F(ResolverConstEvalTest, Vec3_MixConstruct_i32) {
    auto* expr = vec3<i32>(1_i, vec2<i32>(2_i, 3_i));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::I32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AInt>(), 1);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AInt>(), 2);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AInt>(), 3);
}

TEST_F(ResolverConstEvalTest, Vec3_MixConstruct_u32) {
    auto* expr = vec3<u32>(vec2<u32>(1_u, 2_u), 3_u);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::U32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AInt>(), 1);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AInt>(), 2);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AInt>(), 3);
}

TEST_F(ResolverConstEvalTest, Vec3_MixConstruct_f32) {
    auto* expr = vec3<f32>(1_f, vec2<f32>(2_f, 3_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AFloat>(), 1.f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AFloat>(), 2.f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AFloat>(), 3.f);
}

TEST_F(ResolverConstEvalTest, Vec3_MixConstruct_f32_all_10) {
    auto* expr = vec3<f32>(10_f, vec2<f32>(10_f, 10_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<f32>(), 10_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<f32>(), 10_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<f32>(), 10_f);
}

TEST_F(ResolverConstEvalTest, Vec3_MixConstruct_f32_all_positive_0) {
    auto* expr = vec3<f32>(0_f, vec2<f32>(0_f, 0_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<f32>(), 0_f);
}

TEST_F(ResolverConstEvalTest, Vec3_MixConstruct_f32_all_negative_0) {
    auto* expr = vec3<f32>(vec2<f32>(-0_f, -0_f), -0_f);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<f32>(), -0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<f32>(), -0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<f32>(), -0_f);
}

TEST_F(ResolverConstEvalTest, Vec3_MixConstruct_f32_mixed_sign_0) {
    auto* expr = vec3<f32>(0_f, vec2<f32>(-0_f, 0_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<f32>(), -0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<f32>(), 0_f);
}

TEST_F(ResolverConstEvalTest, Vec3_MixConstruct_f16) {
    Enable(ast::Extension::kF16);

    auto* expr = vec3<f16>(1_h, vec2<f16>(2_h, 3_h));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AFloat>(), 1.f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AFloat>(), 2.f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AFloat>(), 3.f);
}

TEST_F(ResolverConstEvalTest, Vec3_MixConstruct_f16_all_10) {
    Enable(ast::Extension::kF16);

    auto* expr = vec3<f16>(10_h, vec2<f16>(10_h, 10_h));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<f16>(), 10_h);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<f16>(), 10_h);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<f16>(), 10_h);
}

TEST_F(ResolverConstEvalTest, Vec3_MixConstruct_f16_all_positive_0) {
    Enable(ast::Extension::kF16);

    auto* expr = vec3<f16>(0_h, vec2<f16>(0_h, 0_h));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<f16>(), 0_h);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<f16>(), 0_h);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<f16>(), 0_h);
}

TEST_F(ResolverConstEvalTest, Vec3_MixConstruct_f16_all_negative_0) {
    Enable(ast::Extension::kF16);

    auto* expr = vec3<f16>(vec2<f16>(-0_h, -0_h), -0_h);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<f16>(), -0_h);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<f16>(), -0_h);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<f16>(), -0_h);
}

TEST_F(ResolverConstEvalTest, Vec3_MixConstruct_f16_mixed_sign_0) {
    Enable(ast::Extension::kF16);

    auto* expr = vec3<f16>(0_h, vec2<f16>(-0_h, 0_h));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<f16>(), 0_h);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<f16>(), -0_h);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<f16>(), 0_h);
}

TEST_F(ResolverConstEvalTest, Vec3_MixConstruct_bool) {
    auto* expr = vec3<bool>(vec2<bool>(true, false), true);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::Bool>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<bool>(), true);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<bool>(), false);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<bool>(), true);
}

TEST_F(ResolverConstEvalTest, Vec3_MixConstruct_all_true) {
    auto* expr = vec3<bool>(true, vec2<bool>(true, true));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::Bool>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<bool>(), true);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<bool>(), true);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<bool>(), true);
}

TEST_F(ResolverConstEvalTest, Vec3_MixConstruct_all_false) {
    auto* expr = vec3<bool>(false, vec2<bool>(false, false));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::Bool>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<bool>(), false);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<bool>(), false);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<bool>(), false);
}

TEST_F(ResolverConstEvalTest, Vec3_Convert_f32_to_i32) {
    auto* expr = vec3<i32>(vec3<f32>(1.1_f, 2.2_f, 3.3_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::I32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AInt>(), 1);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AInt>(), 2);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AInt>(), 3);
}

TEST_F(ResolverConstEvalTest, Vec3_Convert_u32_to_f32) {
    auto* expr = vec3<f32>(vec3<u32>(10_u, 20_u, 30_u));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AFloat>(), 10.f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AFloat>(), 20.f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AFloat>(), 30.f);
}

TEST_F(ResolverConstEvalTest, Vec3_Convert_f16_to_i32) {
    Enable(ast::Extension::kF16);

    auto* expr = vec3<i32>(vec3<f16>(1.1_h, 2.2_h, 3.3_h));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::I32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AInt>(), 1_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AInt>(), 2_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AInt>(), 3_i);
}

TEST_F(ResolverConstEvalTest, Vec3_Convert_u32_to_f16) {
    Enable(ast::Extension::kF16);

    auto* expr = vec3<f16>(vec3<u32>(10_u, 20_u, 30_u));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AFloat>(), 10.f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AFloat>(), 20.f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AFloat>(), 30.f);
}

TEST_F(ResolverConstEvalTest, Vec3_Convert_Large_f32_to_i32) {
    auto* expr = vec3<i32>(vec3<f32>(1e10_f, -1e20_f, 1e30_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::I32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AInt>(), i32::kHighest);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AInt>(), i32::kLowest);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AInt>(), i32::kHighest);
}

TEST_F(ResolverConstEvalTest, Vec3_Convert_Large_f32_to_u32) {
    auto* expr = vec3<u32>(vec3<f32>(1e10_f, -1e20_f, 1e30_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::U32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AInt>(), u32::kHighest);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AInt>(), u32::kLowest);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AInt>(), u32::kHighest);
}

TEST_F(ResolverConstEvalTest, Vec3_Convert_Large_f32_to_f16) {
    Enable(ast::Extension::kF16);

    auto* expr = vec3<f16>(vec3<f32>(1e10_f, -1e20_f, 1e30_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    constexpr auto kInfinity = std::numeric_limits<double>::infinity();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AFloat>(), kInfinity);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AFloat>(), -kInfinity);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AFloat>(), kInfinity);
}

TEST_F(ResolverConstEvalTest, Vec3_Convert_Small_f32_to_f16) {
    Enable(ast::Extension::kF16);

    auto* expr = vec3<f16>(vec3<f32>(1e-20_f, -2e-30_f, 3e-40_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AFloat>(), 0.0);
    EXPECT_FALSE(std::signbit(sem->ConstantValue()->Index(0)->As<AFloat>().value));

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AFloat>(), -0.0);
    EXPECT_TRUE(std::signbit(sem->ConstantValue()->Index(1)->As<AFloat>().value));

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AFloat>(), 0.0);
    EXPECT_FALSE(std::signbit(sem->ConstantValue()->Index(2)->As<AFloat>().value));
}

TEST_F(ResolverConstEvalTest, Mat2x3_ZeroInit_f32) {
    auto* expr = mat2x3<f32>();
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* mat = sem->Type()->As<sem::Matrix>();
    ASSERT_NE(mat, nullptr);
    EXPECT_TRUE(mat->type()->Is<sem::F32>());
    EXPECT_EQ(mat->columns(), 2u);
    EXPECT_EQ(mat->rows(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->As<f32>(), 0._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->As<f32>(), 0._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(2)->As<f32>(), 0._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->As<f32>(), 0._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->As<f32>(), 0._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(2)->As<f32>(), 0._f);
}

TEST_F(ResolverConstEvalTest, Mat2x3_ZeroInit_f16) {
    Enable(ast::Extension::kF16);

    auto* expr = mat2x3<f16>();
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* mat = sem->Type()->As<sem::Matrix>();
    ASSERT_NE(mat, nullptr);
    EXPECT_TRUE(mat->type()->Is<sem::F16>());
    EXPECT_EQ(mat->columns(), 2u);
    EXPECT_EQ(mat->rows(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->As<f16>(), 0._h);

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->As<f16>(), 0._h);

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(2)->As<f16>(), 0._h);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->As<f16>(), 0._h);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->As<f16>(), 0._h);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(2)->As<f16>(), 0._h);
}

TEST_F(ResolverConstEvalTest, Mat3x2_Construct_Scalars_af) {
    auto* expr = Construct(ty.mat(nullptr, 3, 2), 1.0_a, 2.0_a, 3.0_a, 4.0_a, 5.0_a, 6.0_a);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* mat = sem->Type()->As<sem::Matrix>();
    ASSERT_NE(mat, nullptr);
    EXPECT_TRUE(mat->type()->Is<sem::F32>());
    EXPECT_EQ(mat->columns(), 3u);
    EXPECT_EQ(mat->rows(), 2u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->As<AFloat>(), 1._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->As<AFloat>(), 2._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->As<AFloat>(), 3._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->As<AFloat>(), 4._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(0)->As<AFloat>(), 5._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(1)->As<AFloat>(), 6._a);
}

TEST_F(ResolverConstEvalTest, Mat3x2_Construct_Columns_af) {
    auto* expr = Construct(ty.mat(nullptr, 3, 2),           //
                           vec(nullptr, 2u, 1.0_a, 2.0_a),  //
                           vec(nullptr, 2u, 3.0_a, 4.0_a),  //
                           vec(nullptr, 2u, 5.0_a, 6.0_a));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* mat = sem->Type()->As<sem::Matrix>();
    ASSERT_NE(mat, nullptr);
    EXPECT_TRUE(mat->type()->Is<sem::F32>());
    EXPECT_EQ(mat->columns(), 3u);
    EXPECT_EQ(mat->rows(), 2u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->As<AFloat>(), 1._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->As<AFloat>(), 2._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->As<AFloat>(), 3._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->As<AFloat>(), 4._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(0)->As<AFloat>(), 5._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(1)->As<AFloat>(), 6._a);
}

TEST_F(ResolverConstEvalTest, Array_i32_Zero) {
    auto* expr = Construct(ty.array<i32, 4>());
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<sem::Array>();
    ASSERT_NE(arr, nullptr);
    EXPECT_TRUE(arr->ElemType()->Is<sem::I32>());
    EXPECT_EQ(arr->Count(), 4u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<i32>(), 0_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<i32>(), 0_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<i32>(), 0_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(3)->As<i32>(), 0_i);
}

TEST_F(ResolverConstEvalTest, Array_f32_Zero) {
    auto* expr = Construct(ty.array<f32, 4>());
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<sem::Array>();
    ASSERT_NE(arr, nullptr);
    EXPECT_TRUE(arr->ElemType()->Is<sem::F32>());
    EXPECT_EQ(arr->Count(), 4u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(3)->As<f32>(), 0_f);
}

TEST_F(ResolverConstEvalTest, Array_vec3_f32_Zero) {
    auto* expr = Construct(ty.array(ty.vec3<f32>(), 2_u));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<sem::Array>();
    ASSERT_NE(arr, nullptr);
    EXPECT_TRUE(arr->ElemType()->Is<sem::Vector>());
    EXPECT_EQ(arr->Count(), 2u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->As<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->As<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(2)->As<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->As<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->As<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(2)->As<f32>(), 0_f);
}

TEST_F(ResolverConstEvalTest, Array_Struct_f32_Zero) {
    Structure("S", {
                       Member("m1", ty.f32()),
                       Member("m2", ty.f32()),
                   });
    auto* expr = Construct(ty.array(ty.type_name("S"), 2_u));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<sem::Array>();
    ASSERT_NE(arr, nullptr);
    EXPECT_TRUE(arr->ElemType()->Is<sem::Struct>());
    EXPECT_EQ(arr->Count(), 2u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->As<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->As<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->As<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->As<f32>(), 0_f);
}

TEST_F(ResolverConstEvalTest, Array_i32_Elements) {
    auto* expr = Construct(ty.array<i32, 4>(), 10_i, 20_i, 30_i, 40_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<sem::Array>();
    ASSERT_NE(arr, nullptr);
    EXPECT_TRUE(arr->ElemType()->Is<sem::I32>());
    EXPECT_EQ(arr->Count(), 4u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<i32>(), 10_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<i32>(), 20_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<i32>(), 30_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(3)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(3)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(3)->As<i32>(), 40_i);
}

TEST_F(ResolverConstEvalTest, Array_f32_Elements) {
    auto* expr = Construct(ty.array<f32, 4>(), 10_f, 20_f, 30_f, 40_f);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<sem::Array>();
    ASSERT_NE(arr, nullptr);
    EXPECT_TRUE(arr->ElemType()->Is<sem::F32>());
    EXPECT_EQ(arr->Count(), 4u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<f32>(), 10_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<f32>(), 20_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<f32>(), 30_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(3)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(3)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(3)->As<f32>(), 40_f);
}

TEST_F(ResolverConstEvalTest, Array_vec3_f32_Elements) {
    auto* expr = Construct(ty.array(ty.vec3<f32>(), 2_u),  //
                           vec3<f32>(1_f, 2_f, 3_f), vec3<f32>(4_f, 5_f, 6_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<sem::Array>();
    ASSERT_NE(arr, nullptr);
    EXPECT_TRUE(arr->ElemType()->Is<sem::Vector>());
    EXPECT_EQ(arr->Count(), 2u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->As<f32>(), 1_f);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->As<f32>(), 2_f);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(2)->As<f32>(), 3_f);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->As<f32>(), 4_f);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->As<f32>(), 5_f);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(2)->As<f32>(), 6_f);
}

TEST_F(ResolverConstEvalTest, Array_Struct_f32_Elements) {
    Structure("S", {
                       Member("m1", ty.f32()),
                       Member("m2", ty.f32()),
                   });
    auto* expr = Construct(ty.array(ty.type_name("S"), 2_u),        //
                           Construct(ty.type_name("S"), 1_f, 2_f),  //
                           Construct(ty.type_name("S"), 3_f, 4_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<sem::Array>();
    ASSERT_NE(arr, nullptr);
    EXPECT_TRUE(arr->ElemType()->Is<sem::Struct>());
    EXPECT_EQ(arr->Count(), 2u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->As<f32>(), 1_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->As<f32>(), 2_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->As<f32>(), 3_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->As<f32>(), 4_f);
}

TEST_F(ResolverConstEvalTest, Struct_I32s_ZeroInit) {
    Structure("S", {Member("m1", ty.i32()), Member("m2", ty.i32()), Member("m3", ty.i32())});
    auto* expr = Construct(ty.type_name("S"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* str = sem->Type()->As<sem::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().size(), 3u);
    ASSERT_NE(sem->ConstantValue(), nullptr);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->Is<sem::I32>());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<i32>(), 0_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->Is<sem::I32>());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<i32>(), 0_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Type()->Is<sem::I32>());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<i32>(), 0_i);
}

TEST_F(ResolverConstEvalTest, Struct_MixedScalars_ZeroInit) {
    Enable(ast::Extension::kF16);

    Structure("S", {
                       Member("m1", ty.i32()),
                       Member("m2", ty.u32()),
                       Member("m3", ty.f32()),
                       Member("m4", ty.f16()),
                       Member("m5", ty.bool_()),
                   });
    auto* expr = Construct(ty.type_name("S"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* str = sem->Type()->As<sem::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().size(), 5u);
    ASSERT_NE(sem->ConstantValue(), nullptr);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->Is<sem::I32>());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<i32>(), 0_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->Is<sem::U32>());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<u32>(), 0_u);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Type()->Is<sem::F32>());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<f32>(), 0._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->Type()->Is<sem::F16>());
    EXPECT_EQ(sem->ConstantValue()->Index(3)->As<f16>(), 0._h);

    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->Type()->Is<sem::Bool>());
    EXPECT_EQ(sem->ConstantValue()->Index(4)->As<bool>(), false);
}

TEST_F(ResolverConstEvalTest, Struct_VectorF32s_ZeroInit) {
    Structure("S", {
                       Member("m1", ty.vec3<f32>()),
                       Member("m2", ty.vec3<f32>()),
                       Member("m3", ty.vec3<f32>()),
                   });
    auto* expr = Construct(ty.type_name("S"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* str = sem->Type()->As<sem::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().size(), 3u);
    ASSERT_NE(sem->ConstantValue(), nullptr);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->As<f32>(), 0._f);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->As<f32>(), 0._f);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(2)->As<f32>(), 0._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->As<f32>(), 0._f);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->As<f32>(), 0._f);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(2)->As<f32>(), 0._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Type()->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(0)->As<f32>(), 0._f);
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(1)->As<f32>(), 0._f);
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(2)->As<f32>(), 0._f);
}

TEST_F(ResolverConstEvalTest, Struct_MixedVectors_ZeroInit) {
    Enable(ast::Extension::kF16);

    Structure("S", {
                       Member("m1", ty.vec2<i32>()),
                       Member("m2", ty.vec3<u32>()),
                       Member("m3", ty.vec4<f32>()),
                       Member("m4", ty.vec3<f16>()),
                       Member("m5", ty.vec2<bool>()),
                   });
    auto* expr = Construct(ty.type_name("S"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* str = sem->Type()->As<sem::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().size(), 5u);
    ASSERT_NE(sem->ConstantValue(), nullptr);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->As<i32>(), 0_i);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->As<i32>(), 0_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->As<sem::Vector>()->type()->Is<sem::U32>());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->As<u32>(), 0_u);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->As<u32>(), 0_u);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(2)->As<u32>(), 0_u);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Type()->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(0)->As<f32>(), 0._f);
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(1)->As<f32>(), 0._f);
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(2)->As<f32>(), 0._f);
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(3)->As<f32>(), 0._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->Type()->As<sem::Vector>()->type()->Is<sem::F16>());
    EXPECT_EQ(sem->ConstantValue()->Index(3)->Index(0)->As<f16>(), 0._h);
    EXPECT_EQ(sem->ConstantValue()->Index(3)->Index(1)->As<f16>(), 0._h);
    EXPECT_EQ(sem->ConstantValue()->Index(3)->Index(2)->As<f16>(), 0._h);

    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->Type()->As<sem::Vector>()->type()->Is<sem::Bool>());
    EXPECT_EQ(sem->ConstantValue()->Index(4)->Index(0)->As<bool>(), false);
    EXPECT_EQ(sem->ConstantValue()->Index(4)->Index(1)->As<bool>(), false);
}

TEST_F(ResolverConstEvalTest, Struct_Struct_ZeroInit) {
    Structure("Inner", {
                           Member("m1", ty.i32()),
                           Member("m2", ty.u32()),
                           Member("m3", ty.f32()),
                       });

    Structure("Outer", {
                           Member("m1", ty.type_name("Inner")),
                           Member("m2", ty.type_name("Inner")),
                       });
    auto* expr = Construct(ty.type_name("Outer"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* str = sem->Type()->As<sem::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().size(), 2u);
    ASSERT_NE(sem->ConstantValue(), nullptr);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->Is<sem::Struct>());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->As<i32>(), 0_i);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->As<u32>(), 0_u);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(2)->As<f32>(), 0_f);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->Is<sem::Struct>());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->As<i32>(), 0_i);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->As<u32>(), 0_u);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(2)->As<f32>(), 0_f);
}

TEST_F(ResolverConstEvalTest, Struct_MixedScalars_Construct) {
    Enable(ast::Extension::kF16);

    Structure("S", {
                       Member("m1", ty.i32()),
                       Member("m2", ty.u32()),
                       Member("m3", ty.f32()),
                       Member("m4", ty.f16()),
                       Member("m5", ty.bool_()),
                   });
    auto* expr = Construct(ty.type_name("S"), 1_i, 2_u, 3_f, 4_h, false);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* str = sem->Type()->As<sem::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().size(), 5u);
    ASSERT_NE(sem->ConstantValue(), nullptr);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->Is<sem::I32>());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<i32>(), 1_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->Is<sem::U32>());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<u32>(), 2_u);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Type()->Is<sem::F32>());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<f32>(), 3._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(3)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(3)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->Type()->Is<sem::F16>());
    EXPECT_EQ(sem->ConstantValue()->Index(3)->As<f16>(), 4._h);

    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->Type()->Is<sem::Bool>());
    EXPECT_EQ(sem->ConstantValue()->Index(4)->As<bool>(), false);
}

TEST_F(ResolverConstEvalTest, Struct_MixedVectors_Construct) {
    Enable(ast::Extension::kF16);

    Structure("S", {
                       Member("m1", ty.vec2<i32>()),
                       Member("m2", ty.vec3<u32>()),
                       Member("m3", ty.vec4<f32>()),
                       Member("m4", ty.vec3<f16>()),
                       Member("m5", ty.vec2<bool>()),
                   });
    auto* expr = Construct(ty.type_name("S"), vec2<i32>(1_i), vec3<u32>(2_u), vec4<f32>(3_f),
                           vec3<f16>(4_h), vec2<bool>(false));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* str = sem->Type()->As<sem::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().size(), 5u);
    ASSERT_NE(sem->ConstantValue(), nullptr);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->As<i32>(), 1_i);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->As<i32>(), 1_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->As<sem::Vector>()->type()->Is<sem::U32>());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->As<u32>(), 2_u);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->As<u32>(), 2_u);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(2)->As<u32>(), 2_u);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Type()->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(0)->As<f32>(), 3._f);
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(1)->As<f32>(), 3._f);
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(2)->As<f32>(), 3._f);
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(3)->As<f32>(), 3._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(3)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(3)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->Type()->As<sem::Vector>()->type()->Is<sem::F16>());
    EXPECT_EQ(sem->ConstantValue()->Index(3)->Index(0)->As<f16>(), 4._h);
    EXPECT_EQ(sem->ConstantValue()->Index(3)->Index(1)->As<f16>(), 4._h);
    EXPECT_EQ(sem->ConstantValue()->Index(3)->Index(2)->As<f16>(), 4._h);

    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->Type()->As<sem::Vector>()->type()->Is<sem::Bool>());
    EXPECT_EQ(sem->ConstantValue()->Index(4)->Index(0)->As<bool>(), false);
    EXPECT_EQ(sem->ConstantValue()->Index(4)->Index(1)->As<bool>(), false);
}

TEST_F(ResolverConstEvalTest, Struct_Struct_Construct) {
    Structure("Inner", {
                           Member("m1", ty.i32()),
                           Member("m2", ty.u32()),
                           Member("m3", ty.f32()),
                       });

    Structure("Outer", {
                           Member("m1", ty.type_name("Inner")),
                           Member("m2", ty.type_name("Inner")),
                       });
    auto* expr = Construct(ty.type_name("Outer"),  //
                           Construct(ty.type_name("Inner"), 1_i, 2_u, 3_f),
                           Construct(ty.type_name("Inner"), 4_i, 0_u, 6_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* str = sem->Type()->As<sem::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().size(), 2u);
    ASSERT_NE(sem->ConstantValue(), nullptr);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->Is<sem::Struct>());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->As<i32>(), 1_i);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->As<u32>(), 2_u);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(2)->As<f32>(), 3_f);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->Is<sem::Struct>());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->As<i32>(), 4_i);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->As<u32>(), 0_u);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(2)->As<f32>(), 6_f);
}

TEST_F(ResolverConstEvalTest, Struct_Array_Construct) {
    Structure("S", {
                       Member("m1", ty.array<i32, 2>()),
                       Member("m2", ty.array<f32, 3>()),
                   });
    auto* expr = Construct(ty.type_name("S"),  //
                           Construct(ty.array<i32, 2>(), 1_i, 2_i),
                           Construct(ty.array<f32, 3>(), 1_f, 2_f, 3_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* str = sem->Type()->As<sem::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().size(), 2u);
    ASSERT_NE(sem->ConstantValue(), nullptr);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->Is<sem::Array>());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->As<i32>(), 1_i);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->As<u32>(), 2_i);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->Is<sem::Array>());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->As<i32>(), 1_f);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->As<u32>(), 2_f);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(2)->As<f32>(), 3_f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Indexing
////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(ResolverConstEvalTest, Vec3_Index) {
    auto* expr = IndexAccessor(vec3<i32>(1_i, 2_i, 3_i), 2_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    ASSERT_TRUE(sem->Type()->Is<sem::I32>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    EXPECT_EQ(sem->ConstantValue()->As<i32>(), 3_i);
}

TEST_F(ResolverConstEvalTest, Vec3_Index_OOB_High) {
    auto* expr = IndexAccessor(vec3<i32>(1_i, 2_i, 3_i), Expr(Source{{12, 34}}, 3_i));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 warning: index 3 out of bounds [0..2]. Clamping index to 2");

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    ASSERT_TRUE(sem->Type()->Is<sem::I32>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    EXPECT_EQ(sem->ConstantValue()->As<i32>(), 3_i);
}

TEST_F(ResolverConstEvalTest, Vec3_Index_OOB_Low) {
    auto* expr = IndexAccessor(vec3<i32>(1_i, 2_i, 3_i), Expr(Source{{12, 34}}, -3_i));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 warning: index -3 out of bounds [0..2]. Clamping index to 0");

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    ASSERT_TRUE(sem->Type()->Is<sem::I32>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    EXPECT_EQ(sem->ConstantValue()->As<i32>(), 1_i);
}

TEST_F(ResolverConstEvalTest, Vec3_Swizzle_Scalar) {
    auto* expr = MemberAccessor(vec3<i32>(1_i, 2_i, 3_i), "y");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    ASSERT_TRUE(sem->Type()->Is<sem::I32>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    EXPECT_EQ(sem->ConstantValue()->As<i32>(), 2_i);
}

TEST_F(ResolverConstEvalTest, Vec3_Swizzle_Vector) {
    auto* expr = MemberAccessor(vec3<i32>(1_i, 2_i, 3_i), "zx");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_EQ(vec->Width(), 2u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<f32>(), 3._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<f32>(), 1._a);
}

TEST_F(ResolverConstEvalTest, Vec3_Swizzle_Chain) {
    auto* expr =  // (1, 2, 3) -> (2, 3, 1) -> (3, 2) -> 2
        MemberAccessor(MemberAccessor(MemberAccessor(vec3<i32>(1_i, 2_i, 3_i), "gbr"), "yx"), "y");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    ASSERT_TRUE(sem->Type()->Is<sem::I32>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    EXPECT_EQ(sem->ConstantValue()->As<i32>(), 2_i);
}

TEST_F(ResolverConstEvalTest, Mat3x2_Index) {
    auto* expr = IndexAccessor(
        mat3x2<f32>(vec2<f32>(1._a, 2._a), vec2<f32>(3._a, 4._a), vec2<f32>(5._a, 6._a)), 2_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_EQ(vec->Width(), 2u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<f32>(), 5._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<f32>(), 6._a);
}

TEST_F(ResolverConstEvalTest, Mat3x2_Index_OOB_High) {
    auto* expr = IndexAccessor(
        mat3x2<f32>(vec2<f32>(1._a, 2._a), vec2<f32>(3._a, 4._a), vec2<f32>(5._a, 6._a)),
        Expr(Source{{12, 34}}, 3_i));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 warning: index 3 out of bounds [0..2]. Clamping index to 2");

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_EQ(vec->Width(), 2u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<f32>(), 5._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<f32>(), 6._a);
}

TEST_F(ResolverConstEvalTest, Mat3x2_Index_OOB_Low) {
    auto* expr = IndexAccessor(
        mat3x2<f32>(vec2<f32>(1._a, 2._a), vec2<f32>(3._a, 4._a), vec2<f32>(5._a, 6._a)),
        Expr(Source{{12, 34}}, -3_i));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 warning: index -3 out of bounds [0..2]. Clamping index to 0");

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_EQ(vec->Width(), 2u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<f32>(), 1._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<f32>(), 2._a);
}

TEST_F(ResolverConstEvalTest, Array_vec3_f32_Index) {
    auto* expr = IndexAccessor(Construct(ty.array(ty.vec3<f32>(), 2_u),  //
                                         vec3<f32>(1_f, 2_f, 3_f), vec3<f32>(4_f, 5_f, 6_f)),
                               1_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<f32>(), 4_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<f32>(), 5_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<f32>(), 6_f);
}

TEST_F(ResolverConstEvalTest, Array_vec3_f32_Index_OOB_High) {
    auto* expr = IndexAccessor(Construct(ty.array(ty.vec3<f32>(), 2_u),  //
                                         vec3<f32>(1_f, 2_f, 3_f), vec3<f32>(4_f, 5_f, 6_f)),
                               Expr(Source{{12, 34}}, 2_i));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 warning: index 2 out of bounds [0..1]. Clamping index to 1");

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<f32>(), 4_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<f32>(), 5_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<f32>(), 6_f);
}

TEST_F(ResolverConstEvalTest, Array_vec3_f32_Index_OOB_Low) {
    auto* expr = IndexAccessor(Construct(ty.array(ty.vec3<f32>(), 2_u),  //
                                         vec3<f32>(1_f, 2_f, 3_f), vec3<f32>(4_f, 5_f, 6_f)),
                               Expr(Source{{12, 34}}, -2_i));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 warning: index -2 out of bounds [0..1]. Clamping index to 0");

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<f32>(), 1_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<f32>(), 2_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<f32>(), 3_f);
}

TEST_F(ResolverConstEvalTest, ChainedIndex) {
    auto* arr_expr = Construct(ty.array(ty.mat2x3<f32>(), 2_u),        // array<mat2x3<f32>, 2u>
                               mat2x3<f32>(vec3<f32>(1_f, 2_f, 3_f),   //
                                           vec3<f32>(4_f, 5_f, 6_f)),  //
                               mat2x3<f32>(vec3<f32>(7_f, 0_f, 9_f),   //
                                           vec3<f32>(10_f, 11_f, 12_f)));

    auto* mat_expr = IndexAccessor(arr_expr, 1_i);  // arr[1]
    auto* vec_expr = IndexAccessor(mat_expr, 0_i);  // arr[1][0]
    auto* f32_expr = IndexAccessor(vec_expr, 2_i);  // arr[1][0][2]
    WrapInFunction(f32_expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    {
        auto* mat = Sem().Get(mat_expr);
        EXPECT_NE(mat, nullptr);
        auto* ty = mat->Type()->As<sem::Matrix>();
        ASSERT_NE(mat->Type(), nullptr);
        EXPECT_TRUE(ty->ColumnType()->Is<sem::Vector>());
        EXPECT_EQ(ty->columns(), 2u);
        EXPECT_EQ(ty->rows(), 3u);
        EXPECT_EQ(mat->ConstantValue()->Type(), mat->Type());
        EXPECT_FALSE(mat->ConstantValue()->AllEqual());
        EXPECT_TRUE(mat->ConstantValue()->AnyZero());
        EXPECT_FALSE(mat->ConstantValue()->AllZero());

        EXPECT_TRUE(mat->ConstantValue()->Index(0)->Index(0)->AllEqual());
        EXPECT_FALSE(mat->ConstantValue()->Index(0)->Index(0)->AnyZero());
        EXPECT_FALSE(mat->ConstantValue()->Index(0)->Index(0)->AllZero());
        EXPECT_EQ(mat->ConstantValue()->Index(0)->Index(0)->As<f32>(), 7_f);

        EXPECT_TRUE(mat->ConstantValue()->Index(0)->Index(1)->AllEqual());
        EXPECT_TRUE(mat->ConstantValue()->Index(0)->Index(1)->AnyZero());
        EXPECT_TRUE(mat->ConstantValue()->Index(0)->Index(1)->AllZero());
        EXPECT_EQ(mat->ConstantValue()->Index(0)->Index(1)->As<f32>(), 0_f);

        EXPECT_TRUE(mat->ConstantValue()->Index(0)->Index(2)->AllEqual());
        EXPECT_FALSE(mat->ConstantValue()->Index(0)->Index(2)->AnyZero());
        EXPECT_FALSE(mat->ConstantValue()->Index(0)->Index(2)->AllZero());
        EXPECT_EQ(mat->ConstantValue()->Index(0)->Index(2)->As<f32>(), 9_f);

        EXPECT_TRUE(mat->ConstantValue()->Index(1)->Index(0)->AllEqual());
        EXPECT_FALSE(mat->ConstantValue()->Index(1)->Index(0)->AnyZero());
        EXPECT_FALSE(mat->ConstantValue()->Index(1)->Index(0)->AllZero());
        EXPECT_EQ(mat->ConstantValue()->Index(1)->Index(0)->As<f32>(), 10_f);

        EXPECT_TRUE(mat->ConstantValue()->Index(1)->Index(1)->AllEqual());
        EXPECT_FALSE(mat->ConstantValue()->Index(1)->Index(1)->AnyZero());
        EXPECT_FALSE(mat->ConstantValue()->Index(1)->Index(1)->AllZero());
        EXPECT_EQ(mat->ConstantValue()->Index(1)->Index(1)->As<f32>(), 11_f);

        EXPECT_TRUE(mat->ConstantValue()->Index(1)->Index(2)->AllEqual());
        EXPECT_FALSE(mat->ConstantValue()->Index(1)->Index(2)->AnyZero());
        EXPECT_FALSE(mat->ConstantValue()->Index(1)->Index(2)->AllZero());
        EXPECT_EQ(mat->ConstantValue()->Index(1)->Index(2)->As<f32>(), 12_f);
    }
    {
        auto* vec = Sem().Get(vec_expr);
        EXPECT_NE(vec, nullptr);
        auto* ty = vec->Type()->As<sem::Vector>();
        ASSERT_NE(vec->Type(), nullptr);
        EXPECT_TRUE(ty->type()->Is<sem::F32>());
        EXPECT_EQ(ty->Width(), 3u);
        EXPECT_EQ(vec->ConstantValue()->Type(), vec->Type());
        EXPECT_FALSE(vec->ConstantValue()->AllEqual());
        EXPECT_TRUE(vec->ConstantValue()->AnyZero());
        EXPECT_FALSE(vec->ConstantValue()->AllZero());

        EXPECT_TRUE(vec->ConstantValue()->Index(0)->AllEqual());
        EXPECT_FALSE(vec->ConstantValue()->Index(0)->AnyZero());
        EXPECT_FALSE(vec->ConstantValue()->Index(0)->AllZero());
        EXPECT_EQ(vec->ConstantValue()->Index(0)->As<f32>(), 7_f);

        EXPECT_TRUE(vec->ConstantValue()->Index(1)->AllEqual());
        EXPECT_TRUE(vec->ConstantValue()->Index(1)->AnyZero());
        EXPECT_TRUE(vec->ConstantValue()->Index(1)->AllZero());
        EXPECT_EQ(vec->ConstantValue()->Index(1)->As<f32>(), 0_f);

        EXPECT_TRUE(vec->ConstantValue()->Index(2)->AllEqual());
        EXPECT_FALSE(vec->ConstantValue()->Index(2)->AnyZero());
        EXPECT_FALSE(vec->ConstantValue()->Index(2)->AllZero());
        EXPECT_EQ(vec->ConstantValue()->Index(2)->As<f32>(), 9_f);
    }
    {
        auto* f = Sem().Get(f32_expr);
        EXPECT_NE(f, nullptr);
        EXPECT_TRUE(f->Type()->Is<sem::F32>());
        EXPECT_EQ(f->ConstantValue()->Type(), f->Type());
        EXPECT_TRUE(f->ConstantValue()->AllEqual());
        EXPECT_FALSE(f->ConstantValue()->AnyZero());
        EXPECT_FALSE(f->ConstantValue()->AllZero());
        EXPECT_EQ(f->ConstantValue()->As<f32>(), 9_f);
    }
}

TEST_F(ResolverConstEvalTest, ChainedIndex_OOB) {
    auto* arr_expr = Construct(ty.array(ty.mat2x3<f32>(), 2_u),        // array<mat2x3<f32>, 2u>
                               mat2x3<f32>(vec3<f32>(1_f, 2_f, 3_f),   //
                                           vec3<f32>(4_f, 5_f, 6_f)),  //
                               mat2x3<f32>(vec3<f32>(7_f, 8_f, 9_f),   //
                                           vec3<f32>(10_f, 11_f, 12_f)));

    auto* mat_expr = IndexAccessor(arr_expr, Expr(Source{{1, 2}}, -3_i));  // arr[3]
    auto* vec_expr = IndexAccessor(mat_expr, Expr(Source{{3, 4}}, -2_i));  // arr[3][-2]
    auto* f32_expr = IndexAccessor(vec_expr, Expr(Source{{5, 6}}, 4_i));   // arr[3][-2][4]
    WrapInFunction(f32_expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), R"(1:2 warning: index -3 out of bounds [0..1]. Clamping index to 0
3:4 warning: index -2 out of bounds [0..1]. Clamping index to 0
5:6 warning: index 4 out of bounds [0..2]. Clamping index to 2)");

    {
        auto* mat = Sem().Get(mat_expr);
        EXPECT_NE(mat, nullptr);
        auto* ty = mat->Type()->As<sem::Matrix>();
        ASSERT_NE(mat->Type(), nullptr);
        EXPECT_TRUE(ty->ColumnType()->Is<sem::Vector>());
        EXPECT_EQ(ty->columns(), 2u);
        EXPECT_EQ(ty->rows(), 3u);
        EXPECT_EQ(mat->ConstantValue()->Type(), mat->Type());
        EXPECT_FALSE(mat->ConstantValue()->AllEqual());
        EXPECT_FALSE(mat->ConstantValue()->AnyZero());
        EXPECT_FALSE(mat->ConstantValue()->AllZero());

        EXPECT_TRUE(mat->ConstantValue()->Index(0)->Index(0)->AllEqual());
        EXPECT_FALSE(mat->ConstantValue()->Index(0)->Index(0)->AnyZero());
        EXPECT_FALSE(mat->ConstantValue()->Index(0)->Index(0)->AllZero());
        EXPECT_EQ(mat->ConstantValue()->Index(0)->Index(0)->As<f32>(), 1_f);

        EXPECT_TRUE(mat->ConstantValue()->Index(0)->Index(1)->AllEqual());
        EXPECT_FALSE(mat->ConstantValue()->Index(0)->Index(1)->AnyZero());
        EXPECT_FALSE(mat->ConstantValue()->Index(0)->Index(1)->AllZero());
        EXPECT_EQ(mat->ConstantValue()->Index(0)->Index(1)->As<f32>(), 2_f);

        EXPECT_TRUE(mat->ConstantValue()->Index(0)->Index(2)->AllEqual());
        EXPECT_FALSE(mat->ConstantValue()->Index(0)->Index(2)->AnyZero());
        EXPECT_FALSE(mat->ConstantValue()->Index(0)->Index(2)->AllZero());
        EXPECT_EQ(mat->ConstantValue()->Index(0)->Index(2)->As<f32>(), 3_f);

        EXPECT_TRUE(mat->ConstantValue()->Index(1)->Index(0)->AllEqual());
        EXPECT_FALSE(mat->ConstantValue()->Index(1)->Index(0)->AnyZero());
        EXPECT_FALSE(mat->ConstantValue()->Index(1)->Index(0)->AllZero());
        EXPECT_EQ(mat->ConstantValue()->Index(1)->Index(0)->As<f32>(), 4_f);

        EXPECT_TRUE(mat->ConstantValue()->Index(1)->Index(1)->AllEqual());
        EXPECT_FALSE(mat->ConstantValue()->Index(1)->Index(1)->AnyZero());
        EXPECT_FALSE(mat->ConstantValue()->Index(1)->Index(1)->AllZero());
        EXPECT_EQ(mat->ConstantValue()->Index(1)->Index(1)->As<f32>(), 5_f);

        EXPECT_TRUE(mat->ConstantValue()->Index(1)->Index(2)->AllEqual());
        EXPECT_FALSE(mat->ConstantValue()->Index(1)->Index(2)->AnyZero());
        EXPECT_FALSE(mat->ConstantValue()->Index(1)->Index(2)->AllZero());
        EXPECT_EQ(mat->ConstantValue()->Index(1)->Index(2)->As<f32>(), 6_f);
    }
    {
        auto* vec = Sem().Get(vec_expr);
        EXPECT_NE(vec, nullptr);
        auto* ty = vec->Type()->As<sem::Vector>();
        ASSERT_NE(vec->Type(), nullptr);
        EXPECT_TRUE(ty->type()->Is<sem::F32>());
        EXPECT_EQ(ty->Width(), 3u);
        EXPECT_EQ(vec->ConstantValue()->Type(), vec->Type());
        EXPECT_FALSE(vec->ConstantValue()->AllEqual());
        EXPECT_FALSE(vec->ConstantValue()->AnyZero());
        EXPECT_FALSE(vec->ConstantValue()->AllZero());

        EXPECT_TRUE(vec->ConstantValue()->Index(0)->AllEqual());
        EXPECT_FALSE(vec->ConstantValue()->Index(0)->AnyZero());
        EXPECT_FALSE(vec->ConstantValue()->Index(0)->AllZero());
        EXPECT_EQ(vec->ConstantValue()->Index(0)->As<f32>(), 1_f);

        EXPECT_TRUE(vec->ConstantValue()->Index(1)->AllEqual());
        EXPECT_FALSE(vec->ConstantValue()->Index(1)->AnyZero());
        EXPECT_FALSE(vec->ConstantValue()->Index(1)->AllZero());
        EXPECT_EQ(vec->ConstantValue()->Index(1)->As<f32>(), 2_f);

        EXPECT_TRUE(vec->ConstantValue()->Index(2)->AllEqual());
        EXPECT_FALSE(vec->ConstantValue()->Index(2)->AnyZero());
        EXPECT_FALSE(vec->ConstantValue()->Index(2)->AllZero());
        EXPECT_EQ(vec->ConstantValue()->Index(2)->As<f32>(), 3_f);
    }
    {
        auto* f = Sem().Get(f32_expr);
        EXPECT_NE(f, nullptr);
        EXPECT_TRUE(f->Type()->Is<sem::F32>());
        EXPECT_EQ(f->ConstantValue()->Type(), f->Type());
        EXPECT_TRUE(f->ConstantValue()->AllEqual());
        EXPECT_FALSE(f->ConstantValue()->AnyZero());
        EXPECT_FALSE(f->ConstantValue()->AllZero());
        EXPECT_EQ(f->ConstantValue()->As<f32>(), 3_f);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Member accessing
////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(ResolverConstEvalTest, MemberAccess) {
    Structure("Inner", {
                           Member("i1", ty.i32()),
                           Member("i2", ty.u32()),
                           Member("i3", ty.f32()),
                       });

    Structure("Outer", {
                           Member("o1", ty.type_name("Inner")),
                           Member("o2", ty.type_name("Inner")),
                       });
    auto* outer_expr = Construct(ty.type_name("Outer"),  //
                                 Construct(ty.type_name("Inner"), 1_i, 2_u, 3_f),
                                 Construct(ty.type_name("Inner")));
    auto* o1_expr = MemberAccessor(outer_expr, "o1");
    auto* i2_expr = MemberAccessor(o1_expr, "i2");
    WrapInFunction(i2_expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* outer = Sem().Get(outer_expr);
    ASSERT_NE(outer, nullptr);
    auto* str = outer->Type()->As<sem::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().size(), 2u);
    ASSERT_NE(outer->ConstantValue(), nullptr);
    EXPECT_TYPE(outer->ConstantValue()->Type(), outer->Type());
    EXPECT_FALSE(outer->ConstantValue()->AllEqual());
    EXPECT_TRUE(outer->ConstantValue()->AnyZero());
    EXPECT_FALSE(outer->ConstantValue()->AllZero());

    auto* o1 = Sem().Get(o1_expr);
    ASSERT_NE(o1->ConstantValue(), nullptr);
    EXPECT_FALSE(o1->ConstantValue()->AllEqual());
    EXPECT_FALSE(o1->ConstantValue()->AnyZero());
    EXPECT_FALSE(o1->ConstantValue()->AllZero());
    EXPECT_TRUE(o1->ConstantValue()->Type()->Is<sem::Struct>());
    EXPECT_EQ(o1->ConstantValue()->Index(0)->As<i32>(), 1_i);
    EXPECT_EQ(o1->ConstantValue()->Index(1)->As<u32>(), 2_u);
    EXPECT_EQ(o1->ConstantValue()->Index(2)->As<f32>(), 3_f);

    auto* i2 = Sem().Get(i2_expr);
    ASSERT_NE(i2->ConstantValue(), nullptr);
    EXPECT_TRUE(i2->ConstantValue()->AllEqual());
    EXPECT_FALSE(i2->ConstantValue()->AnyZero());
    EXPECT_FALSE(i2->ConstantValue()->AllZero());
    EXPECT_TRUE(i2->ConstantValue()->Type()->Is<sem::U32>());
    EXPECT_EQ(i2->ConstantValue()->As<u32>(), 2_u);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Unary op
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace unary_op {

template <typename T>
struct Values {
    T input;
    T expect;
};

struct Case {
    std::variant<Values<AInt>, Values<AFloat>, Values<u32>, Values<i32>, Values<f32>, Values<f16>>
        values;
};

static std::ostream& operator<<(std::ostream& o, const Case& c) {
    std::visit([&](auto&& v) { o << v.input; }, c.values);
    return o;
}

template <typename T>
Case C(T input, T expect) {
    return Case{Values<T>{input, expect}};
}

using ResolverConstEvalUnaryOpTest = ResolverTestWithParam<std::tuple<ast::UnaryOp, Case>>;

TEST_P(ResolverConstEvalUnaryOpTest, Test) {
    Enable(ast::Extension::kF16);

    auto op = std::get<0>(GetParam());
    auto c = std::get<1>(GetParam());
    std::visit(
        [&](auto&& values) {
            using T = decltype(values.expect);
            auto* expr = create<ast::UnaryOpExpression>(op, Expr(values.input));
            GlobalConst("C", nullptr, expr);

            EXPECT_TRUE(r()->Resolve()) << r()->error();

            auto* sem = Sem().Get(expr);
            const sem::Constant* value = sem->ConstantValue();
            ASSERT_NE(value, nullptr);
            EXPECT_TYPE(value->Type(), sem->Type());
            EXPECT_EQ(value->As<T>(), values.expect);

            if constexpr (IsInteger<UnwrapNumber<T>>) {
                // Check that the constant's integer doesn't contain unexpected data in the MSBs
                // that are outside of the bit-width of T.
                EXPECT_EQ(value->As<AInt>(), AInt(values.expect));
            }
        },
        c.values);
}
INSTANTIATE_TEST_SUITE_P(Complement,
                         ResolverConstEvalUnaryOpTest,
                         testing::Combine(testing::Values(ast::UnaryOp::kComplement),
                                          testing::ValuesIn({
                                              // AInt
                                              C(0_a, 0xffffffffffffffff_a),
                                              C(0xffffffffffffffff_a, 0_a),
                                              C(0xf0f0f0f0f0f0f0f0_a, 0x0f0f0f0f0f0f0f0f_a),
                                              C(0xaaaaaaaaaaaaaaaa_a, 0x5555555555555555_a),
                                              C(0x5555555555555555_a, 0xaaaaaaaaaaaaaaaa_a),
                                              // u32
                                              C(0_u, 0xffffffff_u),
                                              C(0xffffffff_u, 0_u),
                                              C(0xf0f0f0f0_u, 0x0f0f0f0f_u),
                                              C(0xaaaaaaaa_u, 0x55555555_u),
                                              C(0x55555555_u, 0xaaaaaaaa_u),
                                              // i32
                                              C(0_i, -1_i),
                                              C(-1_i, 0_i),
                                              C(1_i, -2_i),
                                              C(-2_i, 1_i),
                                              C(2_i, -3_i),
                                              C(-3_i, 2_i),
                                          })));

INSTANTIATE_TEST_SUITE_P(Negation,
                         ResolverConstEvalUnaryOpTest,
                         testing::Combine(testing::Values(ast::UnaryOp::kNegation),
                                          testing::ValuesIn({
                                              // AInt
                                              C(0_a, -0_a),
                                              C(-0_a, 0_a),
                                              C(1_a, -1_a),
                                              C(-1_a, 1_a),
                                              C(kHighest<AInt>, -kHighest<AInt>),
                                              C(-kHighest<AInt>, kHighest<AInt>),
                                              C(kLowest<AInt>, Negate(kLowest<AInt>)),
                                              C(Negate(kLowest<AInt>), kLowest<AInt>),
                                              // i32
                                              C(0_i, -0_i),
                                              C(-0_i, 0_i),
                                              C(1_i, -1_i),
                                              C(-1_i, 1_i),
                                              C(kHighest<i32>, -kHighest<i32>),
                                              C(-kHighest<i32>, kHighest<i32>),
                                              C(kLowest<i32>, Negate(kLowest<i32>)),
                                              C(Negate(kLowest<i32>), kLowest<i32>),
                                              // AFloat
                                              C(0.0_a, -0.0_a),
                                              C(-0.0_a, 0.0_a),
                                              C(1.0_a, -1.0_a),
                                              C(-1.0_a, 1.0_a),
                                              C(kHighest<AFloat>, -kHighest<AFloat>),
                                              C(-kHighest<AFloat>, kHighest<AFloat>),
                                              C(kLowest<AFloat>, Negate(kLowest<AFloat>)),
                                              C(Negate(kLowest<AFloat>), kLowest<AFloat>),
                                              // f32
                                              C(0.0_f, -0.0_f),
                                              C(-0.0_f, 0.0_f),
                                              C(1.0_f, -1.0_f),
                                              C(-1.0_f, 1.0_f),
                                              C(kHighest<f32>, -kHighest<f32>),
                                              C(-kHighest<f32>, kHighest<f32>),
                                              C(kLowest<f32>, Negate(kLowest<f32>)),
                                              C(Negate(kLowest<f32>), kLowest<f32>),
                                              // f16
                                              C(0.0_h, -0.0_h),
                                              C(-0.0_h, 0.0_h),
                                              C(1.0_h, -1.0_h),
                                              C(-1.0_h, 1.0_h),
                                              C(kHighest<f16>, -kHighest<f16>),
                                              C(-kHighest<f16>, kHighest<f16>),
                                              C(kLowest<f16>, Negate(kLowest<f16>)),
                                              C(Negate(kLowest<f16>), kLowest<f16>),
                                          })));

// Make sure UBSan doesn't trip on C++'s undefined behaviour of negating the smallest negative
// number.
TEST_F(ResolverConstEvalTest, UnaryNegateLowestAbstract) {
    // const break_me = -(-9223372036854775808);
    auto* c = GlobalConst("break_me", nullptr, Negation(Negation(Expr(9223372036854775808_a))));
    (void)c;
    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(c);
    EXPECT_EQ(sem->ConstantValue()->As<AInt>(), 9223372036854775808_a);
}

}  // namespace unary_op

namespace builtin {

template <typename T>
struct Values {
    std::vector<T> args;
    T result;
    bool result_pos_or_neg;
};

struct Case {
    std::variant<Values<AInt>, Values<AFloat>, Values<u32>, Values<i32>, Values<f32>, Values<f16>>
        values;
};

static std::ostream& operator<<(std::ostream& o, const Case& c) {
    std::visit(
        [&](auto&& v) {
            for (auto& e : v.args) {
                o << e << ((&e != &v.args.back()) ? " " : "");
            }
        },
        c.values);
    return o;
}

template <typename T>
Case C(std::vector<T> args, T result, bool result_pos_or_neg = false) {
    return Case{Values<T>{std::move(args), result, result_pos_or_neg}};
}

using ResolverConstEvalBuiltinTest = ResolverTestWithParam<std::tuple<sem::BuiltinType, Case>>;

TEST_P(ResolverConstEvalBuiltinTest, Test) {
    Enable(ast::Extension::kF16);

    auto builtin = std::get<0>(GetParam());
    auto c = std::get<1>(GetParam());
    std::visit(
        [&](auto&& values) {
            using T = decltype(values.result);
            auto args = utils::Transform(values.args, [&](auto&& a) {
                return static_cast<const ast::Expression*>(Expr(a));
            });
            auto* expr = Call(sem::str(builtin), std::move(args));

            GlobalConst("C", nullptr, expr);

            EXPECT_TRUE(r()->Resolve()) << r()->error();

            auto* sem = Sem().Get(expr);
            const sem::Constant* value = sem->ConstantValue();
            ASSERT_NE(value, nullptr);
            EXPECT_TYPE(value->Type(), sem->Type());

            auto actual = value->As<T>();

            if constexpr (IsFloatingPoint<UnwrapNumber<T>>) {
                if (std::isnan(values.result)) {
                    EXPECT_TRUE(std::isnan(actual));
                } else {
                    EXPECT_FLOAT_EQ(values.result_pos_or_neg ? Abs(actual) : actual, values.result);
                }
            } else {
                EXPECT_EQ(values.result_pos_or_neg ? Abs(actual) : actual, values.result);
            }

            if constexpr (IsInteger<UnwrapNumber<T>>) {
                // Check that the constant's integer doesn't contain unexpected data in the MSBs
                // that are outside of the bit-width of T.
                EXPECT_EQ(value->As<AInt>(), AInt(values.result));
            }
        },
        c.values);
}

template <typename T, bool finite_only>
std::vector<Case> Atan2Cases() {
    std::vector<Case> cases = {
        // If y is +/-0 and x is negative or -0, +/-PI is returned
        C({T(0.0), -T(0.0)}, kPi<T>, true),

        // If y is +/-0 and x is positive or +0, +/-0 is returned
        C({T(0.0), T(0.0)}, T(0.0), true),

        // If x is +/-0 and y is negative, -PI/2 is returned
        C({-T(1.0), T(0.0)}, -kPiOver2<T>),
        C({-T(1.0), -T(0.0)}, -kPiOver2<T>),

        // If x is +/-0 and y is positive, +PI/2 is returned
        C({T(1.0), T(0.0)}, kPiOver2<T>),
        C({T(1.0), -T(0.0)}, kPiOver2<T>),
    };

    if constexpr (!finite_only) {
        std::vector<Case> non_finite_cases = {
            // If y is +/-INF and x is finite, +/-PI/2 is returned
            C({kInf<T>, T(0.0)}, kPiOver2<T>, true),
            C({-kInf<T>, T(0.0)}, kPiOver2<T>, true),

            // If y is +/-INF and x is -INF, +/-3PI/4 is returned
            C({kInf<T>, -kInf<T>}, k3PiOver4<T>, true),
            C({-kInf<T>, -kInf<T>}, k3PiOver4<T>, true),

            // If y is +/-INF and x is +INF, +/-PI/4 is returned
            C({kInf<T>, kInf<T>}, kPiOver4<T>, true),
            C({-kInf<T>, kInf<T>}, kPiOver4<T>, true),

            // If x is -INF and y is finite and positive, +PI is returned
            C({T(0.0), -kInf<T>}, kPi<T>),

            // If x is -INF and y is finite and negative, -PI is returned
            C({-T(0.0), -kInf<T>}, -kPi<T>),

            // If x is +INF and y is finite and positive, +0 is returned
            C({T(0.0), kInf<T>}, T(0.0)),

            // If x is +INF and y is finite and negative, -0 is returned
            C({-T(0.0), kInf<T>}, -T(0.0)),

            // If either x is NaN or y is NaN, NaN is returned
            C({kNaN<T>, T(0.0)}, kNaN<T>),
            C({T(0.0), kNaN<T>}, kNaN<T>),
            C({kNaN<T>, kNaN<T>}, kNaN<T>),
        };

        cases = Concat(cases, non_finite_cases);
    }

    return cases;
}

INSTANTIATE_TEST_SUITE_P(  //
    Atan2,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAtan2),
                     testing::ValuesIn(Concat(Atan2Cases<AFloat, true>(),  //
                                              Atan2Cases<f32, false>(),
                                              Atan2Cases<f16, false>()))));

template <typename T>
std::vector<Case> ClampCases() {
    return {
        C({T(0), T(0), T(0)}, T(0)),
        C({T(0), T(42), kHighest<T>}, T(42)),
        C({kLowest<T>, T(0), T(42)}, T(0)),
        C({T(0), kLowest<T>, kHighest<T>}, T(0)),
        C({T(0), kHighest<T>, kLowest<T>}, kLowest<T>),
        C({kHighest<T>, kHighest<T>, kHighest<T>}, kHighest<T>),
        C({kLowest<T>, kLowest<T>, kLowest<T>}, kLowest<T>),
        C({kHighest<T>, kLowest<T>, kHighest<T>}, kHighest<T>),
        C({kLowest<T>, kLowest<T>, kHighest<T>}, kLowest<T>),
    };
}

INSTANTIATE_TEST_SUITE_P(  //
    Clamp,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kClamp),
                     testing::ValuesIn(Concat(ClampCases<AInt>(),  //
                                              ClampCases<i32>(),
                                              ClampCases<u32>(),
                                              ClampCases<AFloat>(),
                                              ClampCases<f32>(),
                                              ClampCases<f16>()))));

}  // namespace builtin

}  // namespace
}  // namespace tint::resolver
