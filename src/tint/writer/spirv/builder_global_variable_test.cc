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
#include "src/tint/type/texture_dimension.h"
#include "src/tint/writer/spirv/spv_dump.h"
#include "src/tint/writer/spirv/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::spirv {
namespace {

using BuilderTest = TestHelper;

TEST_F(BuilderTest, GlobalVar_WithAddressSpace) {
    auto* v = GlobalVar("var", ty.f32(), builtin::AddressSpace::kPrivate);

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Debug()), R"(OpName %1 "var"
)");
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
)");
}

TEST_F(BuilderTest, GlobalVar_WithInitializer) {
    auto* init = vec3<f32>(1_f, 1_f, 3_f);

    auto* v = GlobalVar("var", ty.vec3<f32>(), builtin::AddressSpace::kPrivate, init);

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.Diagnostics();
    ASSERT_FALSE(b.has_error()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Debug()), R"(OpName %6 "var"
)");
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 3
%5 = OpConstantComposite %1 %3 %3 %4
%7 = OpTypePointer Private %1
%6 = OpVariable %7 Private %5
)");
}

TEST_F(BuilderTest, GlobalConst) {
    // const c = 42;
    // var v = c;

    auto* c = GlobalConst("c", Expr(42_a));
    GlobalVar("v", builtin::AddressSpace::kPrivate, Expr(c));

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeInt 32 1
%2 = OpConstant %1 42
%4 = OpTypePointer Private %1
%3 = OpVariable %4 Private %2
%6 = OpTypeVoid
%5 = OpTypeFunction %6
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()), R"()");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, GlobalConst_Vec_Initializer) {
    // const c = vec3<f32>(1f, 2f, 3f);
    // var v = c;

    auto* c = GlobalConst("c", vec3<f32>(1_f, 2_f, 3_f));
    GlobalVar("v", builtin::AddressSpace::kPrivate, Expr(c));

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 2
%5 = OpConstant %2 3
%6 = OpConstantComposite %1 %3 %4 %5
%8 = OpTypePointer Private %1
%7 = OpVariable %8 Private %6
%10 = OpTypeVoid
%9 = OpTypeFunction %10
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()), R"()");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, GlobalConst_Vec_F16_Initializer) {
    // const c = vec3<f16>(1h, 2h, 3h);
    // var v = c;
    Enable(builtin::Extension::kF16);

    auto* c = GlobalConst("c", vec3<f16>(1_h, 2_h, 3_h));
    GlobalVar("v", builtin::AddressSpace::kPrivate, Expr(c));

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 16
%1 = OpTypeVector %2 3
%3 = OpConstant %2 0x1p+0
%4 = OpConstant %2 0x1p+1
%5 = OpConstant %2 0x1.8p+1
%6 = OpConstantComposite %1 %3 %4 %5
%8 = OpTypePointer Private %1
%7 = OpVariable %8 Private %6
%10 = OpTypeVoid
%9 = OpTypeFunction %10
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()), R"()");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, GlobalConst_Vec_AInt_Initializer) {
    // const c = vec3(1, 2, 3);
    // var v = c;

    auto* c = GlobalConst("c", Call(ty.vec3<Infer>(), 1_a, 2_a, 3_a));
    GlobalVar("v", builtin::AddressSpace::kPrivate, Expr(c));

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeInt 32 1
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 2
%5 = OpConstant %2 3
%6 = OpConstantComposite %1 %3 %4 %5
%8 = OpTypePointer Private %1
%7 = OpVariable %8 Private %6
%10 = OpTypeVoid
%9 = OpTypeFunction %10
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()), R"()");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, GlobalConst_Vec_AFloat_Initializer) {
    // const c = vec3(1.0, 2.0, 3.0);
    // var v = c;

    auto* c = GlobalConst("c", Call(ty.vec3<Infer>(), 1._a, 2._a, 3._a));
    GlobalVar("v", builtin::AddressSpace::kPrivate, Expr(c));

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 2
%5 = OpConstant %2 3
%6 = OpConstantComposite %1 %3 %4 %5
%8 = OpTypePointer Private %1
%7 = OpVariable %8 Private %6
%10 = OpTypeVoid
%9 = OpTypeFunction %10
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()), R"()");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, GlobalConst_Nested_Vec_Initializer) {
    // const c = vec3<f32>(vec2<f32>(1f, 2f), 3f));
    // var v = c;

    auto* c = GlobalConst("c", vec3<f32>(vec2<f32>(1_f, 2_f), 3_f));
    GlobalVar("v", builtin::AddressSpace::kPrivate, Expr(c));

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 2
%5 = OpConstant %2 3
%6 = OpConstantComposite %1 %3 %4 %5
%8 = OpTypePointer Private %1
%7 = OpVariable %8 Private %6
%10 = OpTypeVoid
%9 = OpTypeFunction %10
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].variables()), R"()");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, GlobalVar_WithBindingAndGroup) {
    auto* v = GlobalVar("var", ty.sampler(type::SamplerKind::kSampler), Binding(2_a), Group(3_a));

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Debug()), R"(OpName %1 "var"
)");
    EXPECT_EQ(DumpInstructions(b.Module().Annots()), R"(OpDecorate %1 Binding 2
OpDecorate %1 DescriptorSet 3
)");
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeSampler
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
)");
}

