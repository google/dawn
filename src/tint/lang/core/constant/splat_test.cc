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

#include "src/tint/lang/core/constant/splat.h"

#include "src/tint/lang/core/constant/helper_test.h"
#include "src/tint/lang/core/constant/scalar.h"

namespace tint::constant {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using ConstantTest_Splat = TestHelper;

TEST_F(ConstantTest_Splat, AllZero) {
    auto* vec3f = create<type::Vector>(create<type::F32>(), 3u);

    auto* fPos0 = constants.Get(0_f);
    auto* fNeg0 = constants.Get(-0_f);
    auto* fPos1 = constants.Get(1_f);

    auto* SpfPos0 = constants.Splat(vec3f, fPos0, 3);
    auto* SpfNeg0 = constants.Splat(vec3f, fNeg0, 3);
    auto* SpfPos1 = constants.Splat(vec3f, fPos1, 3);

    EXPECT_TRUE(SpfPos0->AllZero());
    EXPECT_FALSE(SpfNeg0->AllZero());
    EXPECT_FALSE(SpfPos1->AllZero());
}

TEST_F(ConstantTest_Splat, AnyZero) {
    auto* vec3f = create<type::Vector>(create<type::F32>(), 3u);

    auto* fPos0 = constants.Get(0_f);
    auto* fNeg0 = constants.Get(-0_f);
    auto* fPos1 = constants.Get(1_f);

    auto* SpfPos0 = constants.Splat(vec3f, fPos0, 3);
    auto* SpfNeg0 = constants.Splat(vec3f, fNeg0, 3);
    auto* SpfPos1 = constants.Splat(vec3f, fPos1, 3);

    EXPECT_TRUE(SpfPos0->AnyZero());
    EXPECT_FALSE(SpfNeg0->AnyZero());
    EXPECT_FALSE(SpfPos1->AnyZero());
}

TEST_F(ConstantTest_Splat, Index) {
    auto* vec3f = create<type::Vector>(create<type::F32>(), 3u);

    auto* f1 = constants.Get(1_f);
    auto* sp = constants.Splat(vec3f, f1, 2);

    ASSERT_NE(sp->Index(0), nullptr);
    ASSERT_NE(sp->Index(1), nullptr);
    ASSERT_EQ(sp->Index(2), nullptr);

    EXPECT_EQ(sp->Index(0)->As<Scalar<tint::f32>>()->ValueOf(), 1.f);
    EXPECT_EQ(sp->Index(1)->As<Scalar<tint::f32>>()->ValueOf(), 1.f);
}

TEST_F(ConstantTest_Splat, Clone) {
    auto* vec3i = create<type::Vector>(create<type::I32>(), 3u);
    auto* val = constants.Get(12_i);
    auto* sp = constants.Splat(vec3i, val, 2);

    constant::Manager mgr;
    constant::CloneContext ctx{type::CloneContext{{nullptr}, {nullptr, &mgr.types}}, mgr};

    auto* r = sp->Clone(ctx);
    ASSERT_NE(r, nullptr);
    EXPECT_TRUE(r->type->Is<type::Vector>());
    EXPECT_TRUE(r->el->Is<Scalar<tint::i32>>());
    EXPECT_EQ(r->count, 2u);
}

}  // namespace
}  // namespace tint::constant
