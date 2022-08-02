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

using ResolverStorageClassValidationTest = ResolverTest;

TEST_F(ResolverStorageClassValidationTest, GlobalVariableNoStorageClass_Fail) {
    // var g : f32;
    GlobalVar(Source{{12, 34}}, "g", ty.f32(), ast::StorageClass::kNone);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: module-scope 'var' declaration must have a storage class");
}

TEST_F(ResolverStorageClassValidationTest, GlobalVariableFunctionStorageClass_Fail) {
    // var<function> g : f32;
    GlobalVar(Source{{12, 34}}, "g", ty.f32(), ast::StorageClass::kFunction);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: module-scope 'var' must not use storage class 'function'");
}

TEST_F(ResolverStorageClassValidationTest, Private_RuntimeArray) {
    GlobalVar(Source{{12, 34}}, "v", ty.array(ty.i32()), ast::StorageClass::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: runtime-sized arrays can only be used in the <storage> storage class
12:34 note: while instantiating 'var' v)");
}

TEST_F(ResolverStorageClassValidationTest, Private_RuntimeArrayInStruct) {
    auto* s = Structure("S", utils::Vector{Member("m", ty.array(ty.i32()))});
    GlobalVar(Source{{12, 34}}, "v", ty.Of(s), ast::StorageClass::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: runtime-sized arrays can only be used in the <storage> storage class
note: while analysing structure member S.m
12:34 note: while instantiating 'var' v)");
}

TEST_F(ResolverStorageClassValidationTest, Workgroup_RuntimeArray) {
    GlobalVar(Source{{12, 34}}, "v", ty.array(ty.i32()), ast::StorageClass::kWorkgroup);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: runtime-sized arrays can only be used in the <storage> storage class
12:34 note: while instantiating 'var' v)");
}

