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

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariableNoAddressSpace_Fail) {
    // var g : f32;
    GlobalVar(Source{{12, 34}}, "g", ty.f32());

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: module-scope 'var' declaration must have a address space");
}

TEST_F(ResolverAddressSpaceValidationTest, GlobalVariableFunctionAddressSpace_Fail) {
    // var<function> g : f32;
    GlobalVar(Source{{12, 34}}, "g", ty.f32(), ast::AddressSpace::kFunction);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: module-scope 'var' must not use address space 'function'");
}

TEST_F(ResolverAddressSpaceValidationTest, Private_RuntimeArray) {
    GlobalVar(Source{{12, 34}}, "v", ty.array(ty.i32()), ast::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: runtime-sized arrays can only be used in the <storage> address space
12:34 note: while instantiating 'var' v)");
}

TEST_F(ResolverAddressSpaceValidationTest, Private_RuntimeArrayInStruct) {
    auto* s = Structure("S", utils::Vector{Member("m", ty.array(ty.i32()))});
    GlobalVar(Source{{12, 34}}, "v", ty.Of(s), ast::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: runtime-sized arrays can only be used in the <storage> address space
note: while analyzing structure member S.m
12:34 note: while instantiating 'var' v)");
}

TEST_F(ResolverAddressSpaceValidationTest, Workgroup_RuntimeArray) {
    GlobalVar(Source{{12, 34}}, "v", ty.array(ty.i32()), ast::AddressSpace::kWorkgroup);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: runtime-sized arrays can only be used in the <storage> address space
12:34 note: while instantiating 'var' v)");
}

TEST_F(ResolverAddressSpaceValidationTest, Workgroup_RuntimeArrayInStruct) {
    auto* s = Structure("S", utils::Vector{Member("m", ty.array(ty.i32()))});
    GlobalVar(Source{{12, 34}}, "v", ty.Of(s), ast::AddressSpace::kWorkgroup);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: runtime-sized arrays can only be used in the <storage> address space
note: while analyzing structure member S.m
12:34 note: while instantiating 'var' v)");
}

