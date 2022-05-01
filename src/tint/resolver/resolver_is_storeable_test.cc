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

using ResolverIsStorableTest = ResolverTest;

TEST_F(ResolverIsStorableTest, Struct_AllMembersStorable) {
    Structure("S", {
                       Member("a", ty.i32()),
                       Member("b", ty.f32()),
                   });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverIsStorableTest, Struct_SomeMembersNonStorable) {
    Structure("S", {
                       Member("a", ty.i32()),
                       Member("b", ty.pointer<i32>(ast::StorageClass::kPrivate)),
                   });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: ptr<private, i32, read_write> cannot be used as the type of a structure member)");
}

TEST_F(ResolverIsStorableTest, Struct_NestedStorable) {
    auto* storable = Structure("Storable", {
                                               Member("a", ty.i32()),
                                               Member("b", ty.f32()),
                                           });
    Structure("S", {
                       Member("a", ty.i32()),
                       Member("b", ty.Of(storable)),
                   });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverIsStorableTest, Struct_NestedNonStorable) {
    auto* non_storable =
        Structure("nonstorable", {
                                     Member("a", ty.i32()),
                                     Member("b", ty.pointer<i32>(ast::StorageClass::kPrivate)),
                                 });
    Structure("S", {
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
