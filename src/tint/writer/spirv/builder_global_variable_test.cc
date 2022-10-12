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

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::spirv {
namespace {

using BuilderTest = TestHelper;

TEST_F(BuilderTest, GlobalVar_WithAddressSpace) {
    auto* v = GlobalVar("var", ty.f32(), ast::AddressSpace::kPrivate);

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
    auto* init = vec3<f32>(1_f, 1_f, 3_f);

    auto* v = GlobalVar("var", ty.vec3<f32>(), ast::AddressSpace::kPrivate, init);

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

TEST_F(BuilderTest, GlobalConst) {
    // const c = 42;
    // var v = c;

    auto* c = GlobalConst("c", Expr(42_a));
    GlobalVar("v", ast::AddressSpace::kPrivate, Expr(c));

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeInt 32 1
%2 = OpConstant %1 42
%4 = OpTypePointer Private %1
%3 = OpVariable %4 Private %2
%6 = OpTypeVoid
%5 = OpTypeFunction %6
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].variables()), R"()");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), R"(OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, GlobalConst_Vec_Constructor) {
    // const c = vec3<f32>(1f, 2f, 3f);
    // var v = c;

    auto* c = GlobalConst("c", vec3<f32>(1_f, 2_f, 3_f));
    GlobalVar("v", ast::AddressSpace::kPrivate, Expr(c));

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
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
    EXPECT_EQ(DumpInstructions(b.functions()[0].variables()), R"()");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), R"(OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, GlobalConst_Vec_F16_Constructor) {
    // const c = vec3<f16>(1h, 2h, 3h);
    // var v = c;
    Enable(ast::Extension::kF16);

    auto* c = GlobalConst("c", vec3<f16>(1_h, 2_h, 3_h));
    GlobalVar("v", ast::AddressSpace::kPrivate, Expr(c));

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 16
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
    EXPECT_EQ(DumpInstructions(b.functions()[0].variables()), R"()");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), R"(OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, GlobalConst_Vec_AInt_Constructor) {
    // const c = vec3(1, 2, 3);
    // var v = c;

    auto* c = GlobalConst("c", Construct(ty.vec3(nullptr), 1_a, 2_a, 3_a));
    GlobalVar("v", ast::AddressSpace::kPrivate, Expr(c));

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeInt 32 1
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
    EXPECT_EQ(DumpInstructions(b.functions()[0].variables()), R"()");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), R"(OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, GlobalConst_Vec_AFloat_Constructor) {
    // const c = vec3(1.0, 2.0, 3.0);
    // var v = c;

    auto* c = GlobalConst("c", Construct(ty.vec3(nullptr), 1._a, 2._a, 3._a));
    GlobalVar("v", ast::AddressSpace::kPrivate, Expr(c));

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
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
    EXPECT_EQ(DumpInstructions(b.functions()[0].variables()), R"()");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), R"(OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, GlobalConst_Nested_Vec_Constructor) {
    // const c = vec3<f32>(vec2<f32>(1f, 2f), 3f));
    // var v = c;

    auto* c = GlobalConst("c", vec3<f32>(vec2<f32>(1_f, 2_f), 3_f));
    GlobalVar("v", ast::AddressSpace::kPrivate, Expr(c));

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
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
    EXPECT_EQ(DumpInstructions(b.functions()[0].variables()), R"()");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), R"(OpReturn
)");

    Validate(b);
}