struct BuiltinData {
    builtin::BuiltinValue builtin;
    builtin::AddressSpace storage;
    SpvBuiltIn result;
};
inline std::ostream& operator<<(std::ostream& out, BuiltinData data) {
    utils::StringStream str;
    str << data.builtin;
    out << str.str();
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
    testing::Values(BuiltinData{builtin::BuiltinValue::kUndefined,
                                builtin::AddressSpace::kUndefined, SpvBuiltInMax},
                    BuiltinData{builtin::BuiltinValue::kPosition, builtin::AddressSpace::kIn,
                                SpvBuiltInFragCoord},
                    BuiltinData{builtin::BuiltinValue::kPosition, builtin::AddressSpace::kOut,
                                SpvBuiltInPosition},
                    BuiltinData{
                        builtin::BuiltinValue::kVertexIndex,
                        builtin::AddressSpace::kIn,
                        SpvBuiltInVertexIndex,
                    },
                    BuiltinData{builtin::BuiltinValue::kInstanceIndex, builtin::AddressSpace::kIn,
                                SpvBuiltInInstanceIndex},
                    BuiltinData{builtin::BuiltinValue::kFrontFacing, builtin::AddressSpace::kIn,
                                SpvBuiltInFrontFacing},
                    BuiltinData{builtin::BuiltinValue::kFragDepth, builtin::AddressSpace::kOut,
                                SpvBuiltInFragDepth},
                    BuiltinData{builtin::BuiltinValue::kLocalInvocationId,
                                builtin::AddressSpace::kIn, SpvBuiltInLocalInvocationId},
                    BuiltinData{builtin::BuiltinValue::kLocalInvocationIndex,
                                builtin::AddressSpace::kIn, SpvBuiltInLocalInvocationIndex},
                    BuiltinData{builtin::BuiltinValue::kGlobalInvocationId,
                                builtin::AddressSpace::kIn, SpvBuiltInGlobalInvocationId},
                    BuiltinData{builtin::BuiltinValue::kWorkgroupId, builtin::AddressSpace::kIn,
                                SpvBuiltInWorkgroupId},
                    BuiltinData{builtin::BuiltinValue::kNumWorkgroups, builtin::AddressSpace::kIn,
                                SpvBuiltInNumWorkgroups},
                    BuiltinData{builtin::BuiltinValue::kSampleIndex, builtin::AddressSpace::kIn,
                                SpvBuiltInSampleId},
                    BuiltinData{builtin::BuiltinValue::kSampleMask, builtin::AddressSpace::kIn,
                                SpvBuiltInSampleMask},
                    BuiltinData{builtin::BuiltinValue::kSampleMask, builtin::AddressSpace::kOut,
                                SpvBuiltInSampleMask}));