TEST_F(ResolverStorageClassValidationTest, Workgroup_RuntimeArrayInStruct) {
    auto* s = Structure("S", utils::Vector{Member("m", ty.array(ty.i32()))});
    GlobalVar(Source{{12, 34}}, "v", ty.Of(s), ast::StorageClass::kWorkgroup);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: runtime-sized arrays can only be used in the <storage> storage class
note: while analysing structure member S.m
12:34 note: while instantiating 'var' v)");
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferBool) {
    // var<storage> g : bool;
    GlobalVar(Source{{56, 78}}, "g", ty.bool_(), ast::StorageClass::kStorage,
              utils::Vector{
                  create<ast::BindingAttribute>(0u),
                  create<ast::GroupAttribute>(0u),
              });

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: Type 'bool' cannot be used in storage class 'storage' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferBoolAlias) {
    // type a = bool;
    // var<storage, read> g : a;
    auto* a = Alias("a", ty.bool_());
    GlobalVar(Source{{56, 78}}, "g", ty.Of(a), ast::StorageClass::kStorage,
              utils::Vector{
                  create<ast::BindingAttribute>(0u),
                  create<ast::GroupAttribute>(0u),
              });

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: Type 'bool' cannot be used in storage class 'storage' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

// F16 types in storage and uniform buffer is not implemented yet.
// TODO(tint:1473, tint:1502): make these testcases valid after f16 is supported.
TEST_F(ResolverStorageClassValidationTest, StorageBufferF16_TemporallyBan) {
    // var<storage> g : f16;
    Enable(ast::Extension::kF16);

    GlobalVar("g", ty.f16(Source{{56, 78}}), ast::StorageClass::kStorage,
              utils::Vector{
                  create<ast::BindingAttribute>(0u),
                  create<ast::GroupAttribute>(0u),
              });

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              "56:78 error: using f16 types in 'storage' storage class is not "
              "implemented yet");
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferF16Alias_TemporallyBan) {
    // type a = f16;
    // var<storage, read> g : a;
    Enable(ast::Extension::kF16);

    auto* a = Alias("a", ty.f16());
    GlobalVar("g", ty.type_name(Source{{56, 78}}, a->name), ast::StorageClass::kStorage,
              utils::Vector{
                  create<ast::BindingAttribute>(0u),
                  create<ast::GroupAttribute>(0u),
              });

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              "56:78 error: using f16 types in 'storage' storage class is not "
              "implemented yet");
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferVectorF16_TemporallyBan) {
    // var<storage> g : vec4<f16>;
    Enable(ast::Extension::kF16);
    GlobalVar("g", ty.vec(Source{{56, 78}}, ty.Of<f16>(), 4u), ast::StorageClass::kStorage,
              utils::Vector{
                  create<ast::BindingAttribute>(0u),
                  create<ast::GroupAttribute>(0u),
              });

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              "56:78 error: using f16 types in 'storage' storage class is not "
              "implemented yet");
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferArrayF16_TemporallyBan) {
    // struct S { a : f16 };
    // var<storage, read> g : array<S, 3u>;
    Enable(ast::Extension::kF16);

    auto* s = Structure("S", utils::Vector{Member("a", ty.f16(Source{{56, 78}}))});
    auto* a = ty.array(ty.Of(s), 3_u);
    GlobalVar("g", a, ast::StorageClass::kStorage, ast::Access::kRead,
              utils::Vector{
                  create<ast::BindingAttribute>(0u),
                  create<ast::GroupAttribute>(0u),
              });

    ASSERT_FALSE(r()->Resolve());

    EXPECT_THAT(r()->error(), HasSubstr("56:78 error: using f16 types in 'storage' storage "
                                        "class is not implemented yet"));
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferStructF16_TemporallyBan) {
    // struct S { x : f16 };
    // var<storage, read> g : S;
    Enable(ast::Extension::kF16);

    auto* s = Structure("S", utils::Vector{Member("x", ty.f16(Source{{12, 34}}))});
    GlobalVar("g", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kRead,
              utils::Vector{
                  create<ast::BindingAttribute>(0u),
                  create<ast::GroupAttribute>(0u),
              });

    ASSERT_FALSE(r()->Resolve());

    EXPECT_THAT(r()->error(), HasSubstr("12:34 error: using f16 types in 'storage' storage "
                                        "class is not implemented yet"));
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferNoErrorStructF16Aliases_TemporallyBan) {
    // struct S { x : f16 };
    // type a1 = S;
    // var<storage, read> g : a1;
    Enable(ast::Extension::kF16);

    auto* s = Structure("S", utils::Vector{Member("x", ty.f16(Source{{12, 34}}))});
    auto* a1 = Alias("a1", ty.Of(s));
    auto* a2 = Alias("a2", ty.Of(a1));
    GlobalVar("g", ty.Of(a2), ast::StorageClass::kStorage, ast::Access::kRead,
              utils::Vector{
                  create<ast::BindingAttribute>(0u),
                  create<ast::GroupAttribute>(0u),
              });

    ASSERT_FALSE(r()->Resolve());

    EXPECT_THAT(r()->error(), HasSubstr("12:34 error: using f16 types in 'storage' storage "
                                        "class is not implemented yet"));
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferPointer) {
    // var<storage> g : ptr<private, f32>;
    GlobalVar(Source{{56, 78}}, "g", ty.pointer(ty.f32(), ast::StorageClass::kPrivate),
              ast::StorageClass::kStorage,
              utils::Vector{
                  create<ast::BindingAttribute>(0u),
                  create<ast::GroupAttribute>(0u),
              });

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: Type 'ptr<private, f32, read_write>' cannot be used in storage class 'storage' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferIntScalar) {
    // var<storage> g : i32;
    GlobalVar(Source{{56, 78}}, "g", ty.i32(), ast::StorageClass::kStorage,
              utils::Vector{
                  create<ast::BindingAttribute>(0u),
                  create<ast::GroupAttribute>(0u),
              });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferVectorF32) {
    // var<storage> g : vec4<f32>;
    GlobalVar(Source{{56, 78}}, "g", ty.vec4<f32>(), ast::StorageClass::kStorage,
              utils::Vector{
                  create<ast::BindingAttribute>(0u),
                  create<ast::GroupAttribute>(0u),
              });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferArrayF32) {
    // var<storage, read> g : array<S, 3u>;
    auto* s = Structure("S", utils::Vector{Member("a", ty.f32())});
    auto* a = ty.array(ty.Of(s), 3_u);
    GlobalVar(Source{{56, 78}}, "g", a, ast::StorageClass::kStorage, ast::Access::kRead,
              utils::Vector{
                  create<ast::BindingAttribute>(0u),
                  create<ast::GroupAttribute>(0u),
              });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverStorageClassValidationTest, NotStorage_AccessMode) {
    // var<private, read> g : a;
    GlobalVar(Source{{56, 78}}, "g", ty.i32(), ast::StorageClass::kPrivate, ast::Access::kRead);

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: only variables in <storage> storage class may declare an access mode)");
}

