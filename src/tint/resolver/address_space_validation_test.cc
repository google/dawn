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

using ::testing::HasSubstr;

using ResolverAddressSpaceValidationTest = ResolverTest;

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_NoAddressSpace_Fail) {
    // var g : f32;
    GlobalVar(Source{{12, 34}}, "g", ty.f32());

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: module-scope 'var' declaration must have a address space");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_NoAddressSpace_Fail) {
    // type g = ptr<f32>;
    Alias("g", ty.pointer(Source{{12, 34}}, ty.f32(), ast::AddressSpace::kUndefined));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: ptr missing address space");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_FunctionAddressSpace_Fail) {
    // var<private> g : f32;
    GlobalVar(Source{{12, 34}}, "g", ty.f32(), ast::AddressSpace::kFunction);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: module-scope 'var' must not use address space 'function'");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Private_RuntimeArray) {
    // var<private> v : array<i32>;
    GlobalVar(Source{{56, 78}}, "v", ty.array(Source{{12, 34}}, ty.i32()),
              ast::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: runtime-sized arrays can only be used in the <storage> address space
56:78 note: while instantiating 'var' v)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Private_RuntimeArray) {
    // type t : ptr<private, array<i32>>;
    Alias("t", ty.pointer(Source{{56, 78}}, ty.array(Source{{12, 34}}, ty.i32()),
                          ast::AddressSpace::kPrivate));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: runtime-sized arrays can only be used in the <storage> address space
56:78 note: while instantiating ptr<private, array<i32>, read_write>)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Private_RuntimeArrayInStruct) {
    // struct S { m : array<i32> };
    // var<private> v : S;
    Structure("S", utils::Vector{Member(Source{{12, 34}}, "m", ty.array(ty.i32()))});
    GlobalVar(Source{{56, 78}}, "v", ty.type_name("S"), ast::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: runtime-sized arrays can only be used in the <storage> address space
12:34 note: while analyzing structure member S.m
56:78 note: while instantiating 'var' v)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Private_RuntimeArrayInStruct) {
    // struct S { m : array<i32> };
    // type t = ptr<private, S>;
    Structure("S", utils::Vector{Member(Source{{12, 34}}, "m", ty.array(ty.i32()))});
    Alias("t", ty.pointer(ty.type_name("S"), ast::AddressSpace::kPrivate));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: runtime-sized arrays can only be used in the <storage> address space
12:34 note: while analyzing structure member S.m
note: while instantiating ptr<private, S, read_write>)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Workgroup_RuntimeArray) {
    // var<workgroup> v : array<i32>;
    GlobalVar(Source{{56, 78}}, "v", ty.array(Source{{12, 34}}, ty.i32()),
              ast::AddressSpace::kWorkgroup);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: runtime-sized arrays can only be used in the <storage> address space
56:78 note: while instantiating 'var' v)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Workgroup_RuntimeArray) {
    // type t = ptr<workgroup, array<i32>>;
    Alias("t", ty.pointer(ty.array(Source{{12, 34}}, ty.i32()), ast::AddressSpace::kWorkgroup));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: runtime-sized arrays can only be used in the <storage> address space
note: while instantiating ptr<workgroup, array<i32>, read_write>)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Workgroup_RuntimeArrayInStruct) {
    // struct S { m : array<i32> };
    // var<workgroup> v : S;
    Structure("S", utils::Vector{Member(Source{{12, 34}}, "m", ty.array(ty.i32()))});
    GlobalVar(Source{{56, 78}}, "v", ty.type_name("S"), ast::AddressSpace::kWorkgroup);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: runtime-sized arrays can only be used in the <storage> address space
12:34 note: while analyzing structure member S.m
56:78 note: while instantiating 'var' v)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Workgroup_RuntimeArrayInStruct) {
    // struct S { m : array<i32> };
    // type t = ptr<workgroup, S>;
    Structure("S", utils::Vector{Member(Source{{12, 34}}, "m", ty.array(ty.i32()))});
    Alias(Source{{56, 78}}, "t", ty.pointer(ty.type_name("S"), ast::AddressSpace::kWorkgroup));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: runtime-sized arrays can only be used in the <storage> address space
12:34 note: while analyzing structure member S.m
note: while instantiating ptr<workgroup, S, read_write>)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_Bool) {
    // var<storage> g : bool;
    GlobalVar(Source{{56, 78}}, "g", ty.bool_(Source{{12, 34}}), ast::AddressSpace::kStorage,
              Binding(0_a), Group(0_a));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: Type 'bool' cannot be used in address space 'storage' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_Bool) {
    // type t = ptr<storage, bool>;
    Alias(Source{{56, 78}}, "t",
          ty.pointer(ty.bool_(Source{{12, 34}}), ast::AddressSpace::kStorage));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: Type 'bool' cannot be used in address space 'storage' as it is non-host-shareable
note: while instantiating ptr<storage, bool, read>)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_BoolAlias) {
    // type a = bool;
    // @binding(0) @group(0) var<storage, read> g : a;
    Alias("a", ty.bool_());
    GlobalVar(Source{{56, 78}}, "g", ty.type_name(Source{{12, 34}}, "a"),
              ast::AddressSpace::kStorage, Binding(0_a), Group(0_a));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: Type 'bool' cannot be used in address space 'storage' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_BoolAlias) {
    // type a = bool;
    // type t = ptr<storage, a>;
    Alias("a", ty.bool_());
    Alias(Source{{56, 78}}, "t",
          ty.pointer(ty.type_name(Source{{12, 34}}, "a"), ast::AddressSpace::kStorage));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: Type 'bool' cannot be used in address space 'storage' as it is non-host-shareable
note: while instantiating ptr<storage, bool, read>)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_Pointer) {
    // var<storage> g : ptr<private, f32>;
    GlobalVar(Source{{56, 78}}, "g",
              ty.pointer(Source{{12, 34}}, ty.f32(), ast::AddressSpace::kPrivate),
              ast::AddressSpace::kStorage, Binding(0_a), Group(0_a));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: Type 'ptr<private, f32, read_write>' cannot be used in address space 'storage' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_Pointer) {
    // type t = ptr<storage, ptr<private, f32>>;
    Alias("t", ty.pointer(Source{{56, 78}},
                          ty.pointer(Source{{12, 34}}, ty.f32(), ast::AddressSpace::kPrivate),
                          ast::AddressSpace::kStorage));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: Type 'ptr<private, f32, read_write>' cannot be used in address space 'storage' as it is non-host-shareable
56:78 note: while instantiating ptr<storage, ptr<private, f32, read_write>, read>)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_IntScalar) {
    // var<storage> g : i32;
    GlobalVar("g", ty.i32(), ast::AddressSpace::kStorage, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_IntScalar) {
    // type t = ptr<storage, i32;
    Alias("t", ty.pointer(ty.i32(), ast::AddressSpace::kStorage));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_F16) {
    // enable f16;
    // var<storage> g : f16;
    Enable(ast::Extension::kF16);

    GlobalVar("g", ty.f16(), ast::AddressSpace::kStorage, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_F16) {
    // enable f16;
    // type t = ptr<storage, f16>;
    Enable(ast::Extension::kF16);

    Alias("t", ty.pointer(ty.f16(), ast::AddressSpace::kStorage));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_F16Alias) {
    // enable f16;
    // type a = f16;
    // var<storage, read> g : a;
    Enable(ast::Extension::kF16);

    Alias("a", ty.f16());
    GlobalVar("g", ty.type_name("a"), ast::AddressSpace::kStorage, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_F16Alias) {
    // enable f16;
    // type a = f16;
    // type t = ptr<storage, a>;
    Enable(ast::Extension::kF16);

    Alias("a", ty.f16());
    Alias("t", ty.pointer(ty.type_name("a"), ast::AddressSpace::kStorage));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_VectorF32) {
    // var<storage> g : vec4<f32>;
    GlobalVar("g", ty.vec4<f32>(), ast::AddressSpace::kStorage, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_VectorF32) {
    // type t = ptr<storage, vec4<f32>>;
    Alias("t", ty.pointer(ty.vec4<f32>(), ast::AddressSpace::kStorage));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_VectorF16) {
    // var<storage> g : vec4<f16>;
    Enable(ast::Extension::kF16);
    GlobalVar("g", ty.vec(ty.f16(), 4u), ast::AddressSpace::kStorage, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_VectorF16) {
    // type t = ptr<storage, vec4<f16>>;
    Enable(ast::Extension::kF16);
    Alias("t", ty.pointer(ty.vec(ty.f16(), 4u), ast::AddressSpace::kStorage));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_ArrayF32) {
    // struct S{ a : f32 };
    // var<storage, read> g : array<S, 3u>;
    Structure("S", utils::Vector{Member("a", ty.f32())});
    GlobalVar("g", ty.array(ty.type_name("S"), 3_u), ast::AddressSpace::kStorage,
              ast::Access::kRead, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_ArrayF32) {
    // struct S{ a : f32 };
    // type t = ptr<storage, array<S, 3u>>;
    Structure("S", utils::Vector{Member("a", ty.f32())});
    Alias("t", ty.pointer(ty.array(ty.type_name("S"), 3_u), ast::AddressSpace::kStorage));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_ArrayF16) {
    // enable f16;
    // struct S{ a : f16 };
    // var<storage, read> g : array<S, 3u>;
    Enable(ast::Extension::kF16);

    Structure("S", utils::Vector{Member("a", ty.f16())});
    GlobalVar("g", ty.array(ty.type_name("S"), 3_u), ast::AddressSpace::kStorage,
              ast::Access::kRead, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_ArrayF16) {
    // enable f16;
    // struct S{ a : f16 };
    // type t = ptr<storage, read, array<S, 3u>>;
    Enable(ast::Extension::kF16);

    Structure("S", utils::Vector{Member("a", ty.f16())});
    Alias("t", ty.pointer(ty.array(ty.type_name("S"), 3_u), ast::AddressSpace::kStorage,
                          ast::Access::kRead));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_StructI32) {
    // struct S { x : i32 };
    // var<storage, read> g : S;
    Structure("S", utils::Vector{Member("x", ty.i32())});
    GlobalVar("g", ty.type_name("S"), ast::AddressSpace::kStorage, ast::Access::kRead, Binding(0_a),
              Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_StructI32) {
    // struct S { x : i32 };
    // type t = ptr<storage, read, S>;
    Structure("S", utils::Vector{Member("x", ty.i32())});
    Alias("t", ty.pointer(ty.type_name("S"), ast::AddressSpace::kStorage, ast::Access::kRead));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_StructI32Aliases) {
    // struct S { x : i32 };
    // type a1 = S;
    // var<storage, read> g : a1;
    Structure("S", utils::Vector{Member("x", ty.i32())});
    Alias("a1", ty.type_name("S"));
    Alias("a2", ty.type_name("a1"));
    GlobalVar("g", ty.type_name("a2"), ast::AddressSpace::kStorage, ast::Access::kRead,
              Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_StructI32Aliases) {
    // struct S { x : i32 };
    // type a1 = S;
    // type t = ptr<storage, read, a1>;
    Structure("S", utils::Vector{Member("x", ty.i32())});
    Alias("a1", ty.type_name("S"));
    Alias("a2", ty.type_name("a1"));
    Alias("t", ty.pointer(ty.type_name("a2"), ast::AddressSpace::kStorage, ast::Access::kRead));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_StructF16) {
    // struct S { x : f16 };
    // var<storage, read> g : S;
    Enable(ast::Extension::kF16);

    Structure("S", utils::Vector{Member("x", ty.f16())});
    GlobalVar("g", ty.type_name("S"), ast::AddressSpace::kStorage, ast::Access::kRead, Binding(0_a),
              Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_StructF16) {
    // struct S { x : f16 };
    // type t = ptr<storage, read, S>;
    Enable(ast::Extension::kF16);

    Structure("S", utils::Vector{Member("x", ty.f16())});
    Alias("t", ty.pointer(ty.type_name("S"), ast::AddressSpace::kStorage, ast::Access::kRead));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_StructF16Aliases) {
    // struct S { x : f16 };
    // type a1 = S;
    // var<storage, read> g : a1;
    Enable(ast::Extension::kF16);

    Structure("S", utils::Vector{Member("x", ty.f16())});
    Alias("a1", ty.type_name("S"));
    Alias("a2", ty.type_name("a1"));
    GlobalVar("g", ty.type_name("a2"), ast::AddressSpace::kStorage, ast::Access::kRead,
              Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_StructF16Aliases) {
    // struct S { x : f16 };
    // type a1 = S;
    // type t = ptr<storage, read, a1>;
    Enable(ast::Extension::kF16);

    Structure("S", utils::Vector{Member("x", ty.f16())});
    Alias("a1", ty.type_name("S"));
    Alias("a2", ty.type_name("a1"));
    Alias("g", ty.pointer(ty.type_name("a2"), ast::AddressSpace::kStorage, ast::Access::kRead));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_NotStorage_AccessMode) {
    // var<private, read> g : a;
    GlobalVar(Source{{12, 34}}, "g", ty.i32(), ast::AddressSpace::kPrivate, ast::Access::kRead);

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: only variables in <storage> address space may declare an access mode)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_NotStorage_AccessMode) {
    // type t = ptr<private, read, a>;
    Alias("t",
          ty.pointer(Source{{12, 34}}, ty.i32(), ast::AddressSpace::kPrivate, ast::Access::kRead));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: only pointers in <storage> address space may declare an access mode)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_ReadAccessMode) {
    // @group(0) @binding(0) var<storage, read> a : i32;
    GlobalVar("a", ty.i32(), ast::AddressSpace::kStorage, ast::Access::kRead, Group(0_a),
              Binding(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_ReadAccessMode) {
    // type t = ptr<storage, read, i32>;
    Alias("t", ty.pointer(ty.i32(), ast::AddressSpace::kStorage, ast::Access::kRead));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_ReadWriteAccessMode) {
    // @group(0) @binding(0) var<storage, read_write> a : i32;
    GlobalVar("a", ty.i32(), ast::AddressSpace::kStorage, ast::Access::kReadWrite, Group(0_a),
              Binding(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_ReadWriteAccessMode) {
    // type t = ptr<storage, read_write, i32>;
    Alias("t", ty.pointer(ty.i32(), ast::AddressSpace::kStorage, ast::Access::kReadWrite));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_Storage_WriteAccessMode) {
    // @group(0) @binding(0) var<storage, read_write> a : i32;
    GlobalVar(Source{{12, 34}}, "a", ty.i32(), ast::AddressSpace::kStorage, ast::Access::kWrite,
              Group(0_a), Binding(0_a));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(12:34 error: access mode 'write' is not valid for the 'storage' address space)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_Storage_WriteAccessMode) {
    // type t = ptr<storage, read_write, i32>;
    Alias("t",
          ty.pointer(Source{{12, 34}}, ty.i32(), ast::AddressSpace::kStorage, ast::Access::kWrite));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(12:34 error: access mode 'write' is not valid for the 'storage' address space)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBuffer_Struct_Runtime) {
    // struct S { m: array<f32>; };
    // @group(0) @binding(0) var<uniform> svar : S;

    Structure("S",
              utils::Vector{Member(Source{{56, 78}}, "m", ty.array(Source{{12, 34}}, ty.i32()))});

    GlobalVar(Source{{90, 12}}, "svar", ty.type_name("S"), ast::AddressSpace::kUniform,
              Binding(0_a), Group(0_a));

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: runtime-sized arrays can only be used in the <storage> address space
56:78 note: while analyzing structure member S.m
90:12 note: while instantiating 'var' svar)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBuffer_Struct_Runtime) {
    // struct S { m: array<f32>; };
    // type t = ptr<uniform, S>;

    Structure("S",
              utils::Vector{Member(Source{{56, 78}}, "m", ty.array(Source{{12, 34}}, ty.i32()))});

    Alias("t", ty.pointer(Source{{90, 12}}, ty.type_name("S"), ast::AddressSpace::kUniform));

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: uniform storage requires that array elements be aligned to 16 bytes, but array element alignment is currently 4. Consider using a vector or struct as the element type instead.
note: see layout of struct:
/*           align(4) size(4) */ struct S {
/* offset(0) align(4) size(4) */   m : array<i32>;
/*                            */ };
90:12 note: 'S' used in address space 'uniform' here)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferBool) {
    // var<uniform> g : bool;
    GlobalVar(Source{{56, 78}}, "g", ty.bool_(Source{{12, 34}}), ast::AddressSpace::kUniform,
              Binding(0_a), Group(0_a));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: Type 'bool' cannot be used in address space 'uniform' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferBool) {
    // type t = ptr<uniform, bool>;
    Alias("t",
          ty.pointer(Source{{56, 78}}, ty.bool_(Source{{12, 34}}), ast::AddressSpace::kUniform));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: Type 'bool' cannot be used in address space 'uniform' as it is non-host-shareable
56:78 note: while instantiating ptr<uniform, bool, read>)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferBoolAlias) {
    // type a = bool;
    // var<uniform> g : a;
    Alias("a", ty.bool_());
    GlobalVar(Source{{56, 78}}, "g", ty.type_name(Source{{12, 34}}, "a"),
              ast::AddressSpace::kUniform, Binding(0_a), Group(0_a));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: Type 'bool' cannot be used in address space 'uniform' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferBoolAlias) {
    // type a = bool;
    // type t = ptr<uniform, a>;
    Alias("a", ty.bool_());
    Alias("t", ty.pointer(Source{{56, 78}}, ty.type_name(Source{{12, 34}}, "a"),
                          ast::AddressSpace::kUniform));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: Type 'bool' cannot be used in address space 'uniform' as it is non-host-shareable
56:78 note: while instantiating ptr<uniform, bool, read>)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformPointer) {
    // var<uniform> g : ptr<private, f32>;
    GlobalVar(Source{{56, 78}}, "g",
              ty.pointer(Source{{12, 34}}, ty.f32(), ast::AddressSpace::kPrivate),
              ast::AddressSpace::kUniform, Binding(0_a), Group(0_a));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: Type 'ptr<private, f32, read_write>' cannot be used in address space 'uniform' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformPointer) {
    // type t = ptr<uniform, ptr<private, f32>>;
    Alias("t", ty.pointer(Source{{56, 78}},
                          ty.pointer(Source{{12, 34}}, ty.f32(), ast::AddressSpace::kPrivate),
                          ast::AddressSpace::kUniform));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: Type 'ptr<private, f32, read_write>' cannot be used in address space 'uniform' as it is non-host-shareable
56:78 note: while instantiating ptr<uniform, ptr<private, f32, read_write>, read>)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferIntScalar) {
    // var<uniform> g : i32;
    GlobalVar(Source{{56, 78}}, "g", ty.i32(), ast::AddressSpace::kUniform, Binding(0_a),
              Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferIntScalar) {
    // type t = ptr<uniform, i32>;
    Alias("t", ty.pointer(ty.i32(), ast::AddressSpace::kUniform));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferF16) {
    // enable f16;
    // var<uniform> g : f16;
    Enable(ast::Extension::kF16);

    GlobalVar("g", ty.f16(), ast::AddressSpace::kUniform, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferF16) {
    // enable f16;
    // type t = ptr<uniform, f16>;
    Enable(ast::Extension::kF16);

    Alias("t", ty.pointer(ty.f16(), ast::AddressSpace::kUniform));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferVectorF32) {
    // var<uniform> g : vec4<f32>;
    GlobalVar("g", ty.vec4<f32>(), ast::AddressSpace::kUniform, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferVectorF32) {
    // type t = ptr<uniform, vec4<f32>>;
    Alias("t", ty.pointer(ty.vec4<f32>(), ast::AddressSpace::kUniform));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferVectorF16) {
    // enable f16;
    // var<uniform> g : vec4<f16>;
    Enable(ast::Extension::kF16);

    GlobalVar("g", ty.vec4<f16>(), ast::AddressSpace::kUniform, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferVectorF16) {
    // enable f16;
    // type t = ptr<uniform, vec4<f16>>;
    Enable(ast::Extension::kF16);

    Alias("t", ty.pointer(ty.vec4<f16>(), ast::AddressSpace::kUniform));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferArrayF32) {
    // struct S {
    //   @size(16) f : f32;
    // }
    // var<uniform> g : array<S, 3u>;
    Structure("S", utils::Vector{Member("a", ty.f32(), utils::Vector{MemberSize(16_a)})});
    GlobalVar("g", ty.array(ty.type_name("S"), 3_u), ast::AddressSpace::kUniform, Binding(0_a),
              Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferArrayF32) {
    // struct S {
    //   @size(16) f : f32;
    // }
    // type t = ptr<uniform, array<S, 3u>>;
    Structure("S", utils::Vector{Member("a", ty.f32(), utils::Vector{MemberSize(16_a)})});
    Alias("t", ty.pointer(ty.array(ty.type_name("S"), 3_u), ast::AddressSpace::kUniform));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferArrayF16) {
    // enable f16;
    // struct S {
    //   @size(16) f : f16;
    // }
    // var<uniform> g : array<S, 3u>;
    Enable(ast::Extension::kF16);

    Structure("S", utils::Vector{Member("a", ty.f16(), utils::Vector{MemberSize(16_a)})});
    GlobalVar("g", ty.array(ty.type_name("S"), 3_u), ast::AddressSpace::kUniform, Binding(0_a),
              Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferArrayF16) {
    // enable f16;
    // struct S {
    //   @size(16) f : f16;
    // }
    // type t = ptr<uniform, array<S, 3u>>;
    Enable(ast::Extension::kF16);

    Structure("S", utils::Vector{Member("a", ty.f16(), utils::Vector{MemberSize(16_a)})});
    Alias("t", ty.pointer(ty.array(ty.type_name("S"), 3_u), ast::AddressSpace::kUniform));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferStructI32) {
    // struct S { x : i32 };
    // var<uniform> g : S;
    Structure("S", utils::Vector{Member("x", ty.i32())});
    GlobalVar("g", ty.type_name("S"), ast::AddressSpace::kUniform, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferStructI32) {
    // struct S { x : i32 };
    // type t = ptr<uniform, S>;
    Structure("S", utils::Vector{Member("x", ty.i32())});
    Alias("t", ty.pointer(ty.type_name("S"), ast::AddressSpace::kUniform));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferStructI32Aliases) {
    // struct S { x : i32 };
    // type a1 = S;
    // var<uniform> g : a1;
    Structure("S", utils::Vector{Member("x", ty.i32())});
    Alias("a1", ty.type_name("S"));
    GlobalVar("g", ty.type_name("a1"), ast::AddressSpace::kUniform, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferStructI32Aliases) {
    // struct S { x : i32 };
    // type a1 = S;
    // type t = ptr<uniform, a1>;
    Structure("S", utils::Vector{Member("x", ty.i32())});
    Alias("a1", ty.type_name("S"));
    Alias("t", ty.pointer(ty.type_name("a1"), ast::AddressSpace::kUniform));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferStructF16) {
    // enable f16;
    // struct S { x : f16 };
    // var<uniform> g : S;
    Enable(ast::Extension::kF16);

    Structure("S", utils::Vector{Member("x", ty.f16())});
    GlobalVar("g", ty.type_name("S"), ast::AddressSpace::kUniform, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferStructF16) {
    // enable f16;
    // struct S { x : f16 };
    // type t = ptr<uniform, S>;
    Enable(ast::Extension::kF16);

    Structure("S", utils::Vector{Member("x", ty.f16())});
    Alias("t", ty.pointer(ty.type_name("S"), ast::AddressSpace::kUniform));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_UniformBufferStructF16Aliases) {
    // enable f16;
    // struct S { x : f16 };
    // type a1 = S;
    // var<uniform> g : a1;
    Enable(ast::Extension::kF16);

    Structure("S", utils::Vector{Member("x", ty.f16())});
    Alias("a1", ty.type_name("S"));
    GlobalVar("g", ty.type_name("a1"), ast::AddressSpace::kUniform, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_UniformBufferStructF16Aliases) {
    // enable f16;
    // struct S { x : f16 };
    // type a1 = S;
    // type t = ptr<uniform, a1>;
    Enable(ast::Extension::kF16);

    Structure("S", utils::Vector{Member("x", ty.f16())});
    Alias("a1", ty.type_name("S"));
    Alias("t", ty.pointer(ty.type_name("a1"), ast::AddressSpace::kUniform));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_PushConstantBool) {
    // enable chromium_experimental_push_constant;
    // var<push_constant> g : bool;
    Enable(ast::Extension::kChromiumExperimentalPushConstant);
    GlobalVar(Source{{56, 78}}, "g", ty.bool_(Source{{12, 34}}), ast::AddressSpace::kPushConstant);

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: Type 'bool' cannot be used in address space 'push_constant' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_PushConstantBool) {
    // enable chromium_experimental_push_constant;
    // type t = ptr<push_constant, bool>;
    Enable(ast::Extension::kChromiumExperimentalPushConstant);
    Alias(Source{{56, 78}}, "t",
          ty.pointer(ty.bool_(Source{{12, 34}}), ast::AddressSpace::kPushConstant));

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: Type 'bool' cannot be used in address space 'push_constant' as it is non-host-shareable
note: while instantiating ptr<push_constant, bool, read_write>)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_PushConstantF16) {
    // enable f16;
    // enable chromium_experimental_push_constant;
    // var<push_constant> g : f16;
    Enable(ast::Extension::kF16);
    Enable(ast::Extension::kChromiumExperimentalPushConstant);
    GlobalVar("g", ty.f16(Source{{56, 78}}), ast::AddressSpace::kPushConstant);

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "error: using f16 types in 'push_constant' address space is not implemented yet");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_PushConstantF16) {
    // enable f16;
    // enable chromium_experimental_push_constant;
    // type t = ptr<push_constant, f16>;
    Enable(ast::Extension::kF16);
    Enable(ast::Extension::kChromiumExperimentalPushConstant);
    Alias("t", ty.pointer(ty.f16(Source{{56, 78}}), ast::AddressSpace::kPushConstant));

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "error: using f16 types in 'push_constant' address space is not implemented yet");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_PushConstantPointer) {
    // enable chromium_experimental_push_constant;
    // var<push_constant> g : ptr<private, f32>;
    Enable(ast::Extension::kChromiumExperimentalPushConstant);
    GlobalVar(Source{{56, 78}}, "g",
              ty.pointer(Source{{12, 34}}, ty.f32(), ast::AddressSpace::kPrivate),
              ast::AddressSpace::kPushConstant);

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: Type 'ptr<private, f32, read_write>' cannot be used in address space 'push_constant' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_PushConstantPointer) {
    // enable chromium_experimental_push_constant;
    // type t = ptr<push_constant, ptr<private, f32>>;
    Enable(ast::Extension::kChromiumExperimentalPushConstant);
    Alias(Source{{56, 78}}, "t",
          ty.pointer(ty.pointer(Source{{12, 34}}, ty.f32(), ast::AddressSpace::kPrivate),
                     ast::AddressSpace::kPushConstant));

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: Type 'ptr<private, f32, read_write>' cannot be used in address space 'push_constant' as it is non-host-shareable
note: while instantiating ptr<push_constant, ptr<private, f32, read_write>, read_write>)");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_PushConstantIntScalar) {
    // enable chromium_experimental_push_constant;
    // var<push_constant> g : i32;
    Enable(ast::Extension::kChromiumExperimentalPushConstant);
    GlobalVar("g", ty.i32(), ast::AddressSpace::kPushConstant);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_PushConstantIntScalar) {
    // enable chromium_experimental_push_constant;
    // type t = ptr<push_constant, i32>;
    Enable(ast::Extension::kChromiumExperimentalPushConstant);
    Alias("t", ty.pointer(ty.i32(), ast::AddressSpace::kPushConstant));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_PushConstantVectorF32) {
    // enable chromium_experimental_push_constant;
    // var<push_constant> g : vec4<f32>;
    Enable(ast::Extension::kChromiumExperimentalPushConstant);
    GlobalVar("g", ty.vec4<f32>(), ast::AddressSpace::kPushConstant);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_PushConstantVectorF32) {
    // enable chromium_experimental_push_constant;
    // var<push_constant> g : vec4<f32>;
    Enable(ast::Extension::kChromiumExperimentalPushConstant);
    Alias("t", ty.pointer(ty.vec4<f32>(), ast::AddressSpace::kPushConstant));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariable_PushConstantArrayF32) {
    // enable chromium_experimental_push_constant;
    // struct S { a : f32}
    // var<push_constant> g : array<S, 3u>;
    Enable(ast::Extension::kChromiumExperimentalPushConstant);
    Structure("S", utils::Vector{Member("a", ty.f32())});
    GlobalVar("g", ty.array(ty.type_name("S"), 3_u), ast::AddressSpace::kPushConstant);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PointerAlias_PushConstantArrayF32) {
    // enable chromium_experimental_push_constant;
    // struct S { a : f32}
    // type t = ptr<push_constant, array<S, 3u>>;
    Enable(ast::Extension::kChromiumExperimentalPushConstant);
    Structure("S", utils::Vector{Member("a", ty.f32())});
    Alias("t", ty.pointer(ty.array(ty.type_name("S"), 3_u), ast::AddressSpace::kPushConstant));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

}  // namespace
}  // namespace tint::resolver