TEST_F(BuilderTest, GlobalVar_DeclReadOnly) {
    // struct A {
    //   a : i32;
    // };
    // var b<storage, read> : A

    auto* A = Structure("A", utils::Vector{
                                 Member("a", ty.i32()),
                                 Member("b", ty.i32()),
                             });

    GlobalVar("b", ty.Of(A), builtin::AddressSpace::kStorage, builtin::Access::kRead, Binding(0_a),
              Group(0_a));

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Annots()), R"(OpDecorate %3 Block
OpMemberDecorate %3 0 Offset 0
OpMemberDecorate %4 0 Offset 0
OpMemberDecorate %4 1 Offset 4
OpDecorate %1 NonWritable
OpDecorate %1 Binding 0
OpDecorate %1 DescriptorSet 0
)");
    EXPECT_EQ(DumpInstructions(b.Module().Debug()), R"(OpName %3 "b_block"
OpMemberName %3 0 "inner"
OpName %4 "A"
OpMemberName %4 0 "a"
OpMemberName %4 1 "b"
OpName %1 "b"
OpName %8 "unused_entry_point"
)");
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%5 = OpTypeInt 32 1
%4 = OpTypeStruct %5 %5
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%7 = OpTypeVoid
%6 = OpTypeFunction %7
)");
}

TEST_F(BuilderTest, GlobalVar_TypeAliasDeclReadOnly) {
    // struct A {
    //   a : i32;
    // };
    // type B = A;
    // var b<storage, read> : B

    auto* A = Structure("A", utils::Vector{Member("a", ty.i32())});
    auto* B = Alias("B", ty.Of(A));
    GlobalVar("b", ty.Of(B), builtin::AddressSpace::kStorage, builtin::Access::kRead, Binding(0_a),
              Group(0_a));

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Annots()), R"(OpDecorate %3 Block
OpMemberDecorate %3 0 Offset 0
OpMemberDecorate %4 0 Offset 0
OpDecorate %1 NonWritable
OpDecorate %1 Binding 0
OpDecorate %1 DescriptorSet 0
)");
    EXPECT_EQ(DumpInstructions(b.Module().Debug()), R"(OpName %3 "b_block"
OpMemberName %3 0 "inner"
OpName %4 "A"
OpMemberName %4 0 "a"
OpName %1 "b"
OpName %8 "unused_entry_point"
)");
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%5 = OpTypeInt 32 1
%4 = OpTypeStruct %5
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%7 = OpTypeVoid
%6 = OpTypeFunction %7
)");
}

TEST_F(BuilderTest, GlobalVar_TypeAliasAssignReadOnly) {
    // struct A {
    //   a : i32;
    // };
    // type B = A;
    // var<storage, read> b : B

    auto* A = Structure("A", utils::Vector{Member("a", ty.i32())});
    auto* B = Alias("B", ty.Of(A));
    GlobalVar("b", ty.Of(B), builtin::AddressSpace::kStorage, builtin::Access::kRead, Binding(0_a),
              Group(0_a));

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Annots()), R"(OpDecorate %3 Block
OpMemberDecorate %3 0 Offset 0
OpMemberDecorate %4 0 Offset 0
OpDecorate %1 NonWritable
OpDecorate %1 Binding 0
OpDecorate %1 DescriptorSet 0
)");
    EXPECT_EQ(DumpInstructions(b.Module().Debug()), R"(OpName %3 "b_block"
OpMemberName %3 0 "inner"
OpName %4 "A"
OpMemberName %4 0 "a"
OpName %1 "b"
OpName %8 "unused_entry_point"
)");
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%5 = OpTypeInt 32 1
%4 = OpTypeStruct %5
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%7 = OpTypeVoid
%6 = OpTypeFunction %7
)");
}