TEST_F(ResolverStorageClassValidationTest, Storage_ReadAccessMode) {
    // @group(0) @binding(0) var<storage, read> a : i32;
    GlobalVar(Source{{56, 78}}, "a", ty.i32(), ast::StorageClass::kStorage, ast::Access::kRead,
              GroupAndBinding(0, 0));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverStorageClassValidationTest, Storage_ReadWriteAccessMode) {
    // @group(0) @binding(0) var<storage, read_write> a : i32;
    GlobalVar(Source{{56, 78}}, "a", ty.i32(), ast::StorageClass::kStorage, ast::Access::kReadWrite,
              GroupAndBinding(0, 0));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverStorageClassValidationTest, Storage_WriteAccessMode) {
    // @group(0) @binding(0) var<storage, read_write> a : i32;
    GlobalVar(Source{{56, 78}}, "a", ty.i32(), ast::StorageClass::kStorage, ast::Access::kWrite,
              GroupAndBinding(0, 0));

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(56:78 error: access mode 'write' is not valid for the 'storage' address space)");
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferStructI32) {
    // struct S { x : i32 };
    // var<storage, read> g : S;
    auto* s = Structure("S", utils::Vector{Member(Source{{12, 34}}, "x", ty.i32())});
    GlobalVar(Source{{56, 78}}, "g", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kRead,
              utils::Vector{
                  create<ast::BindingAttribute>(0u),
                  create<ast::GroupAttribute>(0u),
              });

    ASSERT_TRUE(r()->Resolve());
}

TEST_F(ResolverStorageClassValidationTest, StorageBufferNoErrorStructI32Aliases) {
    // struct S { x : i32 };
    // type a1 = S;
    // var<storage, read> g : a1;
    auto* s = Structure("S", utils::Vector{Member(Source{{12, 34}}, "x", ty.i32())});
    auto* a1 = Alias("a1", ty.Of(s));
    auto* a2 = Alias("a2", ty.Of(a1));
    GlobalVar(Source{{56, 78}}, "g", ty.Of(a2), ast::StorageClass::kStorage, ast::Access::kRead,
              utils::Vector{
                  create<ast::BindingAttribute>(0u),
                  create<ast::GroupAttribute>(0u),
              });

    ASSERT_TRUE(r()->Resolve());
}

