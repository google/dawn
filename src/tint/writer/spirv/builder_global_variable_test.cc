// Copyright 2020 The Tint Authors.
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

#include "src/tint/ast/id_attribute.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/writer/spirv/spv_dump.h"
#include "src/tint/writer/spirv/test_helper.h"

namespace tint::writer::spirv {
namespace {

using BuilderTest = TestHelper;

TEST_F(BuilderTest, GlobalVar_WithStorageClass) {
    auto* v = Global("var", ty.f32(), ast::StorageClass::kPrivate);

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
    EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "var"
)");
    EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
)");
}

TEST_F(BuilderTest, GlobalVar_WithConstructor) {
    auto* init = vec3<f32>(1.f, 1.f, 3.f);

    auto* v = Global("var", ty.vec3<f32>(), ast::StorageClass::kPrivate, init);

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
    ASSERT_FALSE(b.has_error()) << b.error();

    EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %6 "var"
)");
    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 3
%5 = OpConstantComposite %1 %3 %3 %4
%7 = OpTypePointer Private %1
%6 = OpVariable %7 Private %5
)");
}

TEST_F(BuilderTest, GlobalVar_Const) {
    auto* init = vec3<f32>(1.f, 1.f, 3.f);

    auto* v = GlobalConst("var", ty.vec3<f32>(), init);

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
    ASSERT_FALSE(b.has_error()) << b.error();

    EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %5 "var"
)");
    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 3
%5 = OpConstantComposite %1 %3 %3 %4
)");
}

TEST_F(BuilderTest, GlobalVar_Complex_Constructor) {
    auto* init = vec3<f32>(ast::ExpressionList{Expr(1.f), Expr(2.f), Expr(3.f)});

    auto* v = GlobalConst("var", ty.vec3<f32>(), init);

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
    ASSERT_FALSE(b.has_error()) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 2
%5 = OpConstant %2 3
%6 = OpConstantComposite %1 %3 %4 %5
)");
}

TEST_F(BuilderTest, GlobalVar_Complex_ConstructorWithExtract) {
    auto* init = vec3<f32>(vec2<f32>(1.f, 2.f), 3.f);

    auto* v = GlobalConst("var", ty.vec3<f32>(), init);

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
    ASSERT_FALSE(b.has_error()) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpTypeVector %2 2
%4 = OpConstant %2 1
%5 = OpConstant %2 2
%6 = OpConstantComposite %3 %4 %5
%8 = OpTypeInt 32 0
%9 = OpConstant %8 0
%7 = OpSpecConstantOp %2 CompositeExtract %6 9
%11 = OpConstant %8 1
%10 = OpSpecConstantOp %2 CompositeExtract %6 11
%12 = OpConstant %2 3
%13 = OpSpecConstantComposite %1 %7 %10 %12
)");
}

TEST_F(BuilderTest, GlobalVar_WithBindingAndGroup) {
    auto* v =
        Global("var", ty.sampler(ast::SamplerKind::kSampler), ast::StorageClass::kNone, nullptr,
               ast::AttributeList{
                   create<ast::BindingAttribute>(2),
                   create<ast::GroupAttribute>(3),
               });

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
    EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "var"
)");
    EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %1 Binding 2
OpDecorate %1 DescriptorSet 3
)");
    EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeSampler
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
)");
}

TEST_F(BuilderTest, GlobalVar_Override_Bool) {
    auto* v = Override("var", ty.bool_(), Expr(true),
                       ast::AttributeList{
                           Id(1200),
                       });

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
    EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %2 "var"
)");
    EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %2 SpecId 1200
)");
    EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeBool
%2 = OpSpecConstantTrue %1
)");
}

TEST_F(BuilderTest, GlobalVar_Override_Bool_ZeroValue) {
    auto* v = Override("var", ty.bool_(), Construct<bool>(),
                       ast::AttributeList{
                           Id(1200),
                       });

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
    EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %2 "var"
)");
    EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %2 SpecId 1200
)");
    EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeBool
%2 = OpSpecConstantFalse %1
)");
}

TEST_F(BuilderTest, GlobalVar_Override_Bool_NoConstructor) {
    auto* v = Override("var", ty.bool_(), nullptr,
                       ast::AttributeList{
                           Id(1200),
                       });

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
    EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %2 "var"
)");
    EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %2 SpecId 1200
)");
    EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeBool
%2 = OpSpecConstantFalse %1
)");
}

TEST_F(BuilderTest, GlobalVar_Override_Scalar) {
    auto* v = Override("var", ty.f32(), Expr(2.f),
                       ast::AttributeList{
                           Id(0),
                       });

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
    EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %2 "var"
)");
    EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %2 SpecId 0
)");
    EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeFloat 32
