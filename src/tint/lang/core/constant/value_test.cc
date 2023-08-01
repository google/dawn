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

using ConstantTest_Value = TestHelper;

TEST_F(ConstantTest_Value, Equal_Scalar_Scalar) {
    EXPECT_TRUE(constants.Get(10_i)->Equal(constants.Get(10_i)));
    EXPECT_FALSE(constants.Get(10_i)->Equal(constants.Get(20_i)));
    EXPECT_FALSE(constants.Get(20_i)->Equal(constants.Get(10_i)));

    EXPECT_TRUE(constants.Get(10_u)->Equal(constants.Get(10_u)));
    EXPECT_FALSE(constants.Get(10_u)->Equal(constants.Get(20_u)));
    EXPECT_FALSE(constants.Get(20_u)->Equal(constants.Get(10_u)));

    EXPECT_TRUE(constants.Get(10_f)->Equal(constants.Get(10_f)));
    EXPECT_FALSE(constants.Get(10_f)->Equal(constants.Get(20_f)));
    EXPECT_FALSE(constants.Get(20_f)->Equal(constants.Get(10_f)));
}

TEST_F(ConstantTest_Value, Equal_Splat_Splat) {
    auto* vec3f = create<type::Vector>(create<type::F32>(), 3u);

    auto* vec3f_1_1_1 = constants.Splat(vec3f, constants.Get(1_f), 3);
    auto* vec3f_2_2_2 = constants.Splat(vec3f, constants.Get(2_f), 3);

    EXPECT_TRUE(vec3f_1_1_1->Equal(vec3f_1_1_1));
    EXPECT_FALSE(vec3f_2_2_2->Equal(vec3f_1_1_1));
    EXPECT_FALSE(vec3f_1_1_1->Equal(vec3f_2_2_2));
}

TEST_F(ConstantTest_Value, Equal_Composite_Composite) {
    auto* vec3f = create<type::Vector>(create<type::F32>(), 3u);

    auto* vec3f_1_1_2 = constants.Composite(
        vec3f, Vector{constants.Get(1_f), constants.Get(1_f), constants.Get(2_f)});
    auto* vec3f_1_2_1 = constants.Composite(
        vec3f, Vector{constants.Get(1_f), constants.Get(2_f), constants.Get(1_f)});

    EXPECT_TRUE(vec3f_1_1_2->Equal(vec3f_1_1_2));
    EXPECT_FALSE(vec3f_1_2_1->Equal(vec3f_1_1_2));
    EXPECT_FALSE(vec3f_1_1_2->Equal(vec3f_1_2_1));
}

TEST_F(ConstantTest_Value, Equal_Splat_Composite) {
    auto* vec3f = create<type::Vector>(create<type::F32>(), 3u);

    auto* vec3f_1_1_1 = constants.Splat(vec3f, constants.Get(1_f), 3);
    auto* vec3f_1_2_1 = constants.Composite(
        vec3f, Vector{constants.Get(1_f), constants.Get(2_f), constants.Get(1_f)});

    EXPECT_TRUE(vec3f_1_1_1->Equal(vec3f_1_1_1));
    EXPECT_FALSE(vec3f_1_2_1->Equal(vec3f_1_1_1));
    EXPECT_FALSE(vec3f_1_1_1->Equal(vec3f_1_2_1));
}

}  // namespace
}  // namespace tint::constant