TEST_F(ResolverAddressSpaceValidationTest, StorageBufferBool) {
    // var<storage> g : bool;
    GlobalVar(Source{{56, 78}}, "g", ty.bool_(), ast::AddressSpace::kStorage, Binding(0_a),
              Group(0_a));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: Type 'bool' cannot be used in address space 'storage' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverAddressSpaceValidationTest, StorageBufferBoolAlias) {
    // type a = bool;
    // var<storage, read> g : a;
    auto* a = Alias("a", ty.bool_());
    GlobalVar(Source{{56, 78}}, "g", ty.Of(a), ast::AddressSpace::kStorage, Binding(0_a),
              Group(0_a));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: Type 'bool' cannot be used in address space 'storage' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverAddressSpaceValidationTest, StorageBufferPointer) {
    // var<storage> g : ptr<private, f32>;
    GlobalVar(Source{{56, 78}}, "g", ty.pointer(ty.f32(), ast::AddressSpace::kPrivate),
              ast::AddressSpace::kStorage, Binding(0_a), Group(0_a));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: Type 'ptr<private, f32, read_write>' cannot be used in address space 'storage' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverAddressSpaceValidationTest, StorageBufferIntScalar) {
    // var<storage> g : i32;
    GlobalVar(Source{{56, 78}}, "g", ty.i32(), ast::AddressSpace::kStorage, Binding(0_a),
              Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, StorageBufferF16) {
    // var<storage> g : f16;
    Enable(ast::Extension::kF16);

    GlobalVar("g", ty.f16(Source{{56, 78}}), ast::AddressSpace::kStorage, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, StorageBufferF16Alias) {
    // type a = f16;
    // var<storage, read> g : a;
    Enable(ast::Extension::kF16);

    auto* a = Alias("a", ty.f16());
    GlobalVar("g", ty.type_name(Source{{56, 78}}, a->name), ast::AddressSpace::kStorage,
              Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, StorageBufferVectorF32) {
    // var<storage> g : vec4<f32>;
    GlobalVar(Source{{56, 78}}, "g", ty.vec4<f32>(), ast::AddressSpace::kStorage, Binding(0_a),
              Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, StorageBufferVectorF16) {
    // var<storage> g : vec4<f16>;
    Enable(ast::Extension::kF16);
    GlobalVar("g", ty.vec(Source{{56, 78}}, ty.Of<f16>(), 4u), ast::AddressSpace::kStorage,
              Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, StorageBufferArrayF32) {
    // var<storage, read> g : array<S, 3u>;
    auto* s = Structure("S", utils::Vector{Member("a", ty.f32())});
    auto* a = ty.array(ty.Of(s), 3_u);
    GlobalVar(Source{{56, 78}}, "g", a, ast::AddressSpace::kStorage, ast::Access::kRead,
              Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, StorageBufferArrayF16) {
    // var<storage, read> g : array<S, 3u>;
    Enable(ast::Extension::kF16);

    auto* s = Structure("S", utils::Vector{Member("a", ty.f16())});
    auto* a = ty.array(ty.Of(s), 3_u);
    GlobalVar(Source{{56, 78}}, "g", a, ast::AddressSpace::kStorage, ast::Access::kRead,
              Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, StorageBufferStructI32) {
    // struct S { x : i32 };
    // var<storage, read> g : S;
    auto* s = Structure("S", utils::Vector{Member(Source{{12, 34}}, "x", ty.i32())});
    GlobalVar(Source{{56, 78}}, "g", ty.Of(s), ast::AddressSpace::kStorage, ast::Access::kRead,
              Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, StorageBufferStructI32Aliases) {
    // struct S { x : i32 };
    // type a1 = S;
    // var<storage, read> g : a1;
    auto* s = Structure("S", utils::Vector{Member(Source{{12, 34}}, "x", ty.i32())});
    auto* a1 = Alias("a1", ty.Of(s));
    auto* a2 = Alias("a2", ty.Of(a1));
    GlobalVar(Source{{56, 78}}, "g", ty.Of(a2), ast::AddressSpace::kStorage, ast::Access::kRead,
              Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, StorageBufferStructF16) {
    // struct S { x : f16 };
    // var<storage, read> g : S;
    Enable(ast::Extension::kF16);

    auto* s = Structure("S", utils::Vector{Member("x", ty.f16(Source{{12, 34}}))});
    GlobalVar("g", ty.Of(s), ast::AddressSpace::kStorage, ast::Access::kRead, Binding(0_a),
              Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, StorageBufferStructF16Aliases) {
    // struct S { x : f16 };
    // type a1 = S;
    // var<storage, read> g : a1;
    Enable(ast::Extension::kF16);

    auto* s = Structure("S", utils::Vector{Member("x", ty.f16(Source{{12, 34}}))});
    auto* a1 = Alias("a1", ty.Of(s));
    auto* a2 = Alias("a2", ty.Of(a1));
    GlobalVar("g", ty.Of(a2), ast::AddressSpace::kStorage, ast::Access::kRead, Binding(0_a),
              Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, NotStorage_AccessMode) {
    // var<private, read> g : a;
    GlobalVar(Source{{56, 78}}, "g", ty.i32(), ast::AddressSpace::kPrivate, ast::Access::kRead);

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: only variables in <storage> address space may declare an access mode)");
}

TEST_F(ResolverAddressSpaceValidationTest, Storage_ReadAccessMode) {
    // @group(0) @binding(0) var<storage, read> a : i32;
    GlobalVar(Source{{56, 78}}, "a", ty.i32(), ast::AddressSpace::kStorage, ast::Access::kRead,
              Group(0_a), Binding(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, Storage_ReadWriteAccessMode) {
    // @group(0) @binding(0) var<storage, read_write> a : i32;
    GlobalVar(Source{{56, 78}}, "a", ty.i32(), ast::AddressSpace::kStorage, ast::Access::kReadWrite,
              Group(0_a), Binding(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, Storage_WriteAccessMode) {
    // @group(0) @binding(0) var<storage, read_write> a : i32;
    GlobalVar(Source{{56, 78}}, "a", ty.i32(), ast::AddressSpace::kStorage, ast::Access::kWrite,
              Group(0_a), Binding(0_a));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(56:78 error: access mode 'write' is not valid for the 'storage' address space)");
}

TEST_F(ResolverAddressSpaceValidationTest, UniformBuffer_Struct_Runtime) {
    // struct S { m:  array<f32>; };
    // @group(0) @binding(0) var<uniform, > svar : S;

    auto* s = Structure(Source{{12, 34}}, "S", utils::Vector{Member("m", ty.array<i32>())});

    GlobalVar(Source{{56, 78}}, "svar", ty.Of(s), ast::AddressSpace::kUniform, Binding(0_a),
              Group(0_a));

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(56:78 error: runtime-sized arrays can only be used in the <storage> address space
note: while analyzing structure member S.m
56:78 note: while instantiating 'var' svar)");
}

TEST_F(ResolverAddressSpaceValidationTest, UniformBufferBool) {
    // var<uniform> g : bool;
    GlobalVar(Source{{56, 78}}, "g", ty.bool_(), ast::AddressSpace::kUniform, Binding(0_a),
              Group(0_a));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: Type 'bool' cannot be used in address space 'uniform' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverAddressSpaceValidationTest, UniformBufferBoolAlias) {
    // type a = bool;
    // var<uniform> g : a;
    auto* a = Alias("a", ty.bool_());
    GlobalVar(Source{{56, 78}}, "g", ty.Of(a), ast::AddressSpace::kUniform, Binding(0_a),
              Group(0_a));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: Type 'bool' cannot be used in address space 'uniform' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverAddressSpaceValidationTest, UniformBufferPointer) {
    // var<uniform> g : ptr<private, f32>;
    GlobalVar(Source{{56, 78}}, "g", ty.pointer(ty.f32(), ast::AddressSpace::kPrivate),
              ast::AddressSpace::kUniform, Binding(0_a), Group(0_a));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: Type 'ptr<private, f32, read_write>' cannot be used in address space 'uniform' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverAddressSpaceValidationTest, UniformBufferIntScalar) {
    // var<uniform> g : i32;
    GlobalVar(Source{{56, 78}}, "g", ty.i32(), ast::AddressSpace::kUniform, Binding(0_a),
              Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, UniformBufferF16) {
    // var<uniform> g : f16;
    Enable(ast::Extension::kF16);

    GlobalVar(Source{{56, 78}}, "g", ty.f16(), ast::AddressSpace::kUniform, Binding(0_a),
              Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, UniformBufferVectorF32) {
    // var<uniform> g : vec4<f32>;
    GlobalVar(Source{{56, 78}}, "g", ty.vec4<f32>(), ast::AddressSpace::kUniform, Binding(0_a),
              Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, UniformBufferVectorF16) {
    // var<uniform> g : vec4<f16>;
    Enable(ast::Extension::kF16);

    GlobalVar(Source{{56, 78}}, "g", ty.vec4<f16>(), ast::AddressSpace::kUniform, Binding(0_a),
              Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, UniformBufferArrayF32) {
    // struct S {
    //   @size(16) f : f32;
    // }
    // var<uniform> g : array<S, 3u>;
    auto* s = Structure("S", utils::Vector{Member("a", ty.f32(), utils::Vector{MemberSize(16_a)})});
    auto* a = ty.array(ty.Of(s), 3_u);
    GlobalVar(Source{{56, 78}}, "g", a, ast::AddressSpace::kUniform, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, UniformBufferArrayF16) {
    // struct S {
    //   @size(16) f : f16;
    // }
    // var<uniform> g : array<S, 3u>;
    Enable(ast::Extension::kF16);

    auto* s = Structure("S", utils::Vector{Member("a", ty.f16(), utils::Vector{MemberSize(16_a)})});
    auto* a = ty.array(ty.Of(s), 3_u);
    GlobalVar(Source{{56, 78}}, "g", a, ast::AddressSpace::kUniform, Binding(0_a), Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, UniformBufferStructI32) {
    // struct S { x : i32 };
    // var<uniform> g :  S;
    auto* s = Structure("S", utils::Vector{Member(Source{{12, 34}}, "x", ty.i32())});
    GlobalVar(Source{{56, 78}}, "g", ty.Of(s), ast::AddressSpace::kUniform, Binding(0_a),
              Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, UniformBufferStructI32Aliases) {
    // struct S { x : i32 };
    // type a1 = S;
    // var<uniform> g : a1;
    auto* s = Structure("S", utils::Vector{Member(Source{{12, 34}}, "x", ty.i32())});
    auto* a1 = Alias("a1", ty.Of(s));
    GlobalVar(Source{{56, 78}}, "g", ty.Of(a1), ast::AddressSpace::kUniform, Binding(0_a),
              Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, UniformBufferStructF16) {
    // struct S { x : f16 };
    // var<uniform> g :  S;
    Enable(ast::Extension::kF16);

    auto* s = Structure("S", utils::Vector{Member(Source{{12, 34}}, "x", ty.f16())});
    GlobalVar(Source{{56, 78}}, "g", ty.Of(s), ast::AddressSpace::kUniform, Binding(0_a),
              Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, UniformBufferStructF16Aliases) {
    // struct S { x : f16 };
    // type a1 = S;
    // var<uniform> g : a1;
    Enable(ast::Extension::kF16);

    auto* s = Structure("S", utils::Vector{Member(Source{{12, 34}}, "x", ty.f16())});
    auto* a1 = Alias("a1", ty.Of(s));
    GlobalVar(Source{{56, 78}}, "g", ty.Of(a1), ast::AddressSpace::kUniform, Binding(0_a),
              Group(0_a));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PushConstantBool) {
    // enable chromium_experimental_push_constant;
    // var<push_constant> g : bool;
    Enable(ast::Extension::kChromiumExperimentalPushConstant);
    GlobalVar(Source{{56, 78}}, "g", ty.bool_(), ast::AddressSpace::kPushConstant);

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: Type 'bool' cannot be used in address space 'push_constant' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverAddressSpaceValidationTest, PushConstantF16) {
    // enable chromium_experimental_push_constant;
    // enable f16;
    // var<push_constant> g : f16;
    Enable(ast::Extension::kF16);
    Enable(ast::Extension::kChromiumExperimentalPushConstant);
    GlobalVar("g", ty.f16(Source{{56, 78}}), ast::AddressSpace::kPushConstant);

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "56:78 error: using f16 types in 'push_constant' address space is not "
              "implemented yet");
}

TEST_F(ResolverAddressSpaceValidationTest, PushConstantPointer) {
    // enable chromium_experimental_push_constant;
    // var<push_constant> g : ptr<private, f32>;
    Enable(ast::Extension::kChromiumExperimentalPushConstant);
    GlobalVar(Source{{56, 78}}, "g", ty.pointer(ty.f32(), ast::AddressSpace::kPrivate),
              ast::AddressSpace::kPushConstant);

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: Type 'ptr<private, f32, read_write>' cannot be used in address space 'push_constant' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverAddressSpaceValidationTest, PushConstantIntScalar) {
    // enable chromium_experimental_push_constant;
    // var<push_constant> g : i32;
    Enable(ast::Extension::kChromiumExperimentalPushConstant);
    GlobalVar("g", ty.i32(), ast::AddressSpace::kPushConstant);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PushConstantVectorF32) {
    // enable chromium_experimental_push_constant;
    // var<push_constant> g : vec4<f32>;
    Enable(ast::Extension::kChromiumExperimentalPushConstant);
    GlobalVar("g", ty.vec4<f32>(), ast::AddressSpace::kPushConstant);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PushConstantArrayF32) {
    // enable chromium_experimental_push_constant;
    // struct S { a : f32}
    // var<push_constant> g : array<S, 3u>;
    Enable(ast::Extension::kChromiumExperimentalPushConstant);
    auto* s = Structure("S", utils::Vector{Member("a", ty.f32())});
    auto* a = ty.array(ty.Of(s), 3_u);
    GlobalVar("g", a, ast::AddressSpace::kPushConstant);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverAddressSpaceValidationTest, PushConstantWithInitializer) {
    // enable chromium_experimental_push_constant;
    // var<push_constant> a : u32 = 0u;
    Enable(ast::Extension::kChromiumExperimentalPushConstant);
    GlobalVar(Source{{1u, 2u}}, "a", ty.u32(), ast::AddressSpace::kPushConstant,
              Expr(Source{{3u, 4u}}, u32(0)));

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(1:2 error: var of address space 'push_constant' cannot have an initializer. var initializers are only supported for the address spacees 'private' and 'function')");
}

}  // namespace
}  // namespace tint::resolver
