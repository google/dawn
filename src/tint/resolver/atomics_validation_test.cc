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
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/sem/atomic.h"
#include "src/tint/sem/reference.h"

#include "gmock/gmock.h"

namespace tint::resolver {
namespace {

struct ResolverAtomicValidationTest : public resolver::TestHelper,
                                      public testing::Test {};

TEST_F(ResolverAtomicValidationTest, StorageClass_WorkGroup) {
  Global("a", ty.atomic(Source{{12, 34}}, ty.i32()),
         ast::StorageClass::kWorkgroup);

  EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverAtomicValidationTest, StorageClass_Storage) {
  Global("g", ty.atomic(Source{{12, 34}}, ty.i32()),
         ast::StorageClass::kStorage, ast::Access::kReadWrite,
         GroupAndBinding(0, 0));

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAtomicValidationTest, StorageClass_Storage_Struct) {
  auto* s =
      Structure("s", {Member("a", ty.atomic(Source{{12, 34}}, ty.i32()))});
  Global("g", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kReadWrite,
         GroupAndBinding(0, 0));

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAtomicValidationTest, InvalidType) {
  Global("a", ty.atomic(ty.f32(Source{{12, 34}})),
         ast::StorageClass::kWorkgroup);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error: atomic only supports i32 or u32 types");
}

TEST_F(ResolverAtomicValidationTest, InvalidStorageClass_Simple) {
  Global("a", ty.atomic(Source{{12, 34}}, ty.i32()),
         ast::StorageClass::kPrivate);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: atomic variables must have <storage> or <workgroup> "
            "storage class");
}

TEST_F(ResolverAtomicValidationTest, InvalidStorageClass_Array) {
  Global("a", ty.atomic(Source{{12, 34}}, ty.i32()),
         ast::StorageClass::kPrivate);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: atomic variables must have <storage> or <workgroup> "
            "storage class");
}

TEST_F(ResolverAtomicValidationTest, InvalidStorageClass_Struct) {
  auto* s =
      Structure("s", {Member("a", ty.atomic(Source{{12, 34}}, ty.i32()))});
  Global("g", ty.Of(s), ast::StorageClass::kPrivate);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "error: atomic variables must have <storage> or <workgroup> "
            "storage class\n"
            "note: atomic sub-type of 's' is declared here");
}

TEST_F(ResolverAtomicValidationTest, InvalidStorageClass_StructOfStruct) {
  // struct Inner { m : atomic<i32>; };
  // struct Outer { m : array<Inner, 4>; };
  // var<private> g : Outer;

  auto* Inner =
      Structure("Inner", {Member("m", ty.atomic(Source{{12, 34}}, ty.i32()))});
  auto* Outer = Structure("Outer", {Member("m", ty.Of(Inner))});
  Global("g", ty.Of(Outer), ast::StorageClass::kPrivate);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "error: atomic variables must have <storage> or <workgroup> "
            "storage class\n"
            "note: atomic sub-type of 'Outer' is declared here");
}

TEST_F(ResolverAtomicValidationTest,
       InvalidStorageClass_StructOfStructOfArray) {
  // struct Inner { m : array<atomic<i32>, 4>; };
  // struct Outer { m : array<Inner, 4>; };
  // var<private> g : Outer;

  auto* Inner =
      Structure("Inner", {Member(Source{{12, 34}}, "m", ty.atomic(ty.i32()))});
  auto* Outer = Structure("Outer", {Member("m", ty.Of(Inner))});
  Global("g", ty.Of(Outer), ast::StorageClass::kPrivate);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "error: atomic variables must have <storage> or <workgroup> "
            "storage class\n"
            "12:34 note: atomic sub-type of 'Outer' is declared here");
}

TEST_F(ResolverAtomicValidationTest, InvalidStorageClass_ArrayOfArray) {
  // type AtomicArray = array<atomic<i32>, 5>;
  // var<private> v: array<s, 5>;

  auto* atomic_array = Alias(Source{{12, 34}}, "AtomicArray",
                             ty.atomic(Source{{12, 34}}, ty.i32()));
  Global(Source{{56, 78}}, "v", ty.Of(atomic_array),
         ast::StorageClass::kPrivate);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "error: atomic variables must have <storage> or <workgroup> "
            "storage class");
}

