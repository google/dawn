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

using ::testing::UnorderedElementsAre;

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ResolverAddressSpaceUseTest = ResolverTest;

TEST_F(ResolverAddressSpaceUseTest, UnreachableStruct) {
    auto* s = Structure("S", utils::Vector{Member("a", ty.f32())});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->AddressSpaceUsage().empty());
}

TEST_F(ResolverAddressSpaceUseTest, StructReachableFromParameter) {
    auto* s = Structure("S", utils::Vector{Member("a", ty.f32())});

    Func("f", utils::Vector{Param("param", ty.Of(s))}, ty.void_(), utils::Empty, utils::Empty);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->AddressSpaceUsage(), UnorderedElementsAre(builtin::AddressSpace::kUndefined));
}

TEST_F(ResolverAddressSpaceUseTest, StructReachableFromReturnType) {
    auto* s = Structure("S", utils::Vector{Member("a", ty.f32())});

    Func("f", utils::Empty, ty.Of(s), utils::Vector{Return(Call(ty.Of(s)))}, utils::Empty);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->AddressSpaceUsage(), UnorderedElementsAre(builtin::AddressSpace::kUndefined));
}

TEST_F(ResolverAddressSpaceUseTest, StructReachableFromGlobal) {
    auto* s = Structure("S", utils::Vector{Member("a", ty.f32())});

    GlobalVar("g", ty.Of(s), builtin::AddressSpace::kPrivate);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->AddressSpaceUsage(), UnorderedElementsAre(builtin::AddressSpace::kPrivate));
}

TEST_F(ResolverAddressSpaceUseTest, StructReachableViaGlobalAlias) {
    auto* s = Structure("S", utils::Vector{Member("a", ty.f32())});
    auto* a = Alias("A", ty.Of(s));
    GlobalVar("g", ty.Of(a), builtin::AddressSpace::kPrivate);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->AddressSpaceUsage(), UnorderedElementsAre(builtin::AddressSpace::kPrivate));
}

TEST_F(ResolverAddressSpaceUseTest, StructReachableViaGlobalStruct) {
    auto* s = Structure("S", utils::Vector{Member("a", ty.f32())});
    auto* o = Structure("O", utils::Vector{Member("a", ty.Of(s))});
    GlobalVar("g", ty.Of(o), builtin::AddressSpace::kPrivate);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->AddressSpaceUsage(), UnorderedElementsAre(builtin::AddressSpace::kPrivate));
}

TEST_F(ResolverAddressSpaceUseTest, StructReachableViaGlobalArray) {
    auto* s = Structure("S", utils::Vector{Member("a", ty.f32())});
    auto a = ty.array(ty.Of(s), 3_u);
    GlobalVar("g", a, builtin::AddressSpace::kPrivate);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->AddressSpaceUsage(), UnorderedElementsAre(builtin::AddressSpace::kPrivate));
}

TEST_F(ResolverAddressSpaceUseTest, StructReachableFromLocal) {
    auto* s = Structure("S", utils::Vector{Member("a", ty.f32())});

    WrapInFunction(Var("g", ty.Of(s)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->AddressSpaceUsage(), UnorderedElementsAre(builtin::AddressSpace::kFunction));
}

TEST_F(ResolverAddressSpaceUseTest, StructReachableViaLocalAlias) {
    auto* s = Structure("S", utils::Vector{Member("a", ty.f32())});
    auto* a = Alias("A", ty.Of(s));
    WrapInFunction(Var("g", ty.Of(a)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->AddressSpaceUsage(), UnorderedElementsAre(builtin::AddressSpace::kFunction));
}

TEST_F(ResolverAddressSpaceUseTest, StructReachableViaLocalStruct) {
    auto* s = Structure("S", utils::Vector{Member("a", ty.f32())});
    auto* o = Structure("O", utils::Vector{Member("a", ty.Of(s))});
    WrapInFunction(Var("g", ty.Of(o)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->AddressSpaceUsage(), UnorderedElementsAre(builtin::AddressSpace::kFunction));
}

TEST_F(ResolverAddressSpaceUseTest, StructReachableViaLocalArray) {
    auto* s = Structure("S", utils::Vector{Member("a", ty.f32())});
    auto a = ty.array(ty.Of(s), 3_u);
    WrapInFunction(Var("g", a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->AddressSpaceUsage(), UnorderedElementsAre(builtin::AddressSpace::kFunction));
}

TEST_F(ResolverAddressSpaceUseTest, StructMultipleAddressSpaceUses) {
    auto* s = Structure("S", utils::Vector{Member("a", ty.f32())});
    GlobalVar("x", ty.Of(s), builtin::AddressSpace::kUniform, Binding(0_a), Group(0_a));
    GlobalVar("y", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kRead, Binding(1_a),
              Group(0_a));
    WrapInFunction(Var("g", ty.Of(s)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->AddressSpaceUsage(), UnorderedElementsAre(builtin::AddressSpace::kUniform,
                                                               builtin::AddressSpace::kStorage,
                                                               builtin::AddressSpace::kFunction));
}

}  // namespace
}  // namespace tint::resolver