%2 = OpSpecConstant %1 2
)");
}

TEST_F(BuilderTest, GlobalVar_Override_Scalar_ZeroValue) {
    auto* v = Override("var", ty.f32(), Construct<f32>(),
                       ast::AttributeList{
                           Id(0),
                       });

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
    EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %2 "var"
)");
    EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %2 SpecId 0
)");
    EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeFloat 32
%2 = OpSpecConstant %1 0
)");
}

TEST_F(BuilderTest, GlobalVar_Override_Scalar_F32_NoConstructor) {
    auto* v = Override("var", ty.f32(), nullptr,
                       ast::AttributeList{
                           Id(0),
                       });

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
    EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %2 "var"
)");
    EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %2 SpecId 0
)");
    EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeFloat 32
%2 = OpSpecConstant %1 0
)");
}

TEST_F(BuilderTest, GlobalVar_Override_Scalar_I32_NoConstructor) {
    auto* v = Override("var", ty.i32(), nullptr,
                       ast::AttributeList{
                           Id(0),
                       });

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
    EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %2 "var"
)");
    EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %2 SpecId 0
)");
    EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeInt 32 1
%2 = OpSpecConstant %1 0
)");
}

TEST_F(BuilderTest, GlobalVar_Override_Scalar_U32_NoConstructor) {
    auto* v = Override("var", ty.u32(), nullptr,
                       ast::AttributeList{
                           Id(0),
                       });

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
    EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %2 "var"
)");
    EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %2 SpecId 0
)");
    EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeInt 32 0
%2 = OpSpecConstant %1 0
)");
}

TEST_F(BuilderTest, GlobalVar_Override_NoId) {
    auto* var_a = Override("a", ty.bool_(), Expr(true),
                           ast::AttributeList{
                               Id(0),
                           });
    auto* var_b = Override("b", ty.bool_(), Expr(false));

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.GenerateGlobalVariable(var_a)) << b.error();
    EXPECT_TRUE(b.GenerateGlobalVariable(var_b)) << b.error();
    EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %2 "a"
OpName %3 "b"
)");
    EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %2 SpecId 0
OpDecorate %3 SpecId 1
)");
    EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeBool
%2 = OpSpecConstantTrue %1
%3 = OpSpecConstantFalse %1
)");
}

struct BuiltinData {
    ast::Builtin builtin;
    ast::StorageClass storage;
    SpvBuiltIn result;
};
inline std::ostream& operator<<(std::ostream& out, BuiltinData data) {
    out << data.builtin;
    return out;
}
using BuiltinDataTest = TestParamHelper<BuiltinData>;
TEST_P(BuiltinDataTest, Convert) {
    auto params = GetParam();

    spirv::Builder& b = Build();

    EXPECT_EQ(b.ConvertBuiltin(params.builtin, params.storage), params.result);
}
INSTANTIATE_TEST_SUITE_P(
    BuilderTest_Type,
    BuiltinDataTest,
    testing::Values(
        BuiltinData{ast::Builtin::kNone, ast::StorageClass::kNone, SpvBuiltInMax},
        BuiltinData{ast::Builtin::kPosition, ast::StorageClass::kInput, SpvBuiltInFragCoord},
        BuiltinData{ast::Builtin::kPosition, ast::StorageClass::kOutput, SpvBuiltInPosition},
        BuiltinData{
            ast::Builtin::kVertexIndex,
            ast::StorageClass::kInput,
            SpvBuiltInVertexIndex,
        },
        BuiltinData{ast::Builtin::kInstanceIndex, ast::StorageClass::kInput,
                    SpvBuiltInInstanceIndex},
        BuiltinData{ast::Builtin::kFrontFacing, ast::StorageClass::kInput, SpvBuiltInFrontFacing},
        BuiltinData{ast::Builtin::kFragDepth, ast::StorageClass::kOutput, SpvBuiltInFragDepth},
        BuiltinData{ast::Builtin::kLocalInvocationId, ast::StorageClass::kInput,
                    SpvBuiltInLocalInvocationId},
        BuiltinData{ast::Builtin::kLocalInvocationIndex, ast::StorageClass::kInput,
                    SpvBuiltInLocalInvocationIndex},
        BuiltinData{ast::Builtin::kGlobalInvocationId, ast::StorageClass::kInput,
                    SpvBuiltInGlobalInvocationId},
        BuiltinData{ast::Builtin::kWorkgroupId, ast::StorageClass::kInput, SpvBuiltInWorkgroupId},
        BuiltinData{ast::Builtin::kNumWorkgroups, ast::StorageClass::kInput,
                    SpvBuiltInNumWorkgroups},
        BuiltinData{ast::Builtin::kSampleIndex, ast::StorageClass::kInput, SpvBuiltInSampleId},
        BuiltinData{ast::Builtin::kSampleMask, ast::StorageClass::kInput, SpvBuiltInSampleMask},
        BuiltinData{ast::Builtin::kSampleMask, ast::StorageClass::kOutput, SpvBuiltInSampleMask}));