TEST_F(ResolverAtomicValidationTest, InvalidStorageClass_ArrayOfStruct) {
  // struct S{
  //   m: atomic<u32>;
  // };
  // var<private> v: array<S, 5>;

  auto* s = Structure("S", {Member("m", ty.atomic<u32>())});
  Global(Source{{56, 78}}, "v", ty.array(ty.Of(s), 5),
         ast::StorageClass::kPrivate);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "error: atomic variables must have <storage> or <workgroup> "
            "storage class\n"
            "note: atomic sub-type of 'array<S, 5>' is declared here");
}

TEST_F(ResolverAtomicValidationTest, InvalidStorageClass_ArrayOfStructOfArray) {
  // type AtomicArray = array<atomic<i32>, 5>;
  // struct S{
  //   m: AtomicArray;
  // };
  // var<private> v: array<S, 5>;

  auto* atomic_array = Alias(Source{{12, 34}}, "AtomicArray",
                             ty.atomic(Source{{12, 34}}, ty.i32()));
  auto* s = Structure("S", {Member("m", ty.Of(atomic_array))});
  Global(Source{{56, 78}}, "v", ty.array(ty.Of(s), 5),
         ast::StorageClass::kPrivate);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "error: atomic variables must have <storage> or <workgroup> "
            "storage class\n"
            "note: atomic sub-type of 'array<S, 5>' is declared here");
}

TEST_F(ResolverAtomicValidationTest, InvalidStorageClass_Complex) {
  // type AtomicArray = array<atomic<i32>, 5>;
  // struct S6 { x: array<i32, 4>; };
  // struct S5 { x: S6;
  //             y: AtomicArray;
  //             z: array<atomic<u32>, 8>; };
  // struct S4 { x: S6;
  //             y: S5;
  //             z: array<atomic<i32>, 4>; };
  // struct S3 { x: S4; };
  // struct S2 { x: S3; };
  // struct S1 { x: S2; };
  // struct S0 { x: S1; };
  // var<private> g : S0;

  auto* atomic_array = Alias(Source{{12, 34}}, "AtomicArray",
                             ty.atomic(Source{{12, 34}}, ty.i32()));
  auto* array_i32_4 = ty.array(ty.i32(), 4);
  auto* array_atomic_u32_8 = ty.array(ty.atomic(ty.u32()), 8);
  auto* array_atomic_i32_4 = ty.array(ty.atomic(ty.i32()), 4);

  auto* s6 = Structure("S6", {Member("x", array_i32_4)});
  auto* s5 = Structure("S5", {Member("x", ty.Of(s6)),             //
                              Member("y", ty.Of(atomic_array)),   //
                              Member("z", array_atomic_u32_8)});  //
  auto* s4 = Structure("S4", {Member("x", ty.Of(s6)),             //
                              Member("y", ty.Of(s5)),             //
                              Member("z", array_atomic_i32_4)});  //
  auto* s3 = Structure("S3", {Member("x", ty.Of(s4))});
  auto* s2 = Structure("S2", {Member("x", ty.Of(s3))});
  auto* s1 = Structure("S1", {Member("x", ty.Of(s2))});
  auto* s0 = Structure("S0", {Member("x", ty.Of(s1))});
  Global(Source{{56, 78}}, "g", ty.Of(s0), ast::StorageClass::kPrivate);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "error: atomic variables must have <storage> or <workgroup> "
            "storage class\n"
            "note: atomic sub-type of 'S0' is declared here");
}

TEST_F(ResolverAtomicValidationTest, Struct_AccessMode_Read) {
  auto* s =
      Structure("s", {Member("a", ty.atomic(Source{{12, 34}}, ty.i32()))});
  Global(Source{{56, 78}}, "g", ty.Of(s), ast::StorageClass::kStorage,
         ast::Access::kRead, GroupAndBinding(0, 0));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "error: atomic variables in <storage> storage class must have read_write "
      "access mode\n"
      "note: atomic sub-type of 's' is declared here");
}

TEST_F(ResolverAtomicValidationTest, InvalidAccessMode_Struct) {
  auto* s =
      Structure("s", {Member("a", ty.atomic(Source{{12, 34}}, ty.i32()))});
  Global(Source{{56, 78}}, "g", ty.Of(s), ast::StorageClass::kStorage,
         ast::Access::kRead, GroupAndBinding(0, 0));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "error: atomic variables in <storage> storage class must have read_write "
      "access mode\n"
      "note: atomic sub-type of 's' is declared here");
}

