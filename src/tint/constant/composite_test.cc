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

#include "src/tint/constant/composite.h"

#include "src/tint/constant/scalar.h"
#include "src/tint/constant/test_helper.h"

namespace tint::constant {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using ConstantTest_Composite = TestHelper;

TEST_F(ConstantTest_Composite, AllZero) {
    auto* f32 = create<type::F32>();

    auto* fPos0 = create<Scalar<tint::f32>>(f32, 0_f);
    auto* fNeg0 = create<Scalar<tint::f32>>(f32, -0_f);
    auto* fPos1 = create<Scalar<tint::f32>>(f32, 1_f);

    auto* compositeAll = create<Composite>(f32, utils::Vector{fPos0, fPos0});
    auto* compositeAny = create<Composite>(f32, utils::Vector{fNeg0, fPos1, fPos0});
    auto* compositeNone = create<Composite>(f32, utils::Vector{fNeg0, fNeg0});

    EXPECT_TRUE(compositeAll->AllZero());
    EXPECT_FALSE(compositeAny->AllZero());
    EXPECT_FALSE(compositeNone->AllZero());
}

TEST_F(ConstantTest_Composite, AnyZero) {
    auto* f32 = create<type::F32>();

    auto* fPos0 = create<Scalar<tint::f32>>(f32, 0_f);
    auto* fNeg0 = create<Scalar<tint::f32>>(f32, -0_f);
    auto* fPos1 = create<Scalar<tint::f32>>(f32, 1_f);

    auto* compositeAll = create<Composite>(f32, utils::Vector{fPos0, fPos0});
    auto* compositeAny = create<Composite>(f32, utils::Vector{fNeg0, fPos1, fPos0});
    auto* compositeNone = create<Composite>(f32, utils::Vector{fNeg0, fNeg0});

    EXPECT_TRUE(compositeAll->AnyZero());
    EXPECT_TRUE(compositeAny->AnyZero());
    EXPECT_FALSE(compositeNone->AnyZero());
}

TEST_F(ConstantTest_Composite, Index) {
    auto* f32 = create<type::F32>();

    auto* fPos0 = create<Scalar<tint::f32>>(f32, 0_f);
    auto* fPos1 = create<Scalar<tint::f32>>(f32, 1_f);

    auto* composite = create<Composite>(f32, utils::Vector{fPos1, fPos0});

    ASSERT_NE(composite->Index(0), nullptr);
    ASSERT_NE(composite->Index(1), nullptr);
    ASSERT_EQ(composite->Index(2), nullptr);

    EXPECT_TRUE(composite->Index(0)->Is<Scalar<tint::f32>>());
    EXPECT_EQ(composite->Index(0)->As<Scalar<tint::f32>>()->ValueOf(), 1.0);
    EXPECT_TRUE(composite->Index(1)->Is<Scalar<tint::f32>>());
    EXPECT_EQ(composite->Index(1)->As<Scalar<tint::f32>>()->ValueOf(), 0.0);
}

TEST_F(ConstantTest_Composite, Clone) {
    auto* f32 = create<type::F32>();

    auto* fPos0 = create<Scalar<tint::f32>>(f32, 0_f);
    auto* fPos1 = create<Scalar<tint::f32>>(f32, 1_f);

    auto* composite = create<Composite>(f32, utils::Vector{fPos1, fPos0});

    type::Manager mgr;
    utils::BlockAllocator<constant::Value> consts;
    constant::CloneContext ctx{type::CloneContext{{nullptr}, {nullptr, &mgr}}, {&consts}};

    auto* r = composite->As<Composite>()->Clone(ctx);
    ASSERT_NE(r, nullptr);
    EXPECT_TRUE(r->type->Is<type::F32>());
    EXPECT_FALSE(r->all_zero);
    EXPECT_TRUE(r->any_zero);
    ASSERT_EQ(r->elements.Length(), 2u);
}

}  // namespace
}  // namespace tint::constant