TEST_F(BuilderTest, GlobalVar_DeclReadOnly) {
    // struct A {
    //   a : i32;
    // };
    // var b<storage, read> : A

    auto* A = Structure("A", {
                                 Member("a", ty.i32()),
                                 Member("b", ty.i32()),
                             });

    Global("b", ty.Of(A), ast::StorageClass::kStorage, ast::Access::kRead,
           ast::AttributeList{
               create<ast::BindingAttribute>(0),
               create<ast::GroupAttribute>(0),
           });

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %3 Block
OpMemberDecorate %3 0 Offset 0
OpMemberDecorate %3 1 Offset 4
OpDecorate %1 NonWritable
OpDecorate %1 Binding 0
OpDecorate %1 DescriptorSet 0
)");
    EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %3 "A"
OpMemberName %3 0 "a"
OpMemberName %3 1 "b"
OpName %1 "b"
OpName %7 "unused_entry_point"
)");
    EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 1
%3 = OpTypeStruct %4 %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%6 = OpTypeVoid
%5 = OpTypeFunction %6
)");
}

TEST_F(BuilderTest, GlobalVar_TypeAliasDeclReadOnly) {
    // struct A {
    //   a : i32;
    // };
    // type B = A;
    // var b<storage, read> : B

    auto* A = Structure("A", {Member("a", ty.i32())});
    auto* B = Alias("B", ty.Of(A));
    Global("b", ty.Of(B), ast::StorageClass::kStorage, ast::Access::kRead,
           ast::AttributeList{
               create<ast::BindingAttribute>(0),
               create<ast::GroupAttribute>(0),
           });

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %3 Block
OpMemberDecorate %3 0 Offset 0
OpDecorate %1 NonWritable
OpDecorate %1 Binding 0
OpDecorate %1 DescriptorSet 0
)");
    EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %3 "A"
OpMemberName %3 0 "a"
OpName %1 "b"
OpName %7 "unused_entry_point"
)");
    EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 1
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%6 = OpTypeVoid
%5 = OpTypeFunction %6
)");
}

TEST_F(BuilderTest, GlobalVar_TypeAliasAssignReadOnly) {
    // struct A {
    //   a : i32;
    // };
    // type B = A;
    // var<storage, read> b : B

    auto* A = Structure("A", {Member("a", ty.i32())});
    auto* B = Alias("B", ty.Of(A));
    Global("b", ty.Of(B), ast::StorageClass::kStorage, ast::Access::kRead,
           ast::AttributeList{
               create<ast::BindingAttribute>(0),
               create<ast::GroupAttribute>(0),
           });

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %3 Block
OpMemberDecorate %3 0 Offset 0
OpDecorate %1 NonWritable
OpDecorate %1 Binding 0
OpDecorate %1 DescriptorSet 0
)");
    EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %3 "A"
OpMemberName %3 0 "a"
OpName %1 "b"
OpName %7 "unused_entry_point"
)");
    EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 1
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%6 = OpTypeVoid
%5 = OpTypeFunction %6
)");
}

TEST_F(BuilderTest, GlobalVar_TwoVarDeclReadOnly) {
    // struct A {
    //   a : i32;
    // };
    // var<storage, read> b : A
    // var<storage, read_write> c : A

    auto* A = Structure("A", {Member("a", ty.i32())});
    Global("b", ty.Of(A), ast::StorageClass::kStorage, ast::Access::kRead,
           ast::AttributeList{
               create<ast::GroupAttribute>(0),
               create<ast::BindingAttribute>(0),
           });
    Global("c", ty.Of(A), ast::StorageClass::kStorage, ast::Access::kReadWrite,
           ast::AttributeList{
               create<ast::GroupAttribute>(1),
               create<ast::BindingAttribute>(0),
           });

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.annots()),
              R"(OpDecorate %3 Block
OpMemberDecorate %3 0 Offset 0
OpDecorate %1 NonWritable
OpDecorate %1 DescriptorSet 0
OpDecorate %1 Binding 0
OpDecorate %5 DescriptorSet 1
OpDecorate %5 Binding 0
)");
    EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %3 "A"
OpMemberName %3 0 "a"
OpName %1 "b"
OpName %5 "c"
OpName %8 "unused_entry_point"
)");
    EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 1
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%5 = OpVariable %2 StorageBuffer
%7 = OpTypeVoid
%6 = OpTypeFunction %7
)");
}