TEST_F(ResolverAtomicValidationTest, InvalidAccessMode_StructOfStruct) {
  // struct Inner { m : atomic<i32>; };
  // struct Outer { m : array<Inner, 4>; };
  // var<storage, read> g : Outer;

  auto* Inner =
      Structure("Inner", {Member("m", ty.atomic(Source{{12, 34}}, ty.i32()))});
  auto* Outer = Structure("Outer", {Member("m", ty.Of(Inner))});
  Global(Source{{56, 78}}, "g", ty.Of(Outer), ast::StorageClass::kStorage,
         ast::Access::kRead, GroupAndBinding(0, 0));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "error: atomic variables in <storage> storage class must have read_write "
      "access mode\n"
      "note: atomic sub-type of 'Outer' is declared here");
}

TEST_F(ResolverAtomicValidationTest, InvalidAccessMode_StructOfStructOfArray) {
  // struct Inner { m : array<atomic<i32>, 4>; };
  // struct Outer { m : array<Inner, 4>; };
  // var<storage, read> g : Outer;

  auto* Inner =
      Structure("Inner", {Member(Source{{12, 34}}, "m", ty.atomic(ty.i32()))});
  auto* Outer = Structure("Outer", {Member("m", ty.Of(Inner))});
  Global(Source{{56, 78}}, "g", ty.Of(Outer), ast::StorageClass::kStorage,
         ast::Access::kRead, GroupAndBinding(0, 0));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "error: atomic variables in <storage> storage class must have "
            "read_write access mode\n"
            "12:34 note: atomic sub-type of 'Outer' is declared here");
}

TEST_F(ResolverAtomicValidationTest, InvalidAccessMode_Complex) {
  // type AtomicArray = array<atomic<i32>, 5>;
  // struct S6 { x: array<i32, 4>; };
  // struct S5 { x: S6;
  //             y: AtomicArray;
  //             z: array<atomic<u32>, 8>; };
  // struct S4 { x: S6;
  //             y: S5;
  //             z: array<atomic<i32>, 4>; };
  // struct S3 { x: S4; };
  // struct S2 { x: S3; };
  // struct S1 { x: S2; };
  // struct S0 { x: S1; };
  // var<storage, read> g : S0;

  auto* atomic_array = Alias(Source{{12, 34}}, "AtomicArray",
                             ty.atomic(Source{{12, 34}}, ty.i32()));
  auto* array_i32_4 = ty.array(ty.i32(), 4);
  auto* array_atomic_u32_8 = ty.array(ty.atomic(ty.u32()), 8);
  auto* array_atomic_i32_4 = ty.array(ty.atomic(ty.i32()), 4);

  auto* s6 = Structure("S6", {Member("x", array_i32_4)});
  auto* s5 = Structure("S5", {Member("x", ty.Of(s6)),             //
                              Member("y", ty.Of(atomic_array)),   //
                              Member("z", array_atomic_u32_8)});  //
  auto* s4 = Structure("S4", {Member("x", ty.Of(s6)),             //
                              Member("y", ty.Of(s5)),             //
                              Member("z", array_atomic_i32_4)});  //
  auto* s3 = Structure("S3", {Member("x", ty.Of(s4))});
  auto* s2 = Structure("S2", {Member("x", ty.Of(s3))});
  auto* s1 = Structure("S1", {Member("x", ty.Of(s2))});
  auto* s0 = Structure("S0", {Member("x", ty.Of(s1))});
  Global(Source{{56, 78}}, "g", ty.Of(s0), ast::StorageClass::kStorage,
         ast::Access::kRead, GroupAndBinding(0, 0));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "error: atomic variables in <storage> storage class must have "
            "read_write access mode\n"
            "note: atomic sub-type of 'S0' is declared here");
}

TEST_F(ResolverAtomicValidationTest, Local) {
  WrapInFunction(Var("a", ty.atomic(Source{{12, 34}}, ty.i32())));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: function variable must have a constructible type");
}

}  // namespace
}  // namespace tint::resolver