TEST_F(BuilderTest, GlobalVar_WithBindingAndGroup) {
    auto* v = GlobalVar("var", ty.sampler(ast::SamplerKind::kSampler), Binding(2_a), Group(3_a));

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

struct BuiltinData {
    ast::BuiltinValue builtin;
    ast::AddressSpace storage;
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
        BuiltinData{ast::BuiltinValue::kUndefined, ast::AddressSpace::kNone, SpvBuiltInMax},
        BuiltinData{ast::BuiltinValue::kPosition, ast::AddressSpace::kIn, SpvBuiltInFragCoord},
        BuiltinData{ast::BuiltinValue::kPosition, ast::AddressSpace::kOut, SpvBuiltInPosition},
        BuiltinData{
            ast::BuiltinValue::kVertexIndex,
            ast::AddressSpace::kIn,
            SpvBuiltInVertexIndex,
        },
        BuiltinData{ast::BuiltinValue::kInstanceIndex, ast::AddressSpace::kIn,
                    SpvBuiltInInstanceIndex},
        BuiltinData{ast::BuiltinValue::kFrontFacing, ast::AddressSpace::kIn, SpvBuiltInFrontFacing},
        BuiltinData{ast::BuiltinValue::kFragDepth, ast::AddressSpace::kOut, SpvBuiltInFragDepth},
        BuiltinData{ast::BuiltinValue::kLocalInvocationId, ast::AddressSpace::kIn,
                    SpvBuiltInLocalInvocationId},
        BuiltinData{ast::BuiltinValue::kLocalInvocationIndex, ast::AddressSpace::kIn,
                    SpvBuiltInLocalInvocationIndex},
        BuiltinData{ast::BuiltinValue::kGlobalInvocationId, ast::AddressSpace::kIn,
                    SpvBuiltInGlobalInvocationId},
        BuiltinData{ast::BuiltinValue::kWorkgroupId, ast::AddressSpace::kIn, SpvBuiltInWorkgroupId},
        BuiltinData{ast::BuiltinValue::kNumWorkgroups, ast::AddressSpace::kIn,
                    SpvBuiltInNumWorkgroups},
        BuiltinData{ast::BuiltinValue::kSampleIndex, ast::AddressSpace::kIn, SpvBuiltInSampleId},
        BuiltinData{ast::BuiltinValue::kSampleMask, ast::AddressSpace::kIn, SpvBuiltInSampleMask},
        BuiltinData{ast::BuiltinValue::kSampleMask, ast::AddressSpace::kOut,
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

    GlobalVar("b", ty.Of(A), ast::AddressSpace::kStorage, ast::Access::kRead, Binding(0_a),
              Group(0_a));

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

    auto* A = Structure("A", utils::Vector{Member("a", ty.i32())});
    auto* B = Alias("B", ty.Of(A));
    GlobalVar("b", ty.Of(B), ast::AddressSpace::kStorage, ast::Access::kRead, Binding(0_a),
              Group(0_a));

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

    auto* A = Structure("A", utils::Vector{Member("a", ty.i32())});
    auto* B = Alias("B", ty.Of(A));
    GlobalVar("b", ty.Of(B), ast::AddressSpace::kStorage, ast::Access::kRead, Binding(0_a),
              Group(0_a));

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

    auto* A = Structure("A", utils::Vector{Member("a", ty.i32())});
    GlobalVar("b", ty.Of(A), ast::AddressSpace::kStorage, ast::Access::kRead, Group(0_a),
              Binding(0_a));
    GlobalVar("c", ty.Of(A), ast::AddressSpace::kStorage, ast::Access::kReadWrite, Group(1_a),
              Binding(0_a));

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

    auto* var_a = GlobalVar("a", type, Binding(0_a), Group(0_a));

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

TEST_F(BuilderTest, GlobalVar_WorkgroupWithZeroInit) {
    auto* type_scalar = ty.i32();
    auto* var_scalar = GlobalVar("a", type_scalar, ast::AddressSpace::kWorkgroup);

    auto* type_array = ty.array<f32, 16>();
    auto* var_array = GlobalVar("b", type_array, ast::AddressSpace::kWorkgroup);

    auto* type_struct = Structure("C", utils::Vector{
                                           Member("a", ty.i32()),
                                           Member("b", ty.i32()),
                                       });
    auto* var_struct = GlobalVar("c", ty.Of(type_struct), ast::AddressSpace::kWorkgroup);

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
