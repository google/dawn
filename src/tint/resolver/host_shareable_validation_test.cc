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
#include "src/tint/sem/struct.h"

namespace tint::resolver {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using ResolverHostShareableValidationTest = ResolverTest;

TEST_F(ResolverHostShareableValidationTest, BoolMember) {
    auto* s =
        Structure("S", utils::Vector{Member(Source{{56, 78}}, "x", ty.bool_(Source{{12, 34}}))});

    GlobalVar(Source{{90, 12}}, "g", ty.Of(s), builtin::AddressSpace::kStorage,
              builtin::Access::kRead, Binding(0_a), Group(0_a));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: Type 'bool' cannot be used in address space 'storage' as it is non-host-shareable
56:78 note: while analyzing structure member S.x
90:12 note: while instantiating 'var' g)");
}

TEST_F(ResolverHostShareableValidationTest, BoolVectorMember) {
    auto* s = Structure(
        "S", utils::Vector{Member(Source{{56, 78}}, "x", ty.vec3<bool>(Source{{12, 34}}))});

    GlobalVar(Source{{90, 12}}, "g", ty.Of(s), builtin::AddressSpace::kStorage,
              builtin::Access::kRead, Binding(0_a), Group(0_a));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: Type 'vec3<bool>' cannot be used in address space 'storage' as it is non-host-shareable
56:78 note: while analyzing structure member S.x
90:12 note: while instantiating 'var' g)");
}

TEST_F(ResolverHostShareableValidationTest, Aliases) {
    Alias("a1", ty.bool_());
    auto* s =
        Structure("S", utils::Vector{Member(Source{{56, 78}}, "x", ty(Source{{12, 34}}, "a1"))});
    auto* a2 = Alias("a2", ty.Of(s));
    GlobalVar(Source{{90, 12}}, "g", ty.Of(a2), builtin::AddressSpace::kStorage,
              builtin::Access::kRead, Binding(0_a), Group(0_a));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: Type 'bool' cannot be used in address space 'storage' as it is non-host-shareable
56:78 note: while analyzing structure member S.x
90:12 note: while instantiating 'var' g)");
}

TEST_F(ResolverHostShareableValidationTest, NestedStructures) {
    auto* i1 = Structure("I1", utils::Vector{Member(Source{{1, 2}}, "x", ty.bool_())});
    auto* i2 = Structure("I2", utils::Vector{Member(Source{{3, 4}}, "y", ty.Of(i1))});
    auto* i3 = Structure("I3", utils::Vector{Member(Source{{5, 6}}, "z", ty.Of(i2))});

    auto* s = Structure("S", utils::Vector{Member(Source{{7, 8}}, "m", ty.Of(i3))});

    GlobalVar(Source{{9, 10}}, "g", ty.Of(s), builtin::AddressSpace::kStorage,
              builtin::Access::kRead, Binding(0_a), Group(0_a));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(error: Type 'bool' cannot be used in address space 'storage' as it is non-host-shareable
1:2 note: while analyzing structure member I1.x
3:4 note: while analyzing structure member I2.y
5:6 note: while analyzing structure member I3.z
7:8 note: while analyzing structure member S.m
9:10 note: while instantiating 'var' g)");
}

TEST_F(ResolverHostShareableValidationTest, NoError) {
    Enable(builtin::Extension::kF16);

    auto* i1 = Structure("I1", utils::Vector{
                                   Member(Source{{1, 1}}, "w1", ty.f32()),
                                   Member(Source{{2, 1}}, "x1", ty.f32()),
                                   Member(Source{{3, 1}}, "y1", ty.vec3<f32>()),
                                   Member(Source{{4, 1}}, "z1", ty.array<i32, 4>()),
                               });
    auto* a1 = Alias("a1", ty.Of(i1));
    auto* i2 = Structure("I2", utils::Vector{
                                   Member(Source{{5, 1}}, "x2", ty.mat2x2<f32>()),
                                   Member(Source{{6, 1}}, "w2", ty.mat3x4<f32>()),
                                   Member(Source{{7, 1}}, "z2", ty.Of(i1)),
                               });
    auto* a2 = Alias("a2", ty.Of(i2));
    auto* i3 = Structure("I3", utils::Vector{
                                   Member(Source{{4, 1}}, "x3", ty.Of(a1)),
                                   Member(Source{{5, 1}}, "y3", ty.Of(i2)),
                                   Member(Source{{6, 1}}, "z3", ty.Of(a2)),
                               });

    auto* s = Structure("S", utils::Vector{Member(Source{{7, 8}}, "m", ty.Of(i3))});

    GlobalVar(Source{{9, 10}}, "g", ty.Of(s), builtin::AddressSpace::kStorage,
              builtin::Access::kRead, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

}  // namespace
}  // namespace tint::resolver
