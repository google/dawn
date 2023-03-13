// Copyright 2023 The Tint Authors.
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

#include "src/tint/constant/scalar.h"

#include "src/tint/constant/test_helper.h"

namespace tint::constant {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using ConstantTest_Scalar = TestHelper;

TEST_F(ConstantTest_Scalar, AllZero) {
    auto* i32 = create<type::I32>();
    auto* u32 = create<type::U32>();
    auto* f16 = create<type::F16>();
    auto* f32 = create<type::F32>();
    auto* bool_ = create<type::Bool>();

    auto* i0 = create<Scalar<tint::i32>>(i32, 0_i);
    auto* iPos1 = create<Scalar<tint::i32>>(i32, 1_i);
    auto* iNeg1 = create<Scalar<tint::i32>>(i32, -1_i);

    auto* u0 = create<Scalar<tint::u32>>(u32, 0_u);
    auto* u1 = create<Scalar<tint::u32>>(u32, 1_u);

    auto* fPos0 = create<Scalar<tint::f32>>(f32, 0_f);
    auto* fNeg0 = create<Scalar<tint::f32>>(f32, -0_f);
    auto* fPos1 = create<Scalar<tint::f32>>(f32, 1_f);
    auto* fNeg1 = create<Scalar<tint::f32>>(f32, -1_f);

    auto* f16Pos0 = create<Scalar<tint::f16>>(f16, 0_h);
    auto* f16Neg0 = create<Scalar<tint::f16>>(f16, -0_h);
    auto* f16Pos1 = create<Scalar<tint::f16>>(f16, 1_h);
    auto* f16Neg1 = create<Scalar<tint::f16>>(f16, -1_h);

    auto* bf = create<Scalar<bool>>(bool_, false);
    auto* bt = create<Scalar<bool>>(bool_, true);

    auto* afPos0 = create<Scalar<tint::AFloat>>(f32, 0.0_a);
    auto* afNeg0 = create<Scalar<tint::AFloat>>(f32, -0.0_a);
    auto* afPos1 = create<Scalar<tint::AFloat>>(f32, 1.0_a);
    auto* afNeg1 = create<Scalar<tint::AFloat>>(f32, -1.0_a);

    auto* ai0 = create<Scalar<tint::AInt>>(i32, 0_a);
    auto* aiPos1 = create<Scalar<tint::AInt>>(i32, 1_a);
    auto* aiNeg1 = create<Scalar<tint::AInt>>(i32, -1_a);

    EXPECT_TRUE(i0->AllZero());
    EXPECT_FALSE(iPos1->AllZero());
    EXPECT_FALSE(iNeg1->AllZero());

    EXPECT_TRUE(u0->AllZero());
    EXPECT_FALSE(u1->AllZero());

    EXPECT_TRUE(fPos0->AllZero());
    EXPECT_FALSE(fNeg0->AllZero());
    EXPECT_FALSE(fPos1->AllZero());
    EXPECT_FALSE(fNeg1->AllZero());

    EXPECT_TRUE(f16Pos0->AllZero());
    EXPECT_FALSE(f16Neg0->AllZero());
    EXPECT_FALSE(f16Pos1->AllZero());
    EXPECT_FALSE(f16Neg1->AllZero());

    EXPECT_TRUE(bf->AllZero());
    EXPECT_FALSE(bt->AllZero());

    EXPECT_TRUE(afPos0->AllZero());
    EXPECT_FALSE(afNeg0->AllZero());
    EXPECT_FALSE(afPos1->AllZero());
    EXPECT_FALSE(afNeg1->AllZero());

    EXPECT_TRUE(ai0->AllZero());
    EXPECT_FALSE(aiPos1->AllZero());
    EXPECT_FALSE(aiNeg1->AllZero());
}

TEST_F(ConstantTest_Scalar, AnyZero) {
    auto* i32 = create<type::I32>();
    auto* u32 = create<type::U32>();
    auto* f16 = create<type::F16>();
    auto* f32 = create<type::F32>();
    auto* bool_ = create<type::Bool>();

    auto* i0 = create<Scalar<tint::i32>>(i32, 0_i);
    auto* iPos1 = create<Scalar<tint::i32>>(i32, 1_i);
    auto* iNeg1 = create<Scalar<tint::i32>>(i32, -1_i);

    auto* u0 = create<Scalar<tint::u32>>(u32, 0_u);
    auto* u1 = create<Scalar<tint::u32>>(u32, 1_u);

    auto* fPos0 = create<Scalar<tint::f32>>(f32, 0_f);
    auto* fNeg0 = create<Scalar<tint::f32>>(f32, -0_f);
    auto* fPos1 = create<Scalar<tint::f32>>(f32, 1_f);
    auto* fNeg1 = create<Scalar<tint::f32>>(f32, -1_f);

    auto* f16Pos0 = create<Scalar<tint::f16>>(f16, 0_h);
    auto* f16Neg0 = create<Scalar<tint::f16>>(f16, -0_h);
    auto* f16Pos1 = create<Scalar<tint::f16>>(f16, 1_h);
    auto* f16Neg1 = create<Scalar<tint::f16>>(f16, -1_h);

    auto* bf = create<Scalar<bool>>(bool_, false);
    auto* bt = create<Scalar<bool>>(bool_, true);

    auto* afPos0 = create<Scalar<tint::AFloat>>(f32, 0.0_a);
    auto* afNeg0 = create<Scalar<tint::AFloat>>(f32, -0.0_a);
    auto* afPos1 = create<Scalar<tint::AFloat>>(f32, 1.0_a);
    auto* afNeg1 = create<Scalar<tint::AFloat>>(f32, -1.0_a);

    auto* ai0 = create<Scalar<tint::AInt>>(i32, 0_a);
    auto* aiPos1 = create<Scalar<tint::AInt>>(i32, 1_a);
    auto* aiNeg1 = create<Scalar<tint::AInt>>(i32, -1_a);

    EXPECT_TRUE(i0->AnyZero());
    EXPECT_FALSE(iPos1->AnyZero());
    EXPECT_FALSE(iNeg1->AnyZero());

    EXPECT_TRUE(u0->AnyZero());
    EXPECT_FALSE(u1->AnyZero());

    EXPECT_TRUE(fPos0->AnyZero());
    EXPECT_FALSE(fNeg0->AnyZero());
    EXPECT_FALSE(fPos1->AnyZero());
    EXPECT_FALSE(fNeg1->AnyZero());

    EXPECT_TRUE(f16Pos0->AnyZero());
    EXPECT_FALSE(f16Neg0->AnyZero());
    EXPECT_FALSE(f16Pos1->AnyZero());
    EXPECT_FALSE(f16Neg1->AnyZero());

    EXPECT_TRUE(bf->AnyZero());
    EXPECT_FALSE(bt->AnyZero());

    EXPECT_TRUE(afPos0->AnyZero());
    EXPECT_FALSE(afNeg0->AnyZero());
    EXPECT_FALSE(afPos1->AnyZero());
    EXPECT_FALSE(afNeg1->AnyZero());

    EXPECT_TRUE(ai0->AnyZero());
    EXPECT_FALSE(aiPos1->AnyZero());
    EXPECT_FALSE(aiNeg1->AnyZero());
}

TEST_F(ConstantTest_Scalar, ValueOf) {
    auto* i32 = create<type::I32>();
    auto* u32 = create<type::U32>();
    auto* f16 = create<type::F16>();
    auto* f32 = create<type::F32>();
    auto* bool_ = create<type::Bool>();

    auto* i1 = create<Scalar<tint::i32>>(i32, 1_i);
    auto* u1 = create<Scalar<tint::u32>>(u32, 1_u);
    auto* f1 = create<Scalar<tint::f32>>(f32, 1_f);
    auto* f16Pos1 = create<Scalar<tint::f16>>(f16, 1_h);
    auto* bf = create<Scalar<bool>>(bool_, false);
    auto* bt = create<Scalar<bool>>(bool_, true);
    auto* af1 = create<Scalar<tint::AFloat>>(f32, 1.0_a);
    auto* ai1 = create<Scalar<tint::AInt>>(i32, 1_a);

    EXPECT_EQ(i1->ValueOf(), 1);
    EXPECT_EQ(u1->ValueOf(), 1u);
    EXPECT_EQ(f1->ValueOf(), 1.f);
    EXPECT_EQ(f16Pos1->ValueOf(), 1.f);
    EXPECT_FALSE(bf->ValueOf());
    EXPECT_TRUE(bt->ValueOf());
    EXPECT_EQ(af1->ValueOf(), 1.0);
    EXPECT_EQ(ai1->ValueOf(), 1l);
}

TEST_F(ConstantTest_Scalar, Clone) {
    auto* i32 = create<type::I32>();
    auto* val = create<Scalar<tint::i32>>(i32, 12_i);

    type::Manager mgr;
    utils::BlockAllocator<constant::Value> consts;
    constant::CloneContext ctx{type::CloneContext{{nullptr}, {nullptr, &mgr}}, {&consts}};

    auto* r = val->Clone(ctx);
    ASSERT_NE(r, nullptr);
    EXPECT_TRUE(r->type->Is<type::I32>());
    EXPECT_EQ(r->value, 12);
}

}  // namespace
}  // namespace tint::constant
