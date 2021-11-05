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

#include "src/resolver/resolver.h"

#include "gmock/gmock.h"
#include "src/ast/struct_block_decoration.h"
#include "src/resolver/resolver_test_helper.h"
#include "src/sem/struct.h"

namespace tint {
namespace resolver {
namespace {

using ResolverStorageClassValidationTest = ResolverTest;

TEST_F(ResolverStorageClassValidationTest, GlobalVariableNoStorageClass_Fail) {
  // var g : f32;
  Global(Source{{12, 34}}, "g", ty.f32(), ast::StorageClass::kNone);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: global variables must have a storage class");
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferBool) {
  // var<storage> g : i32;
  Global(Source{{56, 78}}, "g", ty.i32(), ast::StorageClass::kStorage,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(56:78 error: variables declared in the <storage> storage class must be of a structure type)");
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferPointer) {
  // var<storage> g : vec4<f32>;
  Global(Source{{56, 78}}, "g", ty.vec4<f32>(), ast::StorageClass::kStorage,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(56:78 error: variables declared in the <storage> storage class must be of a structure type)");
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferArray) {
  // var<storage, read> g : array<S, 3>;
  auto* s = Structure("S", {Member("a", ty.f32())});
  auto* a = ty.array(ty.Of(s), 3);
  Global(Source{{56, 78}}, "g", a, ast::StorageClass::kStorage,
         ast::Access::kRead,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(56:78 error: variables declared in the <storage> storage class must be of a structure type)");
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferBoolAlias) {
  // type a = bool;
  // var<storage, read> g : a;
  auto* a = Alias("a", ty.bool_());
  Global(Source{{56, 78}}, "g", ty.Of(a), ast::StorageClass::kStorage,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(56:78 error: Type 'bool' cannot be used in storage class 'storage' as it is non-host-shareable
56:78 note: while instantiating variable g)");
}

TEST_F(ResolverStorageClassValidationTest, NotStorage_AccessMode) {
  // var<private, read> g : a;
  Global(Source{{56, 78}}, "g", ty.i32(), ast::StorageClass::kPrivate,
         ast::Access::kRead);

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(56:78 error: only variables in <storage> storage class may declare an access mode)");
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferNoBlockDecoration) {
  // struct S { x : i32 };
  // var<storage, read> g : S;
  auto* s = Structure(Source{{12, 34}}, "S", {Member("x", ty.i32())});
  Global(Source{{56, 78}}, "g", ty.Of(s), ast::StorageClass::kStorage,
         ast::Access::kRead,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: structure used as a storage buffer must be declared with the [[block]] decoration
56:78 note: structure used as storage buffer here)");
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferNoError_Basic) {
  // [[block]] struct S { x : i32 };
  // var<storage, read> g : S;
  auto* s = Structure("S", {Member(Source{{12, 34}}, "x", ty.i32())},
                      {create<ast::StructBlockDecoration>()});
  Global(Source{{56, 78}}, "g", ty.Of(s), ast::StorageClass::kStorage,
         ast::Access::kRead,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_TRUE(r()->Resolve());
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferNoError_Aliases) {
  // [[block]] struct S { x : i32 };
  // type a1 = S;
  // var<storage, read> g : a1;
  auto* s = Structure("S", {Member(Source{{12, 34}}, "x", ty.i32())},
                      {create<ast::StructBlockDecoration>()});
  auto* a1 = Alias("a1", ty.Of(s));
  auto* a2 = Alias("a2", ty.Of(a1));
  Global(Source{{56, 78}}, "g", ty.Of(a2), ast::StorageClass::kStorage,
         ast::Access::kRead,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_TRUE(r()->Resolve());
}

TEST_F(ResolverStorageClassValidationTest, UniformBuffer_Struct_Runtime) {
  // [[block]] struct S { m:  array<f32>; };
  // [[group(0), binding(0)]] var<uniform, > svar : S;

  auto* s = Structure(Source{{12, 34}}, "S", {Member("m", ty.array<i32>())},
                      {create<ast::StructBlockDecoration>()});

  Global(Source{{56, 78}}, "svar", ty.Of(s), ast::StorageClass::kUniform,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "56:78 error: structure containing a runtime sized array cannot be "
            "used as a uniform buffer\n12:34 note: structure is declared here");
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferBool) {
  // var<uniform> g : bool;
  Global(Source{{56, 78}}, "g", ty.bool_(), ast::StorageClass::kUniform,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(56:78 error: Type 'bool' cannot be used in storage class 'uniform' as it is non-host-shareable
56:78 note: while instantiating variable g)");
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferPointer) {
  // var<uniform> g : vec4<f32>;
  Global(Source{{56, 78}}, "g", ty.vec4<f32>(), ast::StorageClass::kUniform,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(56:78 error: variables declared in the <uniform> storage class must be of a structure type)");
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferArray) {
  // var<uniform> g : array<S, 3>;
  auto* s = Structure("S", {Member("a", ty.f32())});
  auto* a = ty.array(ty.Of(s), 3);
  Global(Source{{56, 78}}, "g", a, ast::StorageClass::kUniform,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(56:78 error: variables declared in the <uniform> storage class must be of a structure type)");
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferBoolAlias) {
  // type a = bool;
  // var<uniform> g : a;
  auto* a = Alias("a", ty.bool_());
  Global(Source{{56, 78}}, "g", ty.Of(a), ast::StorageClass::kUniform,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(56:78 error: Type 'bool' cannot be used in storage class 'uniform' as it is non-host-shareable
56:78 note: while instantiating variable g)");
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferNoBlockDecoration) {
  // struct S { x : i32 };
  // var<uniform> g : S;
  auto* s = Structure(Source{{12, 34}}, "S", {Member("x", ty.i32())});
  Global(Source{{56, 78}}, "g", ty.Of(s), ast::StorageClass::kUniform,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: structure used as a uniform buffer must be declared with the [[block]] decoration
56:78 note: structure used as uniform buffer here)");
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferNoError_Basic) {
  // [[block]] struct S { x : i32 };
  // var<uniform> g :  S;
  auto* s = Structure("S", {Member(Source{{12, 34}}, "x", ty.i32())},
                      {create<ast::StructBlockDecoration>()});
  Global(Source{{56, 78}}, "g", ty.Of(s), ast::StorageClass::kUniform,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferNoError_Aliases) {
  // [[block]] struct S { x : i32 };
  // type a1 = S;
  // var<uniform> g : a1;
  auto* s = Structure("S", {Member(Source{{12, 34}}, "x", ty.i32())},
                      {create<ast::StructBlockDecoration>()});
  auto* a1 = Alias("a1", ty.Of(s));
  Global(Source{{56, 78}}, "g", ty.Of(a1), ast::StorageClass::kUniform,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_TRUE(r()->Resolve()) << r()->error();
}

}  // namespace
}  // namespace resolver
}  // namespace tint
