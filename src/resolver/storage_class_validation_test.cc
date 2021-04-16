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
#include "src/resolver/resolver_test_helper.h"
#include "src/semantic/struct.h"
#include "src/type/access_control_type.h"

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

TEST_F(ResolverStorageClassValidationTest, Bool) {
  // var<storage> g : bool;
  Global(Source{{56, 78}}, "g", ty.bool_(), ast::StorageClass::kStorage);

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(56:78 error: variables declared in the <storage> storage class must be of an [[access]] qualified structure type)");
}

TEST_F(ResolverStorageClassValidationTest, Pointer) {
  // var<storage> g : ptr<i32, input>;
  Global(Source{{56, 78}}, "g", ty.pointer<i32>(ast::StorageClass::kInput),
         ast::StorageClass::kStorage);

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(56:78 error: variables declared in the <storage> storage class must be of an [[access]] qualified structure type)");
}

TEST_F(ResolverStorageClassValidationTest, Array) {
  // var<storage> g : [[access(read)]] array<S, 3>;
  auto* s = Structure("S", {Member("a", ty.f32())});
  auto* a = ty.array(s, 3);
  auto* ac = ty.access(ast::AccessControl::kReadOnly, a);
  Global(Source{{56, 78}}, "g", ac, ast::StorageClass::kStorage);

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(56:78 error: variables declared in the <storage> storage class must be of an [[access]] qualified structure type)");
}

TEST_F(ResolverStorageClassValidationTest, BoolAlias) {
  // type a = bool;
  // var<storage> g : [[access(read)]] a;
  auto* a = ty.alias("a", ty.bool_());
  Global(Source{{56, 78}}, "g", a, ast::StorageClass::kStorage);

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(56:78 error: variables declared in the <storage> storage class must be of an [[access]] qualified structure type)");
}

TEST_F(ResolverStorageClassValidationTest, NoAccessControl) {
  // var<storage> g : S;
  auto* s = Structure("S", {Member(Source{{12, 34}}, "x", ty.i32())});
  Global(Source{{56, 78}}, "g", s, ast::StorageClass::kStorage);

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(56:78 error: variables declared in the <storage> storage class must be of an [[access]] qualified structure type)");
}

TEST_F(ResolverStorageClassValidationTest, NoError_Basic) {
  // var<storage> g : [[access(read)]] S;
  auto* s = Structure("S", {Member(Source{{12, 34}}, "x", ty.i32())});
  auto* a = ty.access(ast::AccessControl::kReadOnly, s);
  Global(Source{{56, 78}}, "g", a, ast::StorageClass::kStorage);

  ASSERT_TRUE(r()->Resolve());
}

TEST_F(ResolverStorageClassValidationTest, NoError_Aliases) {
  // type a1 = S;
  // type a2 = [[access(read)]] a1;
  // var<storage> g : a2;
  auto* s = Structure("S", {Member(Source{{12, 34}}, "x", ty.i32())});
  auto* a1 = ty.alias("a1", s);
  auto* ac = ty.access(ast::AccessControl::kReadOnly, a1);
  auto* a2 = ty.alias("a2", ac);
  Global(Source{{56, 78}}, "g", a2, ast::StorageClass::kStorage);

  ASSERT_TRUE(r()->Resolve());
}

}  // namespace
}  // namespace resolver
}  // namespace tint
