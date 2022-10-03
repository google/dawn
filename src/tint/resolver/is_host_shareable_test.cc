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

#include "gmock/gmock.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/sem/atomic.h"

namespace tint::resolver {
namespace {

using ResolverIsHostShareable = ResolverTest;

TEST_F(ResolverIsHostShareable, Void) {
    EXPECT_FALSE(r()->IsHostShareable(create<sem::Void>()));
}

TEST_F(ResolverIsHostShareable, Bool) {
    EXPECT_FALSE(r()->IsHostShareable(create<sem::Bool>()));
}

TEST_F(ResolverIsHostShareable, NumericScalar) {
    EXPECT_TRUE(r()->IsHostShareable(create<sem::I32>()));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::U32>()));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::F32>()));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::F16>()));
}

TEST_F(ResolverIsHostShareable, NumericVector) {
    EXPECT_TRUE(r()->IsHostShareable(create<sem::Vector>(create<sem::I32>(), 2u)));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::Vector>(create<sem::I32>(), 3u)));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::Vector>(create<sem::I32>(), 4u)));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::Vector>(create<sem::U32>(), 2u)));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::Vector>(create<sem::U32>(), 3u)));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::Vector>(create<sem::U32>(), 4u)));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::Vector>(create<sem::F32>(), 2u)));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::Vector>(create<sem::F32>(), 3u)));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::Vector>(create<sem::F32>(), 4u)));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::Vector>(create<sem::F16>(), 2u)));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::Vector>(create<sem::F16>(), 3u)));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::Vector>(create<sem::F16>(), 4u)));
}

TEST_F(ResolverIsHostShareable, BoolVector) {
    EXPECT_FALSE(r()->IsHostShareable(create<sem::Vector>(create<sem::Bool>(), 2u)));
    EXPECT_FALSE(r()->IsHostShareable(create<sem::Vector>(create<sem::Bool>(), 3u)));
    EXPECT_FALSE(r()->IsHostShareable(create<sem::Vector>(create<sem::Bool>(), 4u)));
    EXPECT_FALSE(r()->IsHostShareable(create<sem::Vector>(create<sem::Bool>(), 2u)));
    EXPECT_FALSE(r()->IsHostShareable(create<sem::Vector>(create<sem::Bool>(), 3u)));
    EXPECT_FALSE(r()->IsHostShareable(create<sem::Vector>(create<sem::Bool>(), 4u)));
    EXPECT_FALSE(r()->IsHostShareable(create<sem::Vector>(create<sem::Bool>(), 2u)));
    EXPECT_FALSE(r()->IsHostShareable(create<sem::Vector>(create<sem::Bool>(), 3u)));
    EXPECT_FALSE(r()->IsHostShareable(create<sem::Vector>(create<sem::Bool>(), 4u)));
}

TEST_F(ResolverIsHostShareable, Matrix) {
    auto* vec2_f32 = create<sem::Vector>(create<sem::F32>(), 2u);
    auto* vec3_f32 = create<sem::Vector>(create<sem::F32>(), 3u);
    auto* vec4_f32 = create<sem::Vector>(create<sem::F32>(), 4u);
    auto* vec2_f16 = create<sem::Vector>(create<sem::F16>(), 2u);
    auto* vec3_f16 = create<sem::Vector>(create<sem::F16>(), 3u);
    auto* vec4_f16 = create<sem::Vector>(create<sem::F16>(), 4u);

    EXPECT_TRUE(r()->IsHostShareable(create<sem::Matrix>(vec2_f32, 2u)));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::Matrix>(vec2_f32, 3u)));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::Matrix>(vec2_f32, 4u)));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::Matrix>(vec3_f32, 2u)));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::Matrix>(vec3_f32, 3u)));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::Matrix>(vec3_f32, 4u)));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::Matrix>(vec4_f32, 2u)));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::Matrix>(vec4_f32, 3u)));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::Matrix>(vec4_f32, 4u)));

    EXPECT_TRUE(r()->IsHostShareable(create<sem::Matrix>(vec2_f16, 2u)));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::Matrix>(vec2_f16, 3u)));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::Matrix>(vec2_f16, 4u)));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::Matrix>(vec3_f16, 2u)));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::Matrix>(vec3_f16, 3u)));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::Matrix>(vec3_f16, 4u)));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::Matrix>(vec4_f16, 2u)));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::Matrix>(vec4_f16, 3u)));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::Matrix>(vec4_f16, 4u)));
}

TEST_F(ResolverIsHostShareable, Pointer) {
    auto* ptr = create<sem::Pointer>(create<sem::I32>(), ast::AddressSpace::kPrivate,
                                     ast::Access::kReadWrite);
    EXPECT_FALSE(r()->IsHostShareable(ptr));
}

TEST_F(ResolverIsHostShareable, Atomic) {
    EXPECT_TRUE(r()->IsHostShareable(create<sem::Atomic>(create<sem::I32>())));
    EXPECT_TRUE(r()->IsHostShareable(create<sem::Atomic>(create<sem::U32>())));
}

TEST_F(ResolverIsHostShareable, ArraySizedOfHostShareable) {
    auto* arr =
        create<sem::Array>(create<sem::I32>(), sem::ConstantArrayCount{5u}, 4u, 20u, 4u, 4u);
    EXPECT_TRUE(r()->IsHostShareable(arr));
}

TEST_F(ResolverIsHostShareable, ArrayUnsizedOfHostShareable) {
    auto* arr = create<sem::Array>(create<sem::I32>(), sem::RuntimeArrayCount{}, 4u, 4u, 4u, 4u);
    EXPECT_TRUE(r()->IsHostShareable(arr));
}

// Note: Structure tests covered in host_shareable_validation_test.cc

}  // namespace
}  // namespace tint::resolver
