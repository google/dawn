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

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ResolverStorageClassValidationTest = ResolverTest;

TEST_F(ResolverStorageClassValidationTest, GlobalVariableNoStorageClass_Fail) {
    // var g : f32;
    Global(Source{{12, 34}}, "g", ty.f32(), ast::StorageClass::kNone);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: global variables must have a storage class");
}

TEST_F(ResolverStorageClassValidationTest, GlobalVariableFunctionStorageClass_Fail) {
    // var<function> g : f32;
    Global(Source{{12, 34}}, "g", ty.f32(), ast::StorageClass::kFunction);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: variables declared at module scope must not be in "
              "the function storage class");
}

TEST_F(ResolverStorageClassValidationTest, Private_RuntimeArray) {
    Global(Source{{12, 34}}, "v", ty.array(ty.i32()), ast::StorageClass::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: runtime-sized arrays can only be used in the <storage> storage class
12:34 note: while instantiating variable v)");
}

TEST_F(ResolverStorageClassValidationTest, Private_RuntimeArrayInStruct) {
    auto* s = Structure("S", {Member("m", ty.array(ty.i32()))});
    Global(Source{{12, 34}}, "v", ty.Of(s), ast::StorageClass::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: runtime-sized arrays can only be used in the <storage> storage class
note: while analysing structure member S.m
12:34 note: while instantiating variable v)");
}

TEST_F(ResolverStorageClassValidationTest, Workgroup_RuntimeArray) {
    Global(Source{{12, 34}}, "v", ty.array(ty.i32()), ast::StorageClass::kWorkgroup);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: runtime-sized arrays can only be used in the <storage> storage class
12:34 note: while instantiating variable v)");
}

TEST_F(ResolverStorageClassValidationTest, Workgroup_RuntimeArrayInStruct) {
    auto* s = Structure("S", {Member("m", ty.array(ty.i32()))});
    Global(Source{{12, 34}}, "v", ty.Of(s), ast::StorageClass::kWorkgroup);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: runtime-sized arrays can only be used in the <storage> storage class
note: while analysing structure member S.m
12:34 note: while instantiating variable v)");
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferBool) {
    // var<storage> g : bool;
    Global(Source{{56, 78}}, "g", ty.bool_(), ast::StorageClass::kStorage,
           ast::AttributeList{
               create<ast::BindingAttribute>(0),
               create<ast::GroupAttribute>(0),
           });

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: Type 'bool' cannot be used in storage class 'storage' as it is non-host-shareable
56:78 note: while instantiating variable g)");
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferPointer) {
    // var<storage> g : ptr<private, f32>;
    Global(Source{{56, 78}}, "g", ty.pointer(ty.f32(), ast::StorageClass::kPrivate),
           ast::StorageClass::kStorage,
           ast::AttributeList{
               create<ast::BindingAttribute>(0),
               create<ast::GroupAttribute>(0),
           });

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: Type 'ptr<private, f32, read_write>' cannot be used in storage class 'storage' as it is non-host-shareable
56:78 note: while instantiating variable g)");
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferIntScalar) {
    // var<storage> g : i32;
    Global(Source{{56, 78}}, "g", ty.i32(), ast::StorageClass::kStorage,
           ast::AttributeList{
               create<ast::BindingAttribute>(0),
               create<ast::GroupAttribute>(0),
           });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferVector) {
    // var<storage> g : vec4<f32>;
    Global(Source{{56, 78}}, "g", ty.vec4<f32>(), ast::StorageClass::kStorage,
           ast::AttributeList{
               create<ast::BindingAttribute>(0),
               create<ast::GroupAttribute>(0),
           });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferArray) {
    // var<storage, read> g : array<S, 3u>;
    auto* s = Structure("S", {Member("a", ty.f32())});
    auto* a = ty.array(ty.Of(s), 3_u);
    Global(Source{{56, 78}}, "g", a, ast::StorageClass::kStorage, ast::Access::kRead,
           ast::AttributeList{
               create<ast::BindingAttribute>(0),
               create<ast::GroupAttribute>(0),
           });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferBoolAlias) {
    // type a = bool;
    // var<storage, read> g : a;
    auto* a = Alias("a", ty.bool_());
    Global(Source{{56, 78}}, "g", ty.Of(a), ast::StorageClass::kStorage,
           ast::AttributeList{
               create<ast::BindingAttribute>(0),
               create<ast::GroupAttribute>(0),
           });

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: Type 'bool' cannot be used in storage class 'storage' as it is non-host-shareable
56:78 note: while instantiating variable g)");
}