TEST_F(BuilderTest, GlobalVar_TextureStorageWriteOnly) {
    // var<uniform_constant> a : texture_storage_2d<r32uint, write>;

    auto* type = ty.storage_texture(ast::TextureDimension::k2d, ast::TexelFormat::kR32Uint,
                                    ast::Access::kWrite);

    auto* var_a = Global("a", type,
                         ast::AttributeList{
                             create<ast::BindingAttribute>(0),
                             create<ast::GroupAttribute>(0),
                         });

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.GenerateGlobalVariable(var_a)) << b.error();

    EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %1 NonReadable
OpDecorate %1 Binding 0
OpDecorate %1 DescriptorSet 0
)");
    EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 0
%3 = OpTypeImage %4 2D 0 0 0 2 R32ui
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
)");
}

// Check that multiple texture_storage types with different access modifiers
// only produces a single OpTypeImage.
// Test disabled as storage textures currently only support 'write' access. In
// the future we'll likely support read_write.
TEST_F(BuilderTest, DISABLED_GlobalVar_TextureStorageWithDifferentAccess) {
    // var<uniform_constant> a : texture_storage_2d<r32uint, read_write>;
    // var<uniform_constant> b : texture_storage_2d<r32uint, write>;

    auto* type_a = ty.storage_texture(ast::TextureDimension::k2d, ast::TexelFormat::kR32Uint,
                                      ast::Access::kReadWrite);
    auto* var_a = Global("a", type_a, ast::StorageClass::kNone,
                         ast::AttributeList{
                             create<ast::BindingAttribute>(0),
                             create<ast::GroupAttribute>(0),
                         });

    auto* type_b = ty.storage_texture(ast::TextureDimension::k2d, ast::TexelFormat::kR32Uint,
                                      ast::Access::kWrite);
    auto* var_b = Global("b", type_b, ast::StorageClass::kNone,
                         ast::AttributeList{
                             create<ast::BindingAttribute>(1),
                             create<ast::GroupAttribute>(0),
                         });

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.GenerateGlobalVariable(var_a)) << b.error();
    EXPECT_TRUE(b.GenerateGlobalVariable(var_b)) << b.error();

    EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %1 NonWritable
OpDecorate %1 Binding 0
OpDecorate %1 DescriptorSet 0
OpDecorate %5 NonReadable
OpDecorate %5 Binding 1
OpDecorate %5 DescriptorSet 0
)");
    // There must only be one OpTypeImage declaration with the same
    // arguments
    EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 0
%3 = OpTypeImage %4 2D 0 0 0 2 R32ui
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%6 = OpTypePointer UniformConstant %3
%5 = OpVariable %6 UniformConstant
)");
}

TEST_F(BuilderTest, GlobalVar_WorkgroupWithZeroInit) {
    auto* type_scalar = ty.i32();
    auto* var_scalar = Global("a", type_scalar, ast::StorageClass::kWorkgroup);

    auto* type_array = ty.array<f32, 16>();
    auto* var_array = Global("b", type_array, ast::StorageClass::kWorkgroup);

    auto* type_struct = Structure("C", {
                                           Member("a", ty.i32()),
                                           Member("b", ty.i32()),
                                       });
    auto* var_struct = Global("c", ty.Of(type_struct), ast::StorageClass::kWorkgroup);

    program = std::make_unique<Program>(std::move(*this));

    constexpr bool kZeroInitializeWorkgroupMemory = true;
    std::unique_ptr<spirv::Builder> b =
        std::make_unique<spirv::Builder>(program.get(), kZeroInitializeWorkgroupMemory);

    EXPECT_TRUE(b->GenerateGlobalVariable(var_scalar)) << b->error();
    EXPECT_TRUE(b->GenerateGlobalVariable(var_array)) << b->error();
    EXPECT_TRUE(b->GenerateGlobalVariable(var_struct)) << b->error();
    ASSERT_FALSE(b->has_error()) << b->error();

    EXPECT_EQ(DumpInstructions(b->types()), R"(%3 = OpTypeInt 32 1
%2 = OpTypePointer Workgroup %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Workgroup %4
%8 = OpTypeFloat 32
%9 = OpTypeInt 32 0
%10 = OpConstant %9 16
%7 = OpTypeArray %8 %10
%6 = OpTypePointer Workgroup %7
%11 = OpConstantNull %7
%5 = OpVariable %6 Workgroup %11
%14 = OpTypeStruct %3 %3
%13 = OpTypePointer Workgroup %14
%15 = OpConstantNull %14
%12 = OpVariable %13 Workgroup %15
)");
}

}  // namespace
}  // namespace tint::writer::spirv
