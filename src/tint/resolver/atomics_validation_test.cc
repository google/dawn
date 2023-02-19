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
#include "src/tint/type/atomic.h"
#include "src/tint/type/reference.h"

#include "gmock/gmock.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

struct ResolverAtomicValidationTest : public resolver::TestHelper, public testing::Test {};

TEST_F(ResolverAtomicValidationTest, AddressSpace_WorkGroup) {
    GlobalVar("a", ty.atomic(Source{{12, 34}}, ty.i32()), builtin::AddressSpace::kWorkgroup);

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverAtomicValidationTest, AddressSpace_Storage) {
    GlobalVar("g", ty.atomic(Source{{12, 34}}, ty.i32()), builtin::AddressSpace::kStorage,
              builtin::Access::kReadWrite, Group(0_a), Binding(0_a));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAtomicValidationTest, AddressSpace_Storage_Struct) {
    auto* s = Structure("s", utils::Vector{Member(Source{{12, 34}}, "a", ty.atomic(ty.i32()))});
    GlobalVar("g", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kReadWrite,
              Group(0_a), Binding(0_a));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAtomicValidationTest, InvalidType) {
    GlobalVar("a", ty.atomic(ty.f32(Source{{12, 34}})), builtin::AddressSpace::kWorkgroup);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: atomic only supports i32 or u32 types");
}