TEST_F(ResolverStorageClassValidationTest, UniformBuffer_Struct_Runtime) {
    // struct S { m:  array<f32>; };
    // @group(0) @binding(0) var<uniform, > svar : S;

    auto* s = Structure(Source{{12, 34}}, "S", utils::Vector{Member("m", ty.array<i32>())});

    GlobalVar(Source{{56, 78}}, "svar", ty.Of(s), ast::StorageClass::kUniform,
              utils::Vector{
                  create<ast::BindingAttribute>(0u),
                  create<ast::GroupAttribute>(0u),
              });

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(56:78 error: runtime-sized arrays can only be used in the <storage> storage class
note: while analysing structure member S.m
56:78 note: while instantiating 'var' svar)");
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferBool) {
    // var<uniform> g : bool;
    GlobalVar(Source{{56, 78}}, "g", ty.bool_(), ast::StorageClass::kUniform,
              utils::Vector{
                  create<ast::BindingAttribute>(0u),
                  create<ast::GroupAttribute>(0u),
              });

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: Type 'bool' cannot be used in storage class 'uniform' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferBoolAlias) {
    // type a = bool;
    // var<uniform> g : a;
    auto* a = Alias("a", ty.bool_());
    GlobalVar(Source{{56, 78}}, "g", ty.Of(a), ast::StorageClass::kUniform,
              utils::Vector{
                  create<ast::BindingAttribute>(0u),
                  create<ast::GroupAttribute>(0u),
              });

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: Type 'bool' cannot be used in storage class 'uniform' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

// F16 types in storage and uniform buffer is not implemented yet.
// TODO(tint:1473, tint:1502): make these testcases valid after f16 is supported.
TEST_F(ResolverStorageClassValidationTest, UniformBufferF16_TemporallyBan) {
    // var<uniform> g : f16;
    Enable(ast::Extension::kF16);

    GlobalVar("g", ty.f16(Source{{56, 78}}), ast::StorageClass::kUniform,
              utils::Vector{
                  create<ast::BindingAttribute>(0u),
                  create<ast::GroupAttribute>(0u),
              });

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              "56:78 error: using f16 types in 'uniform' storage class is not "
              "implemented yet");
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferF16Alias_TemporallyBan) {
    // type a = f16;
    // var<uniform> g : a;
    Enable(ast::Extension::kF16);

    auto* a = Alias("a", ty.f16());
    GlobalVar("g", ty.type_name(Source{{56, 78}}, a->name), ast::StorageClass::kUniform,
              utils::Vector{
                  create<ast::BindingAttribute>(0u),
                  create<ast::GroupAttribute>(0u),
              });

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              "56:78 error: using f16 types in 'uniform' storage class is not "
              "implemented yet");
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferVectorF16_TemporallyBan) {
    // var<uniform> g : vec4<f16>;
    Enable(ast::Extension::kF16);
    GlobalVar("g", ty.vec(Source{{56, 78}}, ty.Of<f16>(), 4u), ast::StorageClass::kUniform,
              utils::Vector{
                  create<ast::BindingAttribute>(0u),
                  create<ast::GroupAttribute>(0u),
              });

    ASSERT_FALSE(r()->Resolve());

    EXPECT_THAT(r()->error(), HasSubstr("56:78 error: using f16 types in 'uniform' storage "
                                        "class is not implemented yet"));
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferArrayF16_TemporallyBan) {
    // struct S {
    //   @size(16) f : f16;
    // }
    // var<uniform> g : array<S, 3u>;
    Enable(ast::Extension::kF16);

    auto* s = Structure(
        "S", utils::Vector{Member("a", ty.f16(Source{{56, 78}}), utils::Vector{MemberSize(16)})});
    auto* a = ty.array(ty.Of(s), 3_u);
    GlobalVar("g", a, ast::StorageClass::kUniform,
              utils::Vector{
                  create<ast::BindingAttribute>(0u),
                  create<ast::GroupAttribute>(0u),
              });

    ASSERT_FALSE(r()->Resolve());

    EXPECT_THAT(r()->error(), HasSubstr("56:78 error: using f16 types in 'uniform' storage "
                                        "class is not implemented yet"));
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferStructF16_TemporallyBan) {
    // struct S { x : f16 };
    // var<uniform> g :  S;
    Enable(ast::Extension::kF16);

    auto* s = Structure("S", utils::Vector{Member("x", ty.f16(Source{{12, 34}}))});
    GlobalVar("g", ty.Of(s), ast::StorageClass::kUniform,
              utils::Vector{
                  create<ast::BindingAttribute>(0u),
                  create<ast::GroupAttribute>(0u),
              });

    ASSERT_FALSE(r()->Resolve());

    EXPECT_THAT(r()->error(), HasSubstr("12:34 error: using f16 types in 'uniform' storage "
                                        "class is not implemented yet"));
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferStructF16Aliases_TemporallyBan) {
    // struct S { x : f16 };
    // type a1 = S;
    // var<uniform> g : a1;
    Enable(ast::Extension::kF16);

    auto* s = Structure("S", utils::Vector{Member("x", ty.f16(Source{{12, 34}}))});
    auto* a1 = Alias("a1", ty.Of(s));
    GlobalVar("g", ty.Of(a1), ast::StorageClass::kUniform,
              utils::Vector{
                  create<ast::BindingAttribute>(0u),
                  create<ast::GroupAttribute>(0u),
              });

    ASSERT_FALSE(r()->Resolve());

    EXPECT_THAT(r()->error(), HasSubstr("12:34 error: using f16 types in 'uniform' storage "
                                        "class is not implemented yet"));
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferPointer) {
    // var<uniform> g : ptr<private, f32>;
    GlobalVar(Source{{56, 78}}, "g", ty.pointer(ty.f32(), ast::StorageClass::kPrivate),
              ast::StorageClass::kUniform,
              utils::Vector{
                  create<ast::BindingAttribute>(0u),
                  create<ast::GroupAttribute>(0u),
              });

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: Type 'ptr<private, f32, read_write>' cannot be used in storage class 'uniform' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferIntScalar) {
    // var<uniform> g : i32;
    GlobalVar(Source{{56, 78}}, "g", ty.i32(), ast::StorageClass::kUniform,
              utils::Vector{
                  create<ast::BindingAttribute>(0u),
                  create<ast::GroupAttribute>(0u),
              });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferVectorF32) {
    // var<uniform> g : vec4<f32>;
    GlobalVar(Source{{56, 78}}, "g", ty.vec4<f32>(), ast::StorageClass::kUniform,
              utils::Vector{
                  create<ast::BindingAttribute>(0u),
                  create<ast::GroupAttribute>(0u),
              });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferArrayF32) {
    // struct S {
    //   @size(16) f : f32;
    // }
    // var<uniform> g : array<S, 3u>;
    auto* s = Structure("S", utils::Vector{Member("a", ty.f32(), utils::Vector{MemberSize(16)})});
    auto* a = ty.array(ty.Of(s), 3_u);
    GlobalVar(Source{{56, 78}}, "g", a, ast::StorageClass::kUniform,
              utils::Vector{
                  create<ast::BindingAttribute>(0u),
                  create<ast::GroupAttribute>(0u),
              });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferStructI32) {
    // struct S { x : i32 };
    // var<uniform> g :  S;
    auto* s = Structure("S", utils::Vector{Member(Source{{12, 34}}, "x", ty.i32())});
    GlobalVar(Source{{56, 78}}, "g", ty.Of(s), ast::StorageClass::kUniform,
              utils::Vector{
                  create<ast::BindingAttribute>(0u),
                  create<ast::GroupAttribute>(0u),
              });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverStorageClassValidationTest, UniformBufferStructI32Aliases) {
    // struct S { x : i32 };
    // type a1 = S;
    // var<uniform> g : a1;
    auto* s = Structure("S", utils::Vector{Member(Source{{12, 34}}, "x", ty.i32())});
    auto* a1 = Alias("a1", ty.Of(s));
    GlobalVar(Source{{56, 78}}, "g", ty.Of(a1), ast::StorageClass::kUniform,
              utils::Vector{
                  create<ast::BindingAttribute>(0u),
                  create<ast::GroupAttribute>(0u),
              });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverStorageClassValidationTest, PushConstantBool) {
    // enable chromium_experimental_push_constant;
    // var<push_constant> g : bool;
    Enable(ast::Extension::kChromiumExperimentalPushConstant);
    GlobalVar(Source{{56, 78}}, "g", ty.bool_(), ast::StorageClass::kPushConstant);

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: Type 'bool' cannot be used in storage class 'push_constant' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverStorageClassValidationTest, PushConstantF16) {
    // enable chromium_experimental_push_constant;
    // enable f16;
    // var<push_constant> g : f16;
    Enable(ast::Extension::kF16);
    Enable(ast::Extension::kChromiumExperimentalPushConstant);
    GlobalVar("g", ty.f16(Source{{56, 78}}), ast::StorageClass::kPushConstant);

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "56:78 error: using f16 types in 'push_constant' storage class is not "
              "implemented yet");
}

