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

#include "src/tint/resolver/validator.h"

#include "gmock/gmock.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/type/atomic.h"

namespace tint::resolver {
namespace {

using ValidatorIsStorableTest = ResolverTest;

TEST_F(ValidatorIsStorableTest, Void) {
    EXPECT_FALSE(v()->IsStorable(create<type::Void>()));
}

TEST_F(ValidatorIsStorableTest, Scalar) {
    EXPECT_TRUE(v()->IsStorable(create<type::Bool>()));
    EXPECT_TRUE(v()->IsStorable(create<type::I32>()));
    EXPECT_TRUE(v()->IsStorable(create<type::U32>()));
    EXPECT_TRUE(v()->IsStorable(create<type::F32>()));
    EXPECT_TRUE(v()->IsStorable(create<type::F16>()));
}

TEST_F(ValidatorIsStorableTest, Vector) {
    EXPECT_TRUE(v()->IsStorable(create<type::Vector>(create<type::I32>(), 2u)));
    EXPECT_TRUE(v()->IsStorable(create<type::Vector>(create<type::I32>(), 3u)));
    EXPECT_TRUE(v()->IsStorable(create<type::Vector>(create<type::I32>(), 4u)));
    EXPECT_TRUE(v()->IsStorable(create<type::Vector>(create<type::U32>(), 2u)));
    EXPECT_TRUE(v()->IsStorable(create<type::Vector>(create<type::U32>(), 3u)));
    EXPECT_TRUE(v()->IsStorable(create<type::Vector>(create<type::U32>(), 4u)));
    EXPECT_TRUE(v()->IsStorable(create<type::Vector>(create<type::F32>(), 2u)));
    EXPECT_TRUE(v()->IsStorable(create<type::Vector>(create<type::F32>(), 3u)));
    EXPECT_TRUE(v()->IsStorable(create<type::Vector>(create<type::F32>(), 4u)));
    EXPECT_TRUE(v()->IsStorable(create<type::Vector>(create<type::F16>(), 2u)));
    EXPECT_TRUE(v()->IsStorable(create<type::Vector>(create<type::F16>(), 3u)));
    EXPECT_TRUE(v()->IsStorable(create<type::Vector>(create<type::F16>(), 4u)));
}

TEST_F(ValidatorIsStorableTest, Matrix) {
    auto* vec2_f32 = create<type::Vector>(create<type::F32>(), 2u);
    auto* vec3_f32 = create<type::Vector>(create<type::F32>(), 3u);
    auto* vec4_f32 = create<type::Vector>(create<type::F32>(), 4u);
    auto* vec2_f16 = create<type::Vector>(create<type::F16>(), 2u);
    auto* vec3_f16 = create<type::Vector>(create<type::F16>(), 3u);
    auto* vec4_f16 = create<type::Vector>(create<type::F16>(), 4u);
    EXPECT_TRUE(v()->IsStorable(create<type::Matrix>(vec2_f32, 2u)));
    EXPECT_TRUE(v()->IsStorable(create<type::Matrix>(vec2_f32, 3u)));
    EXPECT_TRUE(v()->IsStorable(create<type::Matrix>(vec2_f32, 4u)));
    EXPECT_TRUE(v()->IsStorable(create<type::Matrix>(vec3_f32, 2u)));
    EXPECT_TRUE(v()->IsStorable(create<type::Matrix>(vec3_f32, 3u)));
    EXPECT_TRUE(v()->IsStorable(create<type::Matrix>(vec3_f32, 4u)));
    EXPECT_TRUE(v()->IsStorable(create<type::Matrix>(vec4_f32, 2u)));
    EXPECT_TRUE(v()->IsStorable(create<type::Matrix>(vec4_f32, 3u)));
    EXPECT_TRUE(v()->IsStorable(create<type::Matrix>(vec4_f32, 4u)));
    EXPECT_TRUE(v()->IsStorable(create<type::Matrix>(vec2_f16, 2u)));
    EXPECT_TRUE(v()->IsStorable(create<type::Matrix>(vec2_f16, 3u)));
    EXPECT_TRUE(v()->IsStorable(create<type::Matrix>(vec2_f16, 4u)));
    EXPECT_TRUE(v()->IsStorable(create<type::Matrix>(vec3_f16, 2u)));
    EXPECT_TRUE(v()->IsStorable(create<type::Matrix>(vec3_f16, 3u)));
    EXPECT_TRUE(v()->IsStorable(create<type::Matrix>(vec3_f16, 4u)));
    EXPECT_TRUE(v()->IsStorable(create<type::Matrix>(vec4_f16, 2u)));
    EXPECT_TRUE(v()->IsStorable(create<type::Matrix>(vec4_f16, 3u)));
    EXPECT_TRUE(v()->IsStorable(create<type::Matrix>(vec4_f16, 4u)));
}

TEST_F(ValidatorIsStorableTest, Pointer) {
    auto* ptr = create<type::Pointer>(create<type::I32>(), builtin::AddressSpace::kPrivate,
                                      builtin::Access::kReadWrite);
    EXPECT_FALSE(v()->IsStorable(ptr));
}

TEST_F(ValidatorIsStorableTest, Atomic) {
    EXPECT_TRUE(v()->IsStorable(create<type::Atomic>(create<type::I32>())));
    EXPECT_TRUE(v()->IsStorable(create<type::Atomic>(create<type::U32>())));
}

TEST_F(ValidatorIsStorableTest, ArraySizedOfStorable) {
    auto* arr = create<type::Array>(create<type::I32>(), create<type::ConstantArrayCount>(5u), 4u,
                                    20u, 4u, 4u);
    EXPECT_TRUE(v()->IsStorable(arr));
}

TEST_F(ValidatorIsStorableTest, ArrayUnsizedOfStorable) {
    auto* arr =
        create<type::Array>(create<type::I32>(), create<type::RuntimeArrayCount>(), 4u, 4u, 4u, 4u);
    EXPECT_TRUE(v()->IsStorable(arr));
}

}  // namespace
}  // namespace tint::resolver
