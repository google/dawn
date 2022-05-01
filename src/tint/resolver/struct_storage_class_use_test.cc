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

namespace tint::resolver {
namespace {

using ResolverStorageClassUseTest = ResolverTest;

TEST_F(ResolverStorageClassUseTest, UnreachableStruct) {
    auto* s = Structure("S", {Member("a", ty.f32())});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<sem::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->StorageClassUsage().empty());
}

TEST_F(ResolverStorageClassUseTest, StructReachableFromParameter) {
    auto* s = Structure("S", {Member("a", ty.f32())});

    Func("f", {Param("param", ty.Of(s))}, ty.void_(), {}, {});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<sem::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->StorageClassUsage(), UnorderedElementsAre(ast::StorageClass::kNone));
}

TEST_F(ResolverStorageClassUseTest, StructReachableFromReturnType) {
    auto* s = Structure("S", {Member("a", ty.f32())});

    Func("f", {}, ty.Of(s), {Return(Construct(ty.Of(s)))}, {});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<sem::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->StorageClassUsage(), UnorderedElementsAre(ast::StorageClass::kNone));
}

TEST_F(ResolverStorageClassUseTest, StructReachableFromGlobal) {
    auto* s = Structure("S", {Member("a", ty.f32())});

    Global("g", ty.Of(s), ast::StorageClass::kPrivate);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<sem::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->StorageClassUsage(), UnorderedElementsAre(ast::StorageClass::kPrivate));
}

TEST_F(ResolverStorageClassUseTest, StructReachableViaGlobalAlias) {
    auto* s = Structure("S", {Member("a", ty.f32())});
    auto* a = Alias("A", ty.Of(s));
    Global("g", ty.Of(a), ast::StorageClass::kPrivate);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<sem::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->StorageClassUsage(), UnorderedElementsAre(ast::StorageClass::kPrivate));
}

TEST_F(ResolverStorageClassUseTest, StructReachableViaGlobalStruct) {
    auto* s = Structure("S", {Member("a", ty.f32())});
    auto* o = Structure("O", {Member("a", ty.Of(s))});
    Global("g", ty.Of(o), ast::StorageClass::kPrivate);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<sem::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->StorageClassUsage(), UnorderedElementsAre(ast::StorageClass::kPrivate));
}

TEST_F(ResolverStorageClassUseTest, StructReachableViaGlobalArray) {
    auto* s = Structure("S", {Member("a", ty.f32())});
    auto* a = ty.array(ty.Of(s), 3);
    Global("g", a, ast::StorageClass::kPrivate);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<sem::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->StorageClassUsage(), UnorderedElementsAre(ast::StorageClass::kPrivate));
}

TEST_F(ResolverStorageClassUseTest, StructReachableFromLocal) {
    auto* s = Structure("S", {Member("a", ty.f32())});

    WrapInFunction(Var("g", ty.Of(s)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<sem::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->StorageClassUsage(), UnorderedElementsAre(ast::StorageClass::kFunction));
}

TEST_F(ResolverStorageClassUseTest, StructReachableViaLocalAlias) {
    auto* s = Structure("S", {Member("a", ty.f32())});
    auto* a = Alias("A", ty.Of(s));
    WrapInFunction(Var("g", ty.Of(a)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<sem::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->StorageClassUsage(), UnorderedElementsAre(ast::StorageClass::kFunction));
}

TEST_F(ResolverStorageClassUseTest, StructReachableViaLocalStruct) {
    auto* s = Structure("S", {Member("a", ty.f32())});
    auto* o = Structure("O", {Member("a", ty.Of(s))});
    WrapInFunction(Var("g", ty.Of(o)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<sem::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->StorageClassUsage(), UnorderedElementsAre(ast::StorageClass::kFunction));
}

TEST_F(ResolverStorageClassUseTest, StructReachableViaLocalArray) {
    auto* s = Structure("S", {Member("a", ty.f32())});
    auto* a = ty.array(ty.Of(s), 3);
    WrapInFunction(Var("g", a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<sem::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->StorageClassUsage(), UnorderedElementsAre(ast::StorageClass::kFunction));
}

TEST_F(ResolverStorageClassUseTest, StructMultipleStorageClassUses) {
    auto* s = Structure("S", {Member("a", ty.f32())});
    Global("x", ty.Of(s), ast::StorageClass::kUniform,
           ast::AttributeList{
               create<ast::BindingAttribute>(0),
               create<ast::GroupAttribute>(0),
           });
    Global("y", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kRead,
           ast::AttributeList{
               create<ast::BindingAttribute>(1),
               create<ast::GroupAttribute>(0),
           });
    WrapInFunction(Var("g", ty.Of(s)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<sem::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->StorageClassUsage(),
                UnorderedElementsAre(ast::StorageClass::kUniform, ast::StorageClass::kStorage,
                                     ast::StorageClass::kFunction));
}

}  // namespace
}  // namespace tint::resolver