TEST_F(BuilderTest, GlobalVar_TwoVarDeclReadOnly) {
    // struct A {
    //   a : i32;
    // };
    // var<storage, read> b : A
    // var<storage, read_write> c : A

    auto* A = Structure("A", utils::Vector{Member("a", ty.i32())});
    GlobalVar("b", ty.Of(A), builtin::AddressSpace::kStorage, builtin::Access::kRead, Group(0_a),
              Binding(0_a));
    GlobalVar("c", ty.Of(A), builtin::AddressSpace::kStorage, builtin::Access::kReadWrite,
              Group(1_a), Binding(0_a));

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build());

    EXPECT_EQ(DumpInstructions(b.Module().Annots()),
              R"(OpDecorate %3 Block
OpMemberDecorate %3 0 Offset 0
OpMemberDecorate %4 0 Offset 0
OpDecorate %1 NonWritable
OpDecorate %1 DescriptorSet 0
OpDecorate %1 Binding 0
OpDecorate %6 DescriptorSet 1
OpDecorate %6 Binding 0
)");
    EXPECT_EQ(DumpInstructions(b.Module().Debug()), R"(OpName %3 "b_block"
OpMemberName %3 0 "inner"
OpName %4 "A"
OpMemberName %4 0 "a"
OpName %1 "b"
OpName %6 "c"
OpName %9 "unused_entry_point"
)");
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%5 = OpTypeInt 32 1
%4 = OpTypeStruct %5
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%6 = OpVariable %2 StorageBuffer
%8 = OpTypeVoid
%7 = OpTypeFunction %8
)");
}

TEST_F(BuilderTest, GlobalVar_TextureStorageWriteOnly) {
    // var<uniform_constant> a : texture_storage_2d<r32uint, write>;

    auto type = ty.storage_texture(type::TextureDimension::k2d, builtin::TexelFormat::kR32Uint,
                                   builtin::Access::kWrite);

    auto* var_a = GlobalVar("a", type, Binding(0_a), Group(0_a));

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.GenerateGlobalVariable(var_a)) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Annots()), R"(OpDecorate %1 NonReadable
OpDecorate %1 Binding 0
OpDecorate %1 DescriptorSet 0
)");
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeInt 32 0
%3 = OpTypeImage %4 2D 0 0 0 2 R32ui
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
)");
}

TEST_F(BuilderTest, GlobalVar_WorkgroupWithZeroInit) {
    auto type_scalar = ty.i32();
    auto* var_scalar = GlobalVar("a", type_scalar, builtin::AddressSpace::kWorkgroup);

    auto type_array = ty.array<f32, 16>();
    auto* var_array = GlobalVar("b", type_array, builtin::AddressSpace::kWorkgroup);

    auto* type_struct = Structure("C", utils::Vector{
                                           Member("a", ty.i32()),
                                           Member("b", ty.i32()),
                                       });
    auto* var_struct = GlobalVar("c", ty.Of(type_struct), builtin::AddressSpace::kWorkgroup);

    program = std::make_unique<Program>(std::move(*this));

    constexpr bool kZeroInitializeWorkgroupMemory = true;
    std::unique_ptr<spirv::Builder> b =
        std::make_unique<spirv::Builder>(program.get(), kZeroInitializeWorkgroupMemory);

    EXPECT_TRUE(b->GenerateGlobalVariable(var_scalar)) << b->Diagnostics();
    EXPECT_TRUE(b->GenerateGlobalVariable(var_array)) << b->Diagnostics();
    EXPECT_TRUE(b->GenerateGlobalVariable(var_struct)) << b->Diagnostics();
    ASSERT_FALSE(b->has_error()) << b->Diagnostics();

    EXPECT_EQ(DumpInstructions(b->Module().Types()), R"(%3 = OpTypeInt 32 1
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
