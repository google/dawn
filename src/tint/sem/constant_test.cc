// Copyright 2022 The Tint Authors.
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

#include "src/tint/sem/constant.h"

#include <gmock/gmock.h>

#include "src/tint/sem/abstract_float.h"
#include "src/tint/sem/abstract_int.h"
#include "src/tint/sem/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::sem {
namespace {

struct ConstantTest : public TestHelper {
    const sem::Array* Array(uint32_t n, const sem::Type* el_ty) {
        return create<sem::Array>(el_ty,
                                  /* count */ n,
                                  /* align */ 16u,
                                  /* size */ 4u * n,
                                  /* stride */ 16u * n,
                                  /* implicit_stride */ 16u * n);
    }
};

TEST_F(ConstantTest, ConstructorInitializerList) {
    {
        auto i = AInt(AInt::kHighest);
        Constant c(create<AbstractInt>(), {i});
        c.WithElements([&](auto&& vec) { EXPECT_THAT(vec, testing::ElementsAre(i)); });
    }
    {
        auto i = i32(i32::kHighest);
        Constant c(create<I32>(), {i});
        c.WithElements([&](auto&& vec) { EXPECT_THAT(vec, testing::ElementsAre(i)); });
    }
    {
        auto i = u32(u32::kHighest);
        Constant c(create<U32>(), {i});
        c.WithElements([&](auto&& vec) { EXPECT_THAT(vec, testing::ElementsAre(i)); });
    }
    {
        Constant c(create<Bool>(), {false});
        c.WithElements([&](auto&& vec) { EXPECT_THAT(vec, testing::ElementsAre(0_a)); });
    }
    {
        Constant c(create<Bool>(), {true});
        c.WithElements([&](auto&& vec) { EXPECT_THAT(vec, testing::ElementsAre(1_a)); });
    }
    {
        auto f = AFloat(AFloat::kHighest);
        Constant c(create<AbstractFloat>(), {f});
        c.WithElements([&](auto&& vec) { EXPECT_THAT(vec, testing::ElementsAre(f)); });
    }
    {
        auto f = f32(f32::kHighest);
        Constant c(create<F32>(), {f});
        c.WithElements([&](auto&& vec) { EXPECT_THAT(vec, testing::ElementsAre(f)); });
    }
    {
        auto f = f16(f16::kHighest);
        Constant c(create<F16>(), {f});
        c.WithElements([&](auto&& vec) { EXPECT_THAT(vec, testing::ElementsAre(f)); });
    }
}

TEST_F(ConstantTest, Element_ai) {
    auto* ty = create<AbstractInt>();
    Constant c(ty, {1_a});
    EXPECT_EQ(c.Element<AInt>(0), 1_a);
    EXPECT_EQ(c.ElementCount(), 1u);
    EXPECT_TYPE(c.Type(), ty);
    EXPECT_TYPE(c.ElementType(), ty);
}

TEST_F(ConstantTest, Element_i32) {
    auto* ty = create<I32>();
    Constant c(ty, {1_a});
    EXPECT_EQ(c.Element<i32>(0), 1_i);
    EXPECT_EQ(c.ElementCount(), 1u);
    EXPECT_TYPE(c.Type(), ty);
    EXPECT_TYPE(c.ElementType(), ty);
}

TEST_F(ConstantTest, Element_u32) {
    auto* ty = create<U32>();
    Constant c(ty, {1_a});
    EXPECT_EQ(c.Element<u32>(0), 1_u);
    EXPECT_EQ(c.ElementCount(), 1u);
    EXPECT_TYPE(c.Type(), ty);
    EXPECT_TYPE(c.ElementType(), ty);
}

TEST_F(ConstantTest, Element_bool) {
    auto* ty = create<Bool>();
    Constant c(ty, {true});
    EXPECT_EQ(c.Element<bool>(0), true);
    EXPECT_EQ(c.ElementCount(), 1u);
    EXPECT_TYPE(c.Type(), ty);
    EXPECT_TYPE(c.ElementType(), ty);
}

TEST_F(ConstantTest, Element_af) {
    auto* ty = create<AbstractFloat>();
    Constant c(ty, {1.0_a});
    EXPECT_EQ(c.Element<AFloat>(0), 1.0_a);
    EXPECT_EQ(c.ElementCount(), 1u);
    EXPECT_TYPE(c.Type(), ty);
    EXPECT_TYPE(c.ElementType(), ty);
}

TEST_F(ConstantTest, Element_f32) {
    auto* ty = create<F32>();
    Constant c(ty, {1.0_a});
    EXPECT_EQ(c.Element<f32>(0), 1.0_f);
    EXPECT_EQ(c.ElementCount(), 1u);
    EXPECT_TYPE(c.Type(), ty);
    EXPECT_TYPE(c.ElementType(), ty);
}

TEST_F(ConstantTest, Element_f16) {
    auto* ty = create<F16>();
    Constant c(ty, {1.0_a});
    EXPECT_EQ(c.Element<f16>(0), 1.0_h);
    EXPECT_EQ(c.ElementCount(), 1u);
    EXPECT_TYPE(c.Type(), ty);
    EXPECT_TYPE(c.ElementType(), ty);
}

TEST_F(ConstantTest, Element_vec3_ai) {
    auto* el_ty = create<AbstractInt>();
    auto* ty = create<Vector>(el_ty, 3u);
    Constant c(ty, {1_a, 2_a, 3_a});
    EXPECT_EQ(c.Element<AInt>(0), 1_a);
    EXPECT_EQ(c.Element<AInt>(1), 2_a);
    EXPECT_EQ(c.Element<AInt>(2), 3_a);
    EXPECT_EQ(c.ElementCount(), 3u);
    EXPECT_TYPE(c.Type(), ty);
    EXPECT_TYPE(c.ElementType(), el_ty);
}

TEST_F(ConstantTest, Element_vec3_i32) {
    auto* el_ty = create<I32>();
    auto* ty = create<Vector>(el_ty, 3u);
    Constant c(ty, {1_a, 2_a, 3_a});
    EXPECT_EQ(c.Element<i32>(0), 1_i);
    EXPECT_EQ(c.Element<i32>(1), 2_i);
    EXPECT_EQ(c.Element<i32>(2), 3_i);
    EXPECT_EQ(c.ElementCount(), 3u);
    EXPECT_TYPE(c.Type(), ty);
    EXPECT_TYPE(c.ElementType(), el_ty);
}

TEST_F(ConstantTest, Element_vec3_u32) {
    auto* el_ty = create<U32>();
    auto* ty = create<Vector>(el_ty, 3u);
    Constant c(ty, {1_a, 2_a, 3_a});
    EXPECT_EQ(c.Element<u32>(0), 1_u);
    EXPECT_EQ(c.Element<u32>(1), 2_u);
    EXPECT_EQ(c.Element<u32>(2), 3_u);
    EXPECT_EQ(c.ElementCount(), 3u);
    EXPECT_TYPE(c.Type(), ty);
    EXPECT_TYPE(c.ElementType(), el_ty);
}

TEST_F(ConstantTest, Element_vec3_bool) {
    auto* el_ty = create<Bool>();
    auto* ty = create<Vector>(el_ty, 2u);
    Constant c(ty, {true, false});
    EXPECT_EQ(c.Element<bool>(0), true);
    EXPECT_EQ(c.Element<bool>(1), false);
    EXPECT_EQ(c.ElementCount(), 2u);
    EXPECT_TYPE(c.Type(), ty);
    EXPECT_TYPE(c.ElementType(), el_ty);
}

TEST_F(ConstantTest, Element_vec3_af) {
    auto* el_ty = create<AbstractFloat>();
    auto* ty = create<Vector>(el_ty, 3u);
    Constant c(ty, {1.0_a, 2.0_a, 3.0_a});
    EXPECT_EQ(c.Element<AFloat>(0), 1.0_a);
    EXPECT_EQ(c.Element<AFloat>(1), 2.0_a);
    EXPECT_EQ(c.Element<AFloat>(2), 3.0_a);
    EXPECT_EQ(c.ElementCount(), 3u);
    EXPECT_TYPE(c.Type(), ty);
    EXPECT_TYPE(c.ElementType(), el_ty);
}

TEST_F(ConstantTest, Element_vec3_f32) {
    auto* el_ty = create<F32>();
    auto* ty = create<Vector>(el_ty, 3u);
    Constant c(ty, {1.0_a, 2.0_a, 3.0_a});
    EXPECT_EQ(c.Element<f32>(0), 1.0_f);
    EXPECT_EQ(c.Element<f32>(1), 2.0_f);
    EXPECT_EQ(c.Element<f32>(2), 3.0_f);
    EXPECT_EQ(c.ElementCount(), 3u);
    EXPECT_TYPE(c.Type(), ty);
    EXPECT_TYPE(c.ElementType(), el_ty);
}

TEST_F(ConstantTest, Element_vec3_f16) {
    auto* el_ty = create<F16>();
    auto* ty = create<Vector>(el_ty, 3u);
    Constant c(ty, {1.0_a, 2.0_a, 3.0_a});
    EXPECT_EQ(c.Element<f16>(0), 1.0_h);
    EXPECT_EQ(c.Element<f16>(1), 2.0_h);
    EXPECT_EQ(c.Element<f16>(2), 3.0_h);
    EXPECT_EQ(c.ElementCount(), 3u);
    EXPECT_TYPE(c.Type(), ty);
    EXPECT_TYPE(c.ElementType(), el_ty);
}

TEST_F(ConstantTest, Element_mat2x3_af) {
    auto* el_ty = create<AbstractFloat>();
    auto* ty = create<Matrix>(create<Vector>(el_ty, 3u), 2u);
    Constant c(ty, {1.0_a, 2.0_a, 3.0_a, 4.0_a, 5.0_a, 6.0_a});
    EXPECT_EQ(c.Element<AFloat>(0), 1.0_a);
    EXPECT_EQ(c.Element<AFloat>(1), 2.0_a);
    EXPECT_EQ(c.Element<AFloat>(2), 3.0_a);
    EXPECT_EQ(c.Element<AFloat>(3), 4.0_a);
    EXPECT_EQ(c.Element<AFloat>(4), 5.0_a);
    EXPECT_EQ(c.Element<AFloat>(5), 6.0_a);
    EXPECT_EQ(c.ElementCount(), 6u);
    EXPECT_TYPE(c.Type(), ty);
    EXPECT_TYPE(c.ElementType(), el_ty);
}

TEST_F(ConstantTest, Element_mat2x3_f32) {
    auto* el_ty = create<F32>();
    auto* ty = create<Matrix>(create<Vector>(el_ty, 3u), 2u);
    Constant c(ty, {1.0_a, 2.0_a, 3.0_a, 4.0_a, 5.0_a, 6.0_a});
    EXPECT_EQ(c.Element<f32>(0), 1.0_f);
    EXPECT_EQ(c.Element<f32>(1), 2.0_f);
    EXPECT_EQ(c.Element<f32>(2), 3.0_f);
    EXPECT_EQ(c.Element<f32>(3), 4.0_f);
    EXPECT_EQ(c.Element<f32>(4), 5.0_f);
    EXPECT_EQ(c.Element<f32>(5), 6.0_f);
    EXPECT_EQ(c.ElementCount(), 6u);
    EXPECT_TYPE(c.Type(), ty);
    EXPECT_TYPE(c.ElementType(), el_ty);
}

TEST_F(ConstantTest, Element_mat2x3_f16) {
    auto* el_ty = create<F16>();
    auto* ty = create<Matrix>(create<Vector>(el_ty, 3u), 2u);
    Constant c(ty, {1.0_a, 2.0_a, 3.0_a, 4.0_a, 5.0_a, 6.0_a});
    EXPECT_EQ(c.Element<f16>(0), 1.0_h);
    EXPECT_EQ(c.Element<f16>(1), 2.0_h);
    EXPECT_EQ(c.Element<f16>(2), 3.0_h);
    EXPECT_EQ(c.Element<f16>(3), 4.0_h);
    EXPECT_EQ(c.Element<f16>(4), 5.0_h);
    EXPECT_EQ(c.Element<f16>(5), 6.0_h);
    EXPECT_EQ(c.ElementCount(), 6u);
    EXPECT_TYPE(c.Type(), ty);
    EXPECT_TYPE(c.ElementType(), el_ty);
}

TEST_F(ConstantTest, Element_arr_vec3_ai) {
    auto* el_ty = create<AbstractInt>();
    auto* ty = Array(2, create<Vector>(el_ty, 3u));
    Constant c(ty, {1_a, 2_a, 3_a, 4_a, 5_a, 6_a});
    EXPECT_EQ(c.Element<AInt>(0), 1_a);
    EXPECT_EQ(c.Element<AInt>(1), 2_a);
    EXPECT_EQ(c.Element<AInt>(2), 3_a);
    EXPECT_EQ(c.Element<AInt>(3), 4_a);
    EXPECT_EQ(c.Element<AInt>(4), 5_a);
    EXPECT_EQ(c.Element<AInt>(5), 6_a);
    EXPECT_EQ(c.ElementCount(), 6u);
    EXPECT_TYPE(c.Type(), ty);
    EXPECT_TYPE(c.ElementType(), el_ty);
}

TEST_F(ConstantTest, Element_arr_vec3_i32) {
    auto* el_ty = create<I32>();
    auto* ty = Array(2, create<Vector>(el_ty, 3u));
    Constant c(ty, {1_a, 2_a, 3_a, 4_a, 5_a, 6_a});
    EXPECT_EQ(c.Element<i32>(0), 1_i);
    EXPECT_EQ(c.Element<i32>(1), 2_i);
    EXPECT_EQ(c.Element<i32>(2), 3_i);
    EXPECT_EQ(c.Element<i32>(3), 4_i);
    EXPECT_EQ(c.Element<i32>(4), 5_i);
    EXPECT_EQ(c.Element<i32>(5), 6_i);
    EXPECT_EQ(c.ElementCount(), 6u);
    EXPECT_TYPE(c.Type(), ty);
    EXPECT_TYPE(c.ElementType(), el_ty);
}

TEST_F(ConstantTest, Element_arr_vec3_u32) {
    auto* el_ty = create<U32>();
    auto* ty = Array(2, create<Vector>(el_ty, 3u));
    Constant c(ty, {1_a, 2_a, 3_a, 4_a, 5_a, 6_a});
    EXPECT_EQ(c.Element<u32>(0), 1_u);
    EXPECT_EQ(c.Element<u32>(1), 2_u);
    EXPECT_EQ(c.Element<u32>(2), 3_u);
    EXPECT_EQ(c.Element<u32>(3), 4_u);
    EXPECT_EQ(c.Element<u32>(4), 5_u);
    EXPECT_EQ(c.Element<u32>(5), 6_u);
    EXPECT_EQ(c.ElementCount(), 6u);
    EXPECT_TYPE(c.Type(), ty);
    EXPECT_TYPE(c.ElementType(), el_ty);
}

TEST_F(ConstantTest, Element_arr_vec3_bool) {
    auto* el_ty = create<Bool>();
    auto* ty = Array(2, create<Vector>(el_ty, 2u));
    Constant c(ty, {true, false, false, true});
    EXPECT_EQ(c.Element<bool>(0), true);
    EXPECT_EQ(c.Element<bool>(1), false);
    EXPECT_EQ(c.Element<bool>(2), false);
    EXPECT_EQ(c.Element<bool>(3), true);
    EXPECT_EQ(c.ElementCount(), 4u);
    EXPECT_TYPE(c.Type(), ty);
    EXPECT_TYPE(c.ElementType(), el_ty);
}

TEST_F(ConstantTest, Element_arr_vec3_af) {
    auto* el_ty = create<AbstractFloat>();
    auto* ty = Array(2, create<Vector>(el_ty, 3u));
    Constant c(ty, {1.0_a, 2.0_a, 3.0_a, 4.0_a, 5.0_a, 6.0_a});
    EXPECT_EQ(c.Element<AFloat>(0), 1.0_a);
    EXPECT_EQ(c.Element<AFloat>(1), 2.0_a);
    EXPECT_EQ(c.Element<AFloat>(2), 3.0_a);
    EXPECT_EQ(c.Element<AFloat>(3), 4.0_a);
    EXPECT_EQ(c.Element<AFloat>(4), 5.0_a);
    EXPECT_EQ(c.Element<AFloat>(5), 6.0_a);
    EXPECT_EQ(c.ElementCount(), 6u);
    EXPECT_TYPE(c.Type(), ty);
    EXPECT_TYPE(c.ElementType(), el_ty);
}

TEST_F(ConstantTest, Element_arr_vec3_f32) {
    auto* el_ty = create<F32>();
    auto* ty = Array(2, create<Vector>(el_ty, 3u));
    Constant c(ty, {1.0_a, 2.0_a, 3.0_a, 4.0_a, 5.0_a, 6.0_a});
    EXPECT_EQ(c.Element<f32>(0), 1.0_f);
    EXPECT_EQ(c.Element<f32>(1), 2.0_f);
    EXPECT_EQ(c.Element<f32>(2), 3.0_f);
    EXPECT_EQ(c.Element<f32>(3), 4.0_f);
    EXPECT_EQ(c.Element<f32>(4), 5.0_f);
    EXPECT_EQ(c.Element<f32>(5), 6.0_f);
    EXPECT_EQ(c.ElementCount(), 6u);
    EXPECT_TYPE(c.Type(), ty);
    EXPECT_TYPE(c.ElementType(), el_ty);
}

TEST_F(ConstantTest, Element_arr_vec3_f16) {
    auto* el_ty = create<F16>();
    auto* ty = Array(2, create<Vector>(el_ty, 3u));
    Constant c(ty, {1.0_a, 2.0_a, 3.0_a, 4.0_a, 5.0_a, 6.0_a});
    EXPECT_EQ(c.Element<f16>(0), 1.0_h);
    EXPECT_EQ(c.Element<f16>(1), 2.0_h);
    EXPECT_EQ(c.Element<f16>(2), 3.0_h);
    EXPECT_EQ(c.Element<f16>(3), 4.0_h);
    EXPECT_EQ(c.Element<f16>(4), 5.0_h);
    EXPECT_EQ(c.Element<f16>(5), 6.0_h);
    EXPECT_EQ(c.ElementCount(), 6u);
    EXPECT_TYPE(c.Type(), ty);
    EXPECT_TYPE(c.ElementType(), el_ty);
}

TEST_F(ConstantTest, Element_arr_arr_mat2x3_f32) {
    auto* el_ty = create<F32>();
    auto* ty = Array(2, Array(2, create<Matrix>(create<Vector>(el_ty, 3u), 2u)));
    Constant c(ty, {
                       1.0_a,  2.0_a,  3.0_a,  //
                       4.0_a,  5.0_a,  6.0_a,  //

                       7.0_a,  8.0_a,  9.0_a,   //
                       10.0_a, 11.0_a, 12.0_a,  //

                       13.0_a, 14.0_a, 15.0_a,  //
                       16.0_a, 17.0_a, 18.0_a,  //

                       19.0_a, 20.0_a, 21.0_a,  //
                       22.0_a, 23.0_a, 24.0_a,  //
                   });
    for (size_t i = 0; i < 24; i++) {
        EXPECT_EQ(c.Element<f32>(i), f32(i + 1));
    }
    EXPECT_EQ(c.ElementCount(), 24u);
    EXPECT_TYPE(c.Type(), ty);
    EXPECT_TYPE(c.ElementType(), el_ty);
}

TEST_F(ConstantTest, AnyZero) {
    auto* vec3_ai = create<Vector>(create<AbstractInt>(), 3u);
    EXPECT_EQ(Constant(vec3_ai, {1_a, 2_a, 3_a}).AnyZero(), false);
    EXPECT_EQ(Constant(vec3_ai, {0_a, 2_a, 3_a}).AnyZero(), true);
    EXPECT_EQ(Constant(vec3_ai, {1_a, 0_a, 3_a}).AnyZero(), true);
    EXPECT_EQ(Constant(vec3_ai, {1_a, 2_a, 0_a}).AnyZero(), true);
    EXPECT_EQ(Constant(vec3_ai, {0_a, 0_a, 0_a}).AnyZero(), true);

    auto* vec3_af = create<Vector>(create<AbstractFloat>(), 3u);
    EXPECT_EQ(Constant(vec3_af, {1._a, 2._a, 3._a}).AnyZero(), false);
    EXPECT_EQ(Constant(vec3_af, {0._a, 2._a, 3._a}).AnyZero(), true);
    EXPECT_EQ(Constant(vec3_af, {1._a, 0._a, 3._a}).AnyZero(), true);
    EXPECT_EQ(Constant(vec3_af, {1._a, 2._a, 0._a}).AnyZero(), true);
    EXPECT_EQ(Constant(vec3_af, {0._a, 0._a, 0._a}).AnyZero(), true);

    EXPECT_EQ(Constant(vec3_af, {1._a, -2._a, 3._a}).AnyZero(), false);
    EXPECT_EQ(Constant(vec3_af, {0._a, -2._a, 3._a}).AnyZero(), true);
    EXPECT_EQ(Constant(vec3_af, {1._a, -0._a, 3._a}).AnyZero(), false);
    EXPECT_EQ(Constant(vec3_af, {1._a, -2._a, 0._a}).AnyZero(), true);
    EXPECT_EQ(Constant(vec3_af, {0._a, -0._a, 0._a}).AnyZero(), true);
    EXPECT_EQ(Constant(vec3_af, {-0._a, -0._a, -0._a}).AnyZero(), false);
}

TEST_F(ConstantTest, AllZero) {
    auto* vec3_ai = create<Vector>(create<AbstractInt>(), 3u);
    EXPECT_EQ(Constant(vec3_ai, {1_a, 2_a, 3_a}).AllZero(), false);
    EXPECT_EQ(Constant(vec3_ai, {0_a, 2_a, 3_a}).AllZero(), false);
    EXPECT_EQ(Constant(vec3_ai, {1_a, 0_a, 3_a}).AllZero(), false);
    EXPECT_EQ(Constant(vec3_ai, {1_a, 2_a, 0_a}).AllZero(), false);
    EXPECT_EQ(Constant(vec3_ai, {0_a, 0_a, 0_a}).AllZero(), true);

    auto* vec3_af = create<Vector>(create<AbstractFloat>(), 3u);
    EXPECT_EQ(Constant(vec3_af, {1._a, 2._a, 3._a}).AllZero(), false);
    EXPECT_EQ(Constant(vec3_af, {0._a, 2._a, 3._a}).AllZero(), false);
    EXPECT_EQ(Constant(vec3_af, {1._a, 0._a, 3._a}).AllZero(), false);
    EXPECT_EQ(Constant(vec3_af, {1._a, 2._a, 0._a}).AllZero(), false);
    EXPECT_EQ(Constant(vec3_af, {0._a, 0._a, 0._a}).AllZero(), true);

    EXPECT_EQ(Constant(vec3_af, {1._a, -2._a, 3._a}).AllZero(), false);
    EXPECT_EQ(Constant(vec3_af, {0._a, -2._a, 3._a}).AllZero(), false);
    EXPECT_EQ(Constant(vec3_af, {1._a, -0._a, 3._a}).AllZero(), false);
    EXPECT_EQ(Constant(vec3_af, {1._a, -2._a, 0._a}).AllZero(), false);
    EXPECT_EQ(Constant(vec3_af, {0._a, -0._a, 0._a}).AllZero(), false);
    EXPECT_EQ(Constant(vec3_af, {-0._a, -0._a, -0._a}).AllZero(), false);
}

TEST_F(ConstantTest, AllEqual) {
    auto* vec3_ai = create<Vector>(create<AbstractInt>(), 3u);
    EXPECT_EQ(Constant(vec3_ai, {1_a, 2_a, 3_a}).AllEqual(), false);
    EXPECT_EQ(Constant(vec3_ai, {1_a, 1_a, 3_a}).AllEqual(), false);
    EXPECT_EQ(Constant(vec3_ai, {1_a, 3_a, 3_a}).AllEqual(), false);
    EXPECT_EQ(Constant(vec3_ai, {1_a, 1_a, 1_a}).AllEqual(), true);
    EXPECT_EQ(Constant(vec3_ai, {2_a, 2_a, 2_a}).AllEqual(), true);
    EXPECT_EQ(Constant(vec3_ai, {3_a, 3_a, 3_a}).AllEqual(), true);
    EXPECT_EQ(Constant(vec3_ai, {0_a, 0_a, 0_a}).AllEqual(), true);

    auto* vec3_af = create<Vector>(create<AbstractFloat>(), 3u);
    EXPECT_EQ(Constant(vec3_af, {1._a, 2._a, 3._a}).AllEqual(), false);
    EXPECT_EQ(Constant(vec3_af, {1._a, 1._a, 3._a}).AllEqual(), false);
    EXPECT_EQ(Constant(vec3_af, {1._a, 3._a, 3._a}).AllEqual(), false);
    EXPECT_EQ(Constant(vec3_af, {1._a, 1._a, 1._a}).AllEqual(), true);
    EXPECT_EQ(Constant(vec3_af, {2._a, 2._a, 2._a}).AllEqual(), true);
    EXPECT_EQ(Constant(vec3_af, {3._a, 3._a, 3._a}).AllEqual(), true);
    EXPECT_EQ(Constant(vec3_af, {0._a, 0._a, 0._a}).AllEqual(), true);
    EXPECT_EQ(Constant(vec3_af, {0._a, -0._a, 0._a}).AllEqual(), false);
}

TEST_F(ConstantTest, AllZeroRange) {
    auto* vec3_ai = create<Vector>(create<AbstractInt>(), 3u);
    EXPECT_EQ(Constant(vec3_ai, {1_a, 2_a, 3_a}).AllZero(1, 3), false);
    EXPECT_EQ(Constant(vec3_ai, {0_a, 2_a, 3_a}).AllZero(1, 3), false);
    EXPECT_EQ(Constant(vec3_ai, {1_a, 2_a, 3_a}).AllZero(1, 3), false);
    EXPECT_EQ(Constant(vec3_ai, {1_a, 2_a, 0_a}).AllZero(1, 3), false);
    EXPECT_EQ(Constant(vec3_ai, {0_a, 0_a, 3_a}).AllZero(1, 3), false);
    EXPECT_EQ(Constant(vec3_ai, {0_a, 2_a, 0_a}).AllZero(1, 3), false);
    EXPECT_EQ(Constant(vec3_ai, {1_a, 0_a, 0_a}).AllZero(1, 3), true);

    EXPECT_EQ(Constant(vec3_ai, {1_a, 2_a, 3_a}).AllZero(0, 2), false);
    EXPECT_EQ(Constant(vec3_ai, {0_a, 2_a, 3_a}).AllZero(0, 2), false);
    EXPECT_EQ(Constant(vec3_ai, {1_a, 2_a, 3_a}).AllZero(0, 2), false);
    EXPECT_EQ(Constant(vec3_ai, {1_a, 2_a, 0_a}).AllZero(0, 2), false);
    EXPECT_EQ(Constant(vec3_ai, {0_a, 0_a, 3_a}).AllZero(0, 2), true);
    EXPECT_EQ(Constant(vec3_ai, {0_a, 2_a, 0_a}).AllZero(0, 2), false);
    EXPECT_EQ(Constant(vec3_ai, {1_a, 0_a, 0_a}).AllZero(0, 2), false);

    auto* vec3_af = create<Vector>(create<AbstractFloat>(), 3u);
    EXPECT_EQ(Constant(vec3_af, {1._a, 2._a, 3._a}).AllZero(1, 3), false);
    EXPECT_EQ(Constant(vec3_af, {0._a, 2._a, 3._a}).AllZero(1, 3), false);
    EXPECT_EQ(Constant(vec3_af, {1._a, 2._a, 3._a}).AllZero(1, 3), false);
    EXPECT_EQ(Constant(vec3_af, {1._a, 2._a, 0._a}).AllZero(1, 3), false);
    EXPECT_EQ(Constant(vec3_af, {0._a, 0._a, 3._a}).AllZero(1, 3), false);
    EXPECT_EQ(Constant(vec3_af, {0._a, 2._a, 0._a}).AllZero(1, 3), false);
    EXPECT_EQ(Constant(vec3_af, {1._a, 0._a, 0._a}).AllZero(1, 3), true);
    EXPECT_EQ(Constant(vec3_af, {1._a, -0._a, 0._a}).AllZero(1, 3), false);
    EXPECT_EQ(Constant(vec3_af, {1._a, 0._a, -0._a}).AllZero(1, 3), false);
    EXPECT_EQ(Constant(vec3_af, {1._a, -0._a, -0._a}).AllZero(1, 3), false);

    EXPECT_EQ(Constant(vec3_af, {1._a, 2._a, 3._a}).AllZero(0, 2), false);
    EXPECT_EQ(Constant(vec3_af, {0._a, 2._a, 3._a}).AllZero(0, 2), false);
    EXPECT_EQ(Constant(vec3_af, {1._a, 2._a, 3._a}).AllZero(0, 2), false);
    EXPECT_EQ(Constant(vec3_af, {1._a, 2._a, 0._a}).AllZero(0, 2), false);
    EXPECT_EQ(Constant(vec3_af, {0._a, 0._a, 3._a}).AllZero(0, 2), true);
    EXPECT_EQ(Constant(vec3_af, {-0._a, 0._a, 1._a}).AllZero(0, 2), false);
    EXPECT_EQ(Constant(vec3_af, {0._a, -0._a, 1._a}).AllZero(0, 2), false);
    EXPECT_EQ(Constant(vec3_af, {-0._a, -0._a, 1._a}).AllZero(0, 2), false);
    EXPECT_EQ(Constant(vec3_af, {0._a, 2._a, 0._a}).AllZero(0, 2), false);
    EXPECT_EQ(Constant(vec3_af, {1._a, 0._a, 0._a}).AllZero(0, 2), false);
}

TEST_F(ConstantTest, AllEqualRange) {
    auto* vec3_ai = create<Vector>(create<AbstractInt>(), 3u);
    EXPECT_EQ(Constant(vec3_ai, {1_a, 2_a, 3_a}).AllEqual(1, 3), false);
    EXPECT_EQ(Constant(vec3_ai, {1_a, 1_a, 3_a}).AllEqual(1, 3), false);
    EXPECT_EQ(Constant(vec3_ai, {1_a, 3_a, 3_a}).AllEqual(1, 3), true);
    EXPECT_EQ(Constant(vec3_ai, {1_a, 1_a, 1_a}).AllEqual(1, 3), true);
    EXPECT_EQ(Constant(vec3_ai, {2_a, 2_a, 2_a}).AllEqual(1, 3), true);
    EXPECT_EQ(Constant(vec3_ai, {2_a, 2_a, 3_a}).AllEqual(1, 3), false);
    EXPECT_EQ(Constant(vec3_ai, {1_a, 0_a, 0_a}).AllEqual(1, 3), true);
    EXPECT_EQ(Constant(vec3_ai, {0_a, 1_a, 0_a}).AllEqual(1, 3), false);
    EXPECT_EQ(Constant(vec3_ai, {0_a, 0_a, 1_a}).AllEqual(1, 3), false);
    EXPECT_EQ(Constant(vec3_ai, {0_a, 0_a, 0_a}).AllEqual(1, 3), true);

    auto* vec3_af = create<Vector>(create<AbstractFloat>(), 3u);
    EXPECT_EQ(Constant(vec3_af, {1._a, 2._a, 3._a}).AllEqual(1, 3), false);
    EXPECT_EQ(Constant(vec3_af, {1._a, 1._a, 3._a}).AllEqual(1, 3), false);
    EXPECT_EQ(Constant(vec3_af, {1._a, 3._a, 3._a}).AllEqual(1, 3), true);
    EXPECT_EQ(Constant(vec3_af, {1._a, 1._a, 1._a}).AllEqual(1, 3), true);
    EXPECT_EQ(Constant(vec3_af, {2._a, 2._a, 2._a}).AllEqual(1, 3), true);
    EXPECT_EQ(Constant(vec3_af, {2._a, 2._a, 3._a}).AllEqual(1, 3), false);
    EXPECT_EQ(Constant(vec3_af, {1._a, 0._a, 0._a}).AllEqual(1, 3), true);
    EXPECT_EQ(Constant(vec3_af, {0._a, 1._a, 0._a}).AllEqual(1, 3), false);
    EXPECT_EQ(Constant(vec3_af, {0._a, 0._a, 1._a}).AllEqual(1, 3), false);
    EXPECT_EQ(Constant(vec3_af, {0._a, 0._a, 0._a}).AllEqual(1, 3), true);
    EXPECT_EQ(Constant(vec3_af, {1._a, -0._a, 0._a}).AllEqual(1, 3), false);
    EXPECT_EQ(Constant(vec3_af, {0._a, -1._a, 0._a}).AllEqual(1, 3), false);
    EXPECT_EQ(Constant(vec3_af, {0._a, -0._a, 1._a}).AllEqual(1, 3), false);
    EXPECT_EQ(Constant(vec3_af, {0._a, -0._a, 0._a}).AllEqual(1, 3), false);
    EXPECT_EQ(Constant(vec3_af, {0._a, -0._a, -0._a}).AllEqual(1, 3), true);
    EXPECT_EQ(Constant(vec3_af, {-0._a, -0._a, -0._a}).AllEqual(1, 3), true);
}

}  // namespace
}  // namespace tint::sem
