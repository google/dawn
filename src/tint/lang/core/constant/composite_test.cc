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

#include "src/tint/lang/core/constant/composite.h"

#include "src/tint/lang/core/constant/helper_test.h"
#include "src/tint/lang/core/constant/scalar.h"

namespace tint::constant {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using ConstantTest_Composite = TestHelper;

TEST_F(ConstantTest_Composite, AllZero) {
    auto* vec3f = create<type::Vector>(create<type::F32>(), 3u);

    auto* fPos0 = constants.Get(0_f);
    auto* fNeg0 = constants.Get(-0_f);
    auto* fPos1 = constants.Get(1_f);

    auto* compositeAll = constants.Composite(vec3f, Vector{fPos0, fPos0});
    auto* compositeAny = constants.Composite(vec3f, Vector{fNeg0, fPos1, fPos0});
    auto* compositeNone = constants.Composite(vec3f, Vector{fNeg0, fNeg0});

    EXPECT_TRUE(compositeAll->AllZero());
    EXPECT_FALSE(compositeAny->AllZero());
    EXPECT_FALSE(compositeNone->AllZero());
}

TEST_F(ConstantTest_Composite, AnyZero) {
    auto* vec3f = create<type::Vector>(create<type::F32>(), 3u);

    auto* fPos0 = constants.Get(0_f);
    auto* fNeg0 = constants.Get(-0_f);
    auto* fPos1 = constants.Get(1_f);

    auto* compositeAll = constants.Composite(vec3f, Vector{fPos0, fPos0});
    auto* compositeAny = constants.Composite(vec3f, Vector{fNeg0, fPos1, fPos0});
    auto* compositeNone = constants.Composite(vec3f, Vector{fNeg0, fNeg0});

    EXPECT_TRUE(compositeAll->AnyZero());
    EXPECT_TRUE(compositeAny->AnyZero());
    EXPECT_FALSE(compositeNone->AnyZero());
}

TEST_F(ConstantTest_Composite, Index) {
    auto* vec3f = create<type::Vector>(create<type::F32>(), 3u);

    auto* fPos0 = constants.Get(0_f);
    auto* fPos1 = constants.Get(1_f);

    auto* composite = constants.Composite(vec3f, Vector{fPos1, fPos0});

    ASSERT_NE(composite->Index(0), nullptr);
    ASSERT_NE(composite->Index(1), nullptr);
    ASSERT_EQ(composite->Index(2), nullptr);

    EXPECT_TRUE(composite->Index(0)->Is<Scalar<tint::f32>>());
    EXPECT_EQ(composite->Index(0)->As<Scalar<tint::f32>>()->ValueOf(), 1.0);
    EXPECT_TRUE(composite->Index(1)->Is<Scalar<tint::f32>>());
    EXPECT_EQ(composite->Index(1)->As<Scalar<tint::f32>>()->ValueOf(), 0.0);
}

TEST_F(ConstantTest_Composite, Clone) {
    auto* vec3f = create<type::Vector>(create<type::F32>(), 3u);

    auto* fPos0 = constants.Get(0_f);
    auto* fPos1 = constants.Get(1_f);

    auto* composite = constants.Composite(vec3f, Vector{fPos1, fPos0});

    constant::Manager mgr;
    constant::CloneContext ctx{type::CloneContext{{nullptr}, {nullptr, &mgr.types}}, mgr};

    auto* r = composite->As<Composite>()->Clone(ctx);
    ASSERT_NE(r, nullptr);
    EXPECT_TRUE(r->type->Is<type::Vector>());
    EXPECT_FALSE(r->all_zero);
    EXPECT_TRUE(r->any_zero);
    ASSERT_EQ(r->elements.Length(), 2u);
}

}  // namespace
}  // namespace tint::constant