TEST_F(ResolverAtomicValidationTest, InvalidAddressSpace_Simple) {
    GlobalVar(Source{{12, 34}}, "a", ty.atomic(ty.i32()), builtin::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: atomic variables must have <storage> or <workgroup> address space");
}

TEST_F(ResolverAtomicValidationTest, InvalidAddressSpace_Array) {
    GlobalVar(Source{{12, 34}}, "a", ty.atomic(ty.i32()), builtin::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: atomic variables must have <storage> or <workgroup> address space");
}

TEST_F(ResolverAtomicValidationTest, InvalidAddressSpace_Struct) {
    auto* s = Structure("s", utils::Vector{Member("a", ty.atomic(ty.i32()))});
    GlobalVar(Source{{56, 78}}, "g", ty.Of(s), builtin::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "56:78 error: atomic variables must have <storage> or <workgroup> address space\n"
              "note: atomic sub-type of 's' is declared here");
}

TEST_F(ResolverAtomicValidationTest, InvalidAddressSpace_StructOfStruct) {
    // struct Inner { m : atomic<i32>; };
    // struct Outer { m : array<Inner, 4>; };
    // var<private> g : Outer;

    auto* Inner =
        Structure("Inner", utils::Vector{Member("m", ty.atomic(Source{{12, 34}}, ty.i32()))});
    auto* Outer = Structure("Outer", utils::Vector{Member("m", ty.Of(Inner))});
    GlobalVar(Source{{56, 78}}, "g", ty.Of(Outer), builtin::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "56:78 error: atomic variables must have <storage> or <workgroup> address space\n"
              "note: atomic sub-type of 'Outer' is declared here");
}

TEST_F(ResolverAtomicValidationTest, InvalidAddressSpace_StructOfStructOfArray) {
    // struct Inner { m : array<atomic<i32>, 4>; };
    // struct Outer { m : array<Inner, 4>; };
    // var<private> g : Outer;

    auto* Inner =
        Structure("Inner", utils::Vector{Member(Source{{12, 34}}, "m", ty.atomic(ty.i32()))});
    auto* Outer = Structure("Outer", utils::Vector{Member("m", ty.Of(Inner))});
    GlobalVar(Source{{56, 78}}, "g", ty.Of(Outer), builtin::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(56:78 error: atomic variables must have <storage> or <workgroup> address space
12:34 note: atomic sub-type of 'Outer' is declared here)");
}

TEST_F(ResolverAtomicValidationTest, InvalidAddressSpace_ArrayOfArray) {
    // type AtomicArray = array<atomic<i32>, 5>;
    // var<private> v: array<s, 5>;

    auto* atomic_array =
        Alias(Source{{12, 34}}, "AtomicArray", ty.atomic(Source{{12, 34}}, ty.i32()));
    GlobalVar(Source{{56, 78}}, "v", ty.Of(atomic_array), builtin::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "56:78 error: atomic variables must have <storage> or <workgroup> address space");
}

TEST_F(ResolverAtomicValidationTest, InvalidAddressSpace_ArrayOfStruct) {
    // struct S{
    //   m: atomic<u32>;
    // };
    // var<private> v: array<S, 5u>;

    auto* s = Structure("S", utils::Vector{Member(Source{{12, 34}}, "m", ty.atomic<u32>())});
    GlobalVar(Source{{56, 78}}, "v", ty.array(ty.Of(s), 5_u), builtin::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(56:78 error: atomic variables must have <storage> or <workgroup> address space
12:34 note: atomic sub-type of 'array<S, 5>' is declared here)");
}

TEST_F(ResolverAtomicValidationTest, InvalidAddressSpace_ArrayOfStructOfArray) {
    // type AtomicArray = array<atomic<i32>, 5u>;
    // struct S{
    //   m: AtomicArray;
    // };
    // var<private> v: array<S, 5u>;

    auto* atomic_array = Alias("AtomicArray", ty.atomic(ty.i32()));
    auto* s = Structure("S", utils::Vector{Member(Source{{12, 34}}, "m", ty.Of(atomic_array))});
    GlobalVar(Source{{56, 78}}, "v", ty.array(ty.Of(s), 5_u), builtin::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(56:78 error: atomic variables must have <storage> or <workgroup> address space
12:34 note: atomic sub-type of 'array<S, 5>' is declared here)");
}

TEST_F(ResolverAtomicValidationTest, InvalidAddressSpace_Complex) {
    // type AtomicArray = array<atomic<i32>, 5u>;
    // struct S6 { x: array<i32, 4>; };
    // struct S5 { x: S6;
    //             y: AtomicArray;
    //             z: array<atomic<u32>, 8u>; };
    // struct S4 { x: S6;
    //             y: S5;
    //             z: array<atomic<i32>, 4u>; };
    // struct S3 { x: S4; };
    // struct S2 { x: S3; };
    // struct S1 { x: S2; };
    // struct S0 { x: S1; };
    // var<private> g : S0;

    auto* atomic_array = Alias("AtomicArray", ty.atomic(ty.i32()));
    auto array_i32_4 = ty.array<i32, 4>();
    auto array_atomic_u32_8 = ty.array(ty.atomic(ty.u32()), 8_u);
    auto array_atomic_i32_4 = ty.array(ty.atomic(ty.i32()), 4_u);

    auto* s6 = Structure("S6", utils::Vector{Member("x", array_i32_4)});
    auto* s5 = Structure("S5", utils::Vector{Member("x", ty.Of(s6)),                              //
                                             Member(Source{{12, 34}}, "y", ty.Of(atomic_array)),  //
                                             Member("z", array_atomic_u32_8)});                   //
    auto* s4 = Structure("S4", utils::Vector{Member("x", ty.Of(s6)),                              //
                                             Member("y", ty.Of(s5)),                              //
                                             Member("z", array_atomic_i32_4)});                   //
    auto* s3 = Structure("S3", utils::Vector{Member("x", ty.Of(s4))});
    auto* s2 = Structure("S2", utils::Vector{Member("x", ty.Of(s3))});
    auto* s1 = Structure("S1", utils::Vector{Member("x", ty.Of(s2))});
    auto* s0 = Structure("S0", utils::Vector{Member("x", ty.Of(s1))});
    GlobalVar(Source{{56, 78}}, "g", ty.Of(s0), builtin::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(56:78 error: atomic variables must have <storage> or <workgroup> address space
12:34 note: atomic sub-type of 'S0' is declared here)");
}

TEST_F(ResolverAtomicValidationTest, Struct_AccessMode_Read) {
    auto* s = Structure("s", utils::Vector{Member(Source{{12, 34}}, "a", ty.atomic(ty.i32()))});
    GlobalVar(Source{{56, 78}}, "g", ty.Of(s), builtin::AddressSpace::kStorage,
              builtin::Access::kRead, Group(0_a), Binding(0_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: atomic variables in <storage> address space must have read_write access mode
12:34 note: atomic sub-type of 's' is declared here)");
}

TEST_F(ResolverAtomicValidationTest, InvalidAccessMode_Struct) {
    auto* s = Structure("s", utils::Vector{Member(Source{{12, 34}}, "a", ty.atomic(ty.i32()))});
    GlobalVar(Source{{56, 78}}, "g", ty.Of(s), builtin::AddressSpace::kStorage,
              builtin::Access::kRead, Group(0_a), Binding(0_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: atomic variables in <storage> address space must have read_write access mode
12:34 note: atomic sub-type of 's' is declared here)");
}

TEST_F(ResolverAtomicValidationTest, InvalidAccessMode_StructOfStruct) {
    // struct Inner { m : atomic<i32>; };
    // struct Outer { m : array<Inner, 4>; };
    // var<storage, read> g : Outer;

    auto* Inner =
        Structure("Inner", utils::Vector{Member(Source{{12, 34}}, "m", ty.atomic(ty.i32()))});
    auto* Outer = Structure("Outer", utils::Vector{Member("m", ty.Of(Inner))});
    GlobalVar(Source{{56, 78}}, "g", ty.Of(Outer), builtin::AddressSpace::kStorage,
              builtin::Access::kRead, Group(0_a), Binding(0_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: atomic variables in <storage> address space must have read_write access mode
12:34 note: atomic sub-type of 'Outer' is declared here)");
}

TEST_F(ResolverAtomicValidationTest, InvalidAccessMode_StructOfStructOfArray) {
    // struct Inner { m : array<atomic<i32>, 4>; };
    // struct Outer { m : array<Inner, 4>; };
    // var<storage, read> g : Outer;

    auto* Inner =
        Structure("Inner", utils::Vector{Member(Source{{12, 34}}, "m", ty.atomic(ty.i32()))});
    auto* Outer = Structure("Outer", utils::Vector{Member("m", ty.Of(Inner))});
    GlobalVar(Source{{56, 78}}, "g", ty.Of(Outer), builtin::AddressSpace::kStorage,
              builtin::Access::kRead, Group(0_a), Binding(0_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: atomic variables in <storage> address space must have read_write access mode
12:34 note: atomic sub-type of 'Outer' is declared here)");
}

TEST_F(ResolverAtomicValidationTest, InvalidAccessMode_Complex) {
    // type AtomicArray = array<atomic<i32>, 5>;
    // struct S6 { x: array<i32, 4u>; };
    // struct S5 { x: S6;
    //             y: AtomicArray;
    //             z: array<atomic<u32>, 8u>; };
    // struct S4 { x: S6;
    //             y: S5;
    //             z: array<atomic<i32>, 4u>; };
    // struct S3 { x: S4; };
    // struct S2 { x: S3; };
    // struct S1 { x: S2; };
    // struct S0 { x: S1; };
    // var<storage, read> g : S0;

    auto* atomic_array = Alias("AtomicArray", ty.atomic(ty.i32()));
    auto array_i32_4 = ty.array<i32, 4>();
    auto array_atomic_u32_8 = ty.array(ty.atomic(ty.u32()), 8_u);
    auto array_atomic_i32_4 = ty.array(ty.atomic(ty.i32()), 4_u);

    auto* s6 = Structure("S6", utils::Vector{Member("x", array_i32_4)});
    auto* s5 = Structure("S5", utils::Vector{Member("x", ty.Of(s6)),                              //
                                             Member(Source{{56, 78}}, "y", ty.Of(atomic_array)),  //
                                             Member("z", array_atomic_u32_8)});                   //
    auto* s4 = Structure("S4", utils::Vector{Member("x", ty.Of(s6)),                              //
                                             Member("y", ty.Of(s5)),                              //
                                             Member("z", array_atomic_i32_4)});                   //
    auto* s3 = Structure("S3", utils::Vector{Member("x", ty.Of(s4))});
    auto* s2 = Structure("S2", utils::Vector{Member("x", ty.Of(s3))});
    auto* s1 = Structure("S1", utils::Vector{Member("x", ty.Of(s2))});
    auto* s0 = Structure("S0", utils::Vector{Member("x", ty.Of(s1))});
    GlobalVar(Source{{12, 34}}, "g", ty.Of(s0), builtin::AddressSpace::kStorage,
              builtin::Access::kRead, Group(0_a), Binding(0_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: atomic variables in <storage> address space must have read_write access mode
56:78 note: atomic sub-type of 'S0' is declared here)");
}

TEST_F(ResolverAtomicValidationTest, Local) {
    WrapInFunction(Var("a", ty.atomic(Source{{12, 34}}, ty.i32())));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: function-scope 'var' must have a constructible type");
}

}  // namespace
}  // namespace tint::resolver
