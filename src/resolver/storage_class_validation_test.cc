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
            "12:34 error v-0022: global variables must have a storage class");
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferBool) {
  // var<storage> g : i32;
  Global(Source{{56, 78}}, "g", ty.i32(), ast::StorageClass::kStorage, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(56:78 error: variables declared in the <storage> storage class must be of an [[access]] qualified structure type)");
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferPointer) {
  // var<storage> g : ptr<i32, input>;
  Global(Source{{56, 78}}, "g", ty.pointer<i32>(ast::StorageClass::kInput),
         ast::StorageClass::kStorage, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(56:78 error: variables declared in the <storage> storage class must be of an [[access]] qualified structure type)");
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferArray) {
  // var<storage> g : [[access(read)]] array<S, 3>;
  auto* s = Structure("S", {Member("a", ty.f32())});
  auto* a = ty.array(s, 3);
  auto* ac = ty.access(ast::AccessControl::kReadOnly, a);
  Global(Source{{56, 78}}, "g", ac, ast::StorageClass::kStorage, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(56:78 error: variables declared in the <storage> storage class must be of an [[access]] qualified structure type)");
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferBoolAlias) {
  // type a = bool;
  // var<storage> g : [[access(read)]] a;
  auto* a = ty.alias("a", ty.bool_());
  AST().AddConstructedType(a);
  Global(Source{{56, 78}}, "g", a, ast::StorageClass::kStorage, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(56:78 error: variables declared in the <storage> storage class must be of an [[access]] qualified structure type)");
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferNoAccessControl) {
  // var<storage> g : S;
  auto* s = Structure("S", {Member(Source{{12, 34}}, "x", ty.i32())});
  Global(Source{{56, 78}}, "g", s, ast::StorageClass::kStorage, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(56:78 error: variables declared in the <storage> storage class must be of an [[access]] qualified structure type)");
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferNoBlockDecoration) {
  // struct S { x : i32 };
  // var<storage> g : [[access(read)]] S;
  auto* s = Structure(Source{{12, 34}}, "S", {Member("x", ty.i32())});
  auto* a = ty.access(ast::AccessControl::kReadOnly, s);
  Global(Source{{56, 78}}, "g", a, ast::StorageClass::kStorage, nullptr,
         {
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
  // var<storage> g : [[access(read)]] S;
  auto* s = Structure("S", {Member(Source{{12, 34}}, "x", ty.i32())},
                      {create<ast::StructBlockDecoration>()});
  auto* a = ty.access(ast::AccessControl::kReadOnly, s);
  Global(Source{{56, 78}}, "g", a, ast::StorageClass::kStorage, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_TRUE(r()->Resolve());
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferNoError_Aliases) {
  // [[block]] struct S { x : i32 };
  // type a1 = S;
  // type a2 = [[access(read)]] a1;
  // var<storage> g : a2;
  auto* s = Structure("S", {Member(Source{{12, 34}}, "x", ty.i32())},
                      {create<ast::StructBlockDecoration>()});
  auto* a1 = ty.alias("a1", s);
  AST().AddConstructedType(a1);
  auto* ac = ty.access(ast::AccessControl::kReadOnly, a1);
  auto* a2 = ty.alias("a2", ac);
  AST().AddConstructedType(a2);
  Global(Source{{56, 78}}, "g", a2, ast::StorageClass::kStorage, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_TRUE(r()->Resolve());
}

///

TEST_F(ResolverStorageClassValidationTest, UniformBufferBool) {
  // var<uniform> g : bool;
  Global(Source{{56, 78}}, "g", ty.bool_(), ast::StorageClass::kUniform,
         nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(56:78 error: variables declared in the <uniform> storage class must be of a structure type)");
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferPointer) {
  // var<uniform> g : ptr<i32, input>;
  Global(Source{{56, 78}}, "g", ty.pointer<i32>(ast::StorageClass::kInput),
         ast::StorageClass::kUniform, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(56:78 error: variables declared in the <uniform> storage class must be of a structure type)");
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferArray) {
  // var<uniform> g : [[access(read)]] array<S, 3>;
  auto* s = Structure("S", {Member("a", ty.f32())});
  auto* a = ty.array(s, 3);
  auto* ac = ty.access(ast::AccessControl::kReadOnly, a);
  Global(Source{{56, 78}}, "g", ac, ast::StorageClass::kUniform, nullptr,
         {
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
  // var<uniform> g : [[access(read)]] a;
  auto* a = ty.alias("a", ty.bool_());
  AST().AddConstructedType(a);
  Global(Source{{56, 78}}, "g", a, ast::StorageClass::kUniform, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(56:78 error: variables declared in the <uniform> storage class must be of a structure type)");
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferNoBlockDecoration) {
  // struct S { x : i32 };
  // var<uniform> g : S;
  auto* s = Structure(Source{{12, 34}}, "S", {Member("x", ty.i32())});
  Global(Source{{56, 78}}, "g", s, ast::StorageClass::kUniform, nullptr,
         {
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
  Global(Source{{56, 78}}, "g", s, ast::StorageClass::kUniform, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_TRUE(r()->Resolve());
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferNoError_Aliases) {
  // [[block]] struct S { x : i32 };
  // type a1 = S;
  // var<uniform> g : a1;
  auto* s = Structure("S", {Member(Source{{12, 34}}, "x", ty.i32())},
                      {create<ast::StructBlockDecoration>()});
  auto* a1 = ty.alias("a1", s);
  AST().AddConstructedType(a1);
  Global(Source{{56, 78}}, "g", a1, ast::StorageClass::kUniform, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_TRUE(r()->Resolve());
}

}  // namespace
}  // namespace resolver
}  // namespace tint