TEST_F(ResolverStorageClassValidationTest, PushConstantPointer) {
    // enable chromium_experimental_push_constant;
    // var<push_constant> g : ptr<private, f32>;
    Enable(ast::Extension::kChromiumExperimentalPushConstant);
    GlobalVar(Source{{56, 78}}, "g", ty.pointer(ty.f32(), ast::StorageClass::kPrivate),
              ast::StorageClass::kPushConstant);

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: Type 'ptr<private, f32, read_write>' cannot be used in storage class 'push_constant' as it is non-host-shareable
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverStorageClassValidationTest, PushConstantIntScalar) {
    // enable chromium_experimental_push_constant;
    // var<push_constant> g : i32;
    Enable(ast::Extension::kChromiumExperimentalPushConstant);
    GlobalVar("g", ty.i32(), ast::StorageClass::kPushConstant);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverStorageClassValidationTest, PushConstantVectorF32) {
    // enable chromium_experimental_push_constant;
    // var<push_constant> g : vec4<f32>;
    Enable(ast::Extension::kChromiumExperimentalPushConstant);
    GlobalVar("g", ty.vec4<f32>(), ast::StorageClass::kPushConstant);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverStorageClassValidationTest, PushConstantArrayF32) {
    // enable chromium_experimental_push_constant;
    // struct S { a : f32}
    // var<push_constant> g : array<S, 3u>;
    Enable(ast::Extension::kChromiumExperimentalPushConstant);
    auto* s = Structure("S", utils::Vector{Member("a", ty.f32())});
    auto* a = ty.array(ty.Of(s), 3_u);
    GlobalVar("g", a, ast::StorageClass::kPushConstant);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverStorageClassValidationTest, PushConstantWithInitializer) {
    // enable chromium_experimental_push_constant;
    // var<push_constant> a : u32 = 0u;
    Enable(ast::Extension::kChromiumExperimentalPushConstant);
    GlobalVar(Source{{1u, 2u}}, "a", ty.u32(), ast::StorageClass::kPushConstant,
              Expr(Source{{3u, 4u}}, u32(0)));

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(1:2 error: var of storage class 'push_constant' cannot have an initializer. var initializers are only supported for the storage classes 'private' and 'function')");
}

}  // namespace
}  // namespace tint::resolver
