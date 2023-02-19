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
#include "src/tint/type/atomic.h"

namespace tint::resolver {
namespace {

using ResolverIsStorableTest = ResolverTest;

TEST_F(ResolverIsStorableTest, Void) {
    EXPECT_FALSE(r()->IsStorable(create<type::Void>()));
}

TEST_F(ResolverIsStorableTest, Scalar) {
    EXPECT_TRUE(r()->IsStorable(create<type::Bool>()));
    EXPECT_TRUE(r()->IsStorable(create<type::I32>()));
    EXPECT_TRUE(r()->IsStorable(create<type::U32>()));
    EXPECT_TRUE(r()->IsStorable(create<type::F32>()));
    EXPECT_TRUE(r()->IsStorable(create<type::F16>()));
}

TEST_F(ResolverIsStorableTest, Vector) {
    EXPECT_TRUE(r()->IsStorable(create<type::Vector>(create<type::I32>(), 2u)));
    EXPECT_TRUE(r()->IsStorable(create<type::Vector>(create<type::I32>(), 3u)));
    EXPECT_TRUE(r()->IsStorable(create<type::Vector>(create<type::I32>(), 4u)));
    EXPECT_TRUE(r()->IsStorable(create<type::Vector>(create<type::U32>(), 2u)));
    EXPECT_TRUE(r()->IsStorable(create<type::Vector>(create<type::U32>(), 3u)));
    EXPECT_TRUE(r()->IsStorable(create<type::Vector>(create<type::U32>(), 4u)));
    EXPECT_TRUE(r()->IsStorable(create<type::Vector>(create<type::F32>(), 2u)));
    EXPECT_TRUE(r()->IsStorable(create<type::Vector>(create<type::F32>(), 3u)));
    EXPECT_TRUE(r()->IsStorable(create<type::Vector>(create<type::F32>(), 4u)));
    EXPECT_TRUE(r()->IsStorable(create<type::Vector>(create<type::F16>(), 2u)));
    EXPECT_TRUE(r()->IsStorable(create<type::Vector>(create<type::F16>(), 3u)));
    EXPECT_TRUE(r()->IsStorable(create<type::Vector>(create<type::F16>(), 4u)));
}

TEST_F(ResolverIsStorableTest, Matrix) {
    auto* vec2_f32 = create<type::Vector>(create<type::F32>(), 2u);
    auto* vec3_f32 = create<type::Vector>(create<type::F32>(), 3u);
    auto* vec4_f32 = create<type::Vector>(create<type::F32>(), 4u);
    auto* vec2_f16 = create<type::Vector>(create<type::F16>(), 2u);
    auto* vec3_f16 = create<type::Vector>(create<type::F16>(), 3u);
    auto* vec4_f16 = create<type::Vector>(create<type::F16>(), 4u);
    EXPECT_TRUE(r()->IsStorable(create<type::Matrix>(vec2_f32, 2u)));
    EXPECT_TRUE(r()->IsStorable(create<type::Matrix>(vec2_f32, 3u)));
    EXPECT_TRUE(r()->IsStorable(create<type::Matrix>(vec2_f32, 4u)));
    EXPECT_TRUE(r()->IsStorable(create<type::Matrix>(vec3_f32, 2u)));
    EXPECT_TRUE(r()->IsStorable(create<type::Matrix>(vec3_f32, 3u)));
    EXPECT_TRUE(r()->IsStorable(create<type::Matrix>(vec3_f32, 4u)));
    EXPECT_TRUE(r()->IsStorable(create<type::Matrix>(vec4_f32, 2u)));
    EXPECT_TRUE(r()->IsStorable(create<type::Matrix>(vec4_f32, 3u)));
    EXPECT_TRUE(r()->IsStorable(create<type::Matrix>(vec4_f32, 4u)));
    EXPECT_TRUE(r()->IsStorable(create<type::Matrix>(vec2_f16, 2u)));
    EXPECT_TRUE(r()->IsStorable(create<type::Matrix>(vec2_f16, 3u)));
    EXPECT_TRUE(r()->IsStorable(create<type::Matrix>(vec2_f16, 4u)));
    EXPECT_TRUE(r()->IsStorable(create<type::Matrix>(vec3_f16, 2u)));
    EXPECT_TRUE(r()->IsStorable(create<type::Matrix>(vec3_f16, 3u)));
    EXPECT_TRUE(r()->IsStorable(create<type::Matrix>(vec3_f16, 4u)));
    EXPECT_TRUE(r()->IsStorable(create<type::Matrix>(vec4_f16, 2u)));
    EXPECT_TRUE(r()->IsStorable(create<type::Matrix>(vec4_f16, 3u)));
    EXPECT_TRUE(r()->IsStorable(create<type::Matrix>(vec4_f16, 4u)));
}

TEST_F(ResolverIsStorableTest, Pointer) {
    auto* ptr = create<type::Pointer>(create<type::I32>(), builtin::AddressSpace::kPrivate,
                                      builtin::Access::kReadWrite);
    EXPECT_FALSE(r()->IsStorable(ptr));
}

TEST_F(ResolverIsStorableTest, Atomic) {
    EXPECT_TRUE(r()->IsStorable(create<type::Atomic>(create<type::I32>())));
    EXPECT_TRUE(r()->IsStorable(create<type::Atomic>(create<type::U32>())));
}

TEST_F(ResolverIsStorableTest, ArraySizedOfStorable) {
    auto* arr = create<type::Array>(create<type::I32>(), create<type::ConstantArrayCount>(5u), 4u,
                                    20u, 4u, 4u);
    EXPECT_TRUE(r()->IsStorable(arr));
}

TEST_F(ResolverIsStorableTest, ArrayUnsizedOfStorable) {
    auto* arr =
        create<type::Array>(create<type::I32>(), create<type::RuntimeArrayCount>(), 4u, 4u, 4u, 4u);
    EXPECT_TRUE(r()->IsStorable(arr));
}

TEST_F(ResolverIsStorableTest, Struct_AllMembersStorable) {
    Structure("S", utils::Vector{
                       Member("a", ty.i32()),
                       Member("b", ty.f32()),
                   });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverIsStorableTest, Struct_SomeMembersNonStorable) {
    Structure("S", utils::Vector{
                       Member("a", ty.i32()),
                       Member("b", ty.pointer<i32>(builtin::AddressSpace::kPrivate)),
                   });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: ptr<private, i32, read_write> cannot be used as the type of a structure member)");
}

TEST_F(ResolverIsStorableTest, Struct_NestedStorable) {
    auto* storable = Structure("Storable", utils::Vector{
                                               Member("a", ty.i32()),
                                               Member("b", ty.f32()),
                                           });
    Structure("S", utils::Vector{
                       Member("a", ty.i32()),
                       Member("b", ty.Of(storable)),
                   });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverIsStorableTest, Struct_NestedNonStorable) {
    auto* non_storable =
        Structure("nonstorable", utils::Vector{
                                     Member("a", ty.i32()),
                                     Member("b", ty.pointer<i32>(builtin::AddressSpace::kPrivate)),
                                 });
    Structure("S", utils::Vector{
                       Member("a", ty.i32()),
                       Member("b", ty.Of(non_storable)),
                   });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: ptr<private, i32, read_write> cannot be used as the type of a structure member)");
}

}  // namespace
}  // namespace tint::resolver