TEST_F(ResolverStorageClassValidationTest, NotStorage_AccessMode) {
    // var<private, read> g : a;
    Global(Source{{56, 78}}, "g", ty.i32(), ast::StorageClass::kPrivate, ast::Access::kRead);

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: only variables in <storage> storage class may declare an access mode)");
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferNoError_Basic) {
    // struct S { x : i32 };
    // var<storage, read> g : S;
    auto* s = Structure("S", {Member(Source{{12, 34}}, "x", ty.i32())});
    Global(Source{{56, 78}}, "g", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kRead,
           ast::AttributeList{
               create<ast::BindingAttribute>(0),
               create<ast::GroupAttribute>(0),
           });

    ASSERT_TRUE(r()->Resolve());
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferNoError_Aliases) {
    // struct S { x : i32 };
    // type a1 = S;
    // var<storage, read> g : a1;
    auto* s = Structure("S", {Member(Source{{12, 34}}, "x", ty.i32())});
    auto* a1 = Alias("a1", ty.Of(s));
    auto* a2 = Alias("a2", ty.Of(a1));
    Global(Source{{56, 78}}, "g", ty.Of(a2), ast::StorageClass::kStorage, ast::Access::kRead,
           ast::AttributeList{
               create<ast::BindingAttribute>(0),
               create<ast::GroupAttribute>(0),
           });

    ASSERT_TRUE(r()->Resolve());
}

TEST_F(ResolverStorageClassValidationTest, UniformBuffer_Struct_Runtime) {
    // struct S { m:  array<f32>; };
    // @group(0) @binding(0) var<uniform, > svar : S;

    auto* s = Structure(Source{{12, 34}}, "S", {Member("m", ty.array<i32>())});

    Global(Source{{56, 78}}, "svar", ty.Of(s), ast::StorageClass::kUniform,
           ast::AttributeList{
               create<ast::BindingAttribute>(0),
               create<ast::GroupAttribute>(0),
           });

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(56:78 error: runtime-sized arrays can only be used in the <storage> storage class
note: while analysing structure member S.m
56:78 note: while instantiating variable svar)");
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferBool) {
    // var<uniform> g : bool;
    Global(Source{{56, 78}}, "g", ty.bool_(), ast::StorageClass::kUniform,
           ast::AttributeList{
               create<ast::BindingAttribute>(0),
               create<ast::GroupAttribute>(0),
           });

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: Type 'bool' cannot be used in storage class 'uniform' as it is non-host-shareable
56:78 note: while instantiating variable g)");
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferPointer) {
    // var<uniform> g : ptr<private, f32>;
    Global(Source{{56, 78}}, "g", ty.pointer(ty.f32(), ast::StorageClass::kPrivate),
           ast::StorageClass::kUniform,
           ast::AttributeList{
               create<ast::BindingAttribute>(0),
               create<ast::GroupAttribute>(0),
           });

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: Type 'ptr<private, f32, read_write>' cannot be used in storage class 'uniform' as it is non-host-shareable
56:78 note: while instantiating variable g)");
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferIntScalar) {
    // var<uniform> g : i32;
    Global(Source{{56, 78}}, "g", ty.i32(), ast::StorageClass::kUniform,
           ast::AttributeList{
               create<ast::BindingAttribute>(0),
               create<ast::GroupAttribute>(0),
           });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferVector) {
    // var<uniform> g : vec4<f32>;
    Global(Source{{56, 78}}, "g", ty.vec4<f32>(), ast::StorageClass::kUniform,
           ast::AttributeList{
               create<ast::BindingAttribute>(0),
               create<ast::GroupAttribute>(0),
           });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferArray) {
    // struct S {
    //   @size(16) f : f32;
    // }
    // var<uniform> g : array<S, 3u>;
    auto* s = Structure("S", {Member("a", ty.f32(), {MemberSize(16)})});
    auto* a = ty.array(ty.Of(s), 3_u);
    Global(Source{{56, 78}}, "g", a, ast::StorageClass::kUniform,
           ast::AttributeList{
               create<ast::BindingAttribute>(0),
               create<ast::GroupAttribute>(0),
           });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferBoolAlias) {
    // type a = bool;
    // var<uniform> g : a;
    auto* a = Alias("a", ty.bool_());
    Global(Source{{56, 78}}, "g", ty.Of(a), ast::StorageClass::kUniform,
           ast::AttributeList{
               create<ast::BindingAttribute>(0),
               create<ast::GroupAttribute>(0),
           });

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: Type 'bool' cannot be used in storage class 'uniform' as it is non-host-shareable
56:78 note: while instantiating variable g)");
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferNoError_Basic) {
    // struct S { x : i32 };
    // var<uniform> g :  S;
    auto* s = Structure("S", {Member(Source{{12, 34}}, "x", ty.i32())});
    Global(Source{{56, 78}}, "g", ty.Of(s), ast::StorageClass::kUniform,
           ast::AttributeList{
               create<ast::BindingAttribute>(0),
               create<ast::GroupAttribute>(0),
           });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferNoError_Aliases) {
    // struct S { x : i32 };
    // type a1 = S;
    // var<uniform> g : a1;
    auto* s = Structure("S", {Member(Source{{12, 34}}, "x", ty.i32())});
    auto* a1 = Alias("a1", ty.Of(s));
    Global(Source{{56, 78}}, "g", ty.Of(a1), ast::StorageClass::kUniform,
           ast::AttributeList{
               create<ast::BindingAttribute>(0),
               create<ast::GroupAttribute>(0),
           });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

}  // namespace
}  // namespace tint::resolver
