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

#include "src/tint/constant/splat.h"

#include "src/tint/constant/scalar.h"
#include "src/tint/constant/test_helper.h"

namespace tint::constant {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using ConstantTest_Splat = TestHelper;

TEST_F(ConstantTest_Splat, AllZero) {
    auto* f32 = create<type::F32>();

    auto* fPos0 = create<Scalar<tint::f32>>(f32, 0_f);
    auto* fNeg0 = create<Scalar<tint::f32>>(f32, -0_f);
    auto* fPos1 = create<Scalar<tint::f32>>(f32, 1_f);

    auto* SpfPos0 = create<Splat>(f32, fPos0, 2);
    auto* SpfNeg0 = create<Splat>(f32, fNeg0, 2);
    auto* SpfPos1 = create<Splat>(f32, fPos1, 2);

    EXPECT_TRUE(SpfPos0->AllZero());
    EXPECT_FALSE(SpfNeg0->AllZero());
    EXPECT_FALSE(SpfPos1->AllZero());
}

TEST_F(ConstantTest_Splat, AnyZero) {
    auto* f32 = create<type::F32>();

    auto* fPos0 = create<Scalar<tint::f32>>(f32, 0_f);
    auto* fNeg0 = create<Scalar<tint::f32>>(f32, -0_f);
    auto* fPos1 = create<Scalar<tint::f32>>(f32, 1_f);

    auto* SpfPos0 = create<Splat>(f32, fPos0, 2);
    auto* SpfNeg0 = create<Splat>(f32, fNeg0, 2);
    auto* SpfPos1 = create<Splat>(f32, fPos1, 2);

    EXPECT_TRUE(SpfPos0->AnyZero());
    EXPECT_FALSE(SpfNeg0->AnyZero());
    EXPECT_FALSE(SpfPos1->AnyZero());
}

TEST_F(ConstantTest_Splat, Index) {
    auto* f32 = create<type::F32>();

    auto* f1 = create<Scalar<tint::f32>>(f32, 1_f);
    auto* sp = create<Splat>(f32, f1, 2);

    ASSERT_NE(sp->Index(0), nullptr);
    ASSERT_NE(sp->Index(1), nullptr);
    ASSERT_EQ(sp->Index(2), nullptr);

    EXPECT_EQ(sp->Index(0)->As<Scalar<tint::f32>>()->ValueOf(), 1.f);
    EXPECT_EQ(sp->Index(1)->As<Scalar<tint::f32>>()->ValueOf(), 1.f);
}

TEST_F(ConstantTest_Splat, Clone) {
    auto* i32 = create<type::I32>();
    auto* val = create<Scalar<tint::i32>>(i32, 12_i);
    auto* sp = create<Splat>(i32, val, 2);

    type::Manager mgr;
    utils::BlockAllocator<constant::Value> consts;
    constant::CloneContext ctx{type::CloneContext{{nullptr}, {nullptr, &mgr}}, {&consts}};

    auto* r = sp->Clone(ctx);
    ASSERT_NE(r, nullptr);
    EXPECT_TRUE(r->type->Is<type::I32>());
    EXPECT_TRUE(r->el->Is<Scalar<tint::i32>>());
    EXPECT_EQ(r->count, 2u);
}

}  // namespace
}  // namespace tint::constant
