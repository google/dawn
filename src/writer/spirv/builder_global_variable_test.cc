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

#include "src/ast/override_decoration.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/struct_block_decoration.h"
#include "src/writer/spirv/spv_dump.h"
#include "src/writer/spirv/test_helper.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = TestHelper;

TEST_F(BuilderTest, GlobalVar_WithStorageClass) {
  auto* v = Global("var", ty.f32(), ast::StorageClass::kOutput);

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "var"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Output %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Output %4
)");
}

TEST_F(BuilderTest, GlobalVar_WithStorageClass_Input) {
  auto* v = Global("var", ty.f32(), ast::StorageClass::kInput);

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "var"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Input %3
%1 = OpVariable %2 Input
)");
}

TEST_F(BuilderTest, GlobalVar_WithConstructor) {
  auto* init = vec3<f32>(1.f, 1.f, 3.f);

  auto* v = Global("var", ty.vec3<f32>(), ast::StorageClass::kOutput, init);

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
%7 = OpTypePointer Output %1
%6 = OpVariable %7 Output %5
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

TEST_F(BuilderTest, GlobalVar_WithLocation) {
  auto* v = Global("var", ty.f32(), ast::StorageClass::kOutput, nullptr,
                   ast::DecorationList{
                       Location(5),
                   });

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "var"
)");
  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %1 Location 5
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Output %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Output %4
)");
}

TEST_F(BuilderTest, GlobalVar_WithBindingAndGroup) {
  auto* v = Global("var", ty.sampler(ast::SamplerKind::kSampler),
                   ast::StorageClass::kNone, nullptr,
                   ast::DecorationList{
                       create<ast::BindingDecoration>(2),
                       create<ast::GroupDecoration>(3),
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

TEST_F(BuilderTest, GlobalVar_WithBuiltin) {
  auto* v = Global("var", ty.f32(), ast::StorageClass::kOutput, nullptr,
                   ast::DecorationList{
                       Builtin(ast::Builtin::kPosition),
                   });

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "var"
)");
  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %1 BuiltIn Position
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Output %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Output %4
)");
}

TEST_F(BuilderTest, GlobalVar_Override_Bool) {
  auto* v = GlobalConst("var", ty.bool_(), Expr(true),
                        ast::DecorationList{
                            create<ast::OverrideDecoration>(1200),
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

TEST_F(BuilderTest, GlobalVar_Override_Bool_NoConstructor) {
  auto* v = GlobalConst("var", ty.bool_(), nullptr,
                        ast::DecorationList{
                            create<ast::OverrideDecoration>(1200),
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
  auto* v = GlobalConst("var", ty.f32(), Expr(2.f),
                        ast::DecorationList{
                            create<ast::OverrideDecoration>(0),
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

TEST_F(BuilderTest, GlobalVar_Override_Scalar_F32_NoConstructor) {
  auto* v = GlobalConst("var", ty.f32(), nullptr,
                        ast::DecorationList{
                            create<ast::OverrideDecoration>(0),
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
  auto* v = GlobalConst("var", ty.i32(), nullptr,
                        ast::DecorationList{
                            create<ast::OverrideDecoration>(0),
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
  auto* v = GlobalConst("var", ty.u32(), nullptr,
                        ast::DecorationList{
                            create<ast::OverrideDecoration>(0),
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
  auto* var_a = GlobalConst("a", ty.bool_(), Expr(true),
                            ast::DecorationList{
                                create<ast::OverrideDecoration>(0),
                            });
  auto* var_b = GlobalConst("b", ty.bool_(), Expr(false),
                            ast::DecorationList{
                                create<ast::OverrideDecoration>(),
                            });

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
        BuiltinData{ast::Builtin::kNone, ast::StorageClass::kNone,
                    SpvBuiltInMax},
        BuiltinData{ast::Builtin::kPosition, ast::StorageClass::kInput,
                    SpvBuiltInFragCoord},
        BuiltinData{ast::Builtin::kPosition, ast::StorageClass::kOutput,
                    SpvBuiltInPosition},
        BuiltinData{
            ast::Builtin::kVertexIndex,
            ast::StorageClass::kInput,
            SpvBuiltInVertexIndex,
        },
        BuiltinData{ast::Builtin::kInstanceIndex, ast::StorageClass::kInput,
                    SpvBuiltInInstanceIndex},
        BuiltinData{ast::Builtin::kFrontFacing, ast::StorageClass::kInput,
                    SpvBuiltInFrontFacing},
        BuiltinData{ast::Builtin::kFragCoord, ast::StorageClass::kInput,
                    SpvBuiltInFragCoord},
        BuiltinData{ast::Builtin::kFragDepth, ast::StorageClass::kOutput,
                    SpvBuiltInFragDepth},
        BuiltinData{ast::Builtin::kLocalInvocationId, ast::StorageClass::kInput,
                    SpvBuiltInLocalInvocationId},
        BuiltinData{ast::Builtin::kLocalInvocationIndex,
                    ast::StorageClass::kInput, SpvBuiltInLocalInvocationIndex},
        BuiltinData{ast::Builtin::kGlobalInvocationId,
                    ast::StorageClass::kInput, SpvBuiltInGlobalInvocationId},
        BuiltinData{ast::Builtin::kWorkgroupId, ast::StorageClass::kInput,
                    SpvBuiltInWorkgroupId},
        BuiltinData{ast::Builtin::kSampleIndex, ast::StorageClass::kInput,
                    SpvBuiltInSampleId},
        BuiltinData{ast::Builtin::kSampleMask, ast::StorageClass::kInput,
                    SpvBuiltInSampleMask},
        BuiltinData{ast::Builtin::kSampleMask, ast::StorageClass::kOutput,
                    SpvBuiltInSampleMask},
        BuiltinData{ast::Builtin::kSampleMaskIn, ast::StorageClass::kInput,
                    SpvBuiltInSampleMask},
        BuiltinData{ast::Builtin::kSampleMaskOut, ast::StorageClass::kOutput,
                    SpvBuiltInSampleMask}));

TEST_F(BuilderTest, GlobalVar_DeclReadOnly) {
  // struct A {
  //   a : i32;
  // };
  // var b : [[access(read)]] A

  auto* A = Structure("A",
                      {
                          Member("a", ty.i32()),
                          Member("b", ty.i32()),
                      },
                      {create<ast::StructBlockDecoration>()});
  auto* ac = ty.access(ast::AccessControl::kReadOnly, A);

  auto* var = Global("b", ac, ast::StorageClass::kStorage, nullptr,
                     {
                         create<ast::BindingDecoration>(0),
                         create<ast::GroupDecoration>(0),
                     });

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

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
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 1
%3 = OpTypeStruct %4 %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
)");
}

TEST_F(BuilderTest, GlobalVar_TypeAliasDeclReadOnly) {
  // struct A {
  //   a : i32;
  // };
  // type B = A;
  // var b : [[access(read)]] B

  auto* A = Structure("A", {Member("a", ty.i32())},
                      {create<ast::StructBlockDecoration>()});
  auto* B = ty.alias("B", A);
  AST().AddConstructedType(B);
  auto* ac = ty.access(ast::AccessControl::kReadOnly, B);
  auto* var = Global("b", ac, ast::StorageClass::kStorage, nullptr,
                     {
                         create<ast::BindingDecoration>(0),
                         create<ast::GroupDecoration>(0),
                     });

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %3 Block
OpMemberDecorate %3 0 Offset 0
OpDecorate %1 NonWritable
OpDecorate %1 Binding 0
OpDecorate %1 DescriptorSet 0
)");
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %3 "A"
OpMemberName %3 0 "a"
OpName %1 "b"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 1
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
)");
}

TEST_F(BuilderTest, GlobalVar_TypeAliasAssignReadOnly) {
  // struct A {
  //   a : i32;
  // };
  // type B = [[access(read)]] A;
  // var b : B

  auto* A = Structure("A", {Member("a", ty.i32())},
                      {create<ast::StructBlockDecoration>()});
  auto* ac = ty.access(ast::AccessControl::kReadOnly, A);
  auto* B = ty.alias("B", ac);
  AST().AddConstructedType(B);
  auto* var = Global("b", B, ast::StorageClass::kStorage, nullptr,
                     {
                         create<ast::BindingDecoration>(0),
                         create<ast::GroupDecoration>(0),
                     });

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %3 Block
OpMemberDecorate %3 0 Offset 0
OpDecorate %1 NonWritable
OpDecorate %1 Binding 0
OpDecorate %1 DescriptorSet 0
)");
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %3 "A"
OpMemberName %3 0 "a"
OpName %1 "b"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 1
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
)");
}

TEST_F(BuilderTest, GlobalVar_TwoVarDeclReadOnly) {
  // struct A {
  //   a : i32;
  // };
  // var b : [[access(read)]] A
  // var c : [[access(read_write)]] A

  auto* A = Structure("A", {Member("a", ty.i32())},
                      {create<ast::StructBlockDecoration>()});
  auto* read = ty.access(ast::AccessControl::kReadOnly, A);
  auto* rw = ty.access(ast::AccessControl::kReadWrite, A);

  auto* var_b = Global("b", read, ast::StorageClass::kStorage, nullptr,
                       {
                           create<ast::GroupDecoration>(0),
                           create<ast::BindingDecoration>(0),
                       });
  auto* var_c = Global("c", rw, ast::StorageClass::kStorage, nullptr,
                       {
                           create<ast::GroupDecoration>(1),
                           create<ast::BindingDecoration>(0),
                       });

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(var_b)) << b.error();
  EXPECT_TRUE(b.GenerateGlobalVariable(var_c)) << b.error();

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
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 1
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%5 = OpVariable %2 StorageBuffer
)");
}

TEST_F(BuilderTest, GlobalVar_TextureStorageReadOnly) {
  // var<uniform_constant> a : [[access(read)]] texture_storage_2d<r32uint>;

  auto* type = ty.storage_texture(ast::TextureDimension::k2d,
                                  ast::ImageFormat::kR32Uint);

  auto* ac = ty.access(ast::AccessControl::kReadOnly, type);

  auto* var_a = Global("a", ac, ast::StorageClass::kNone, nullptr,
                       {
                           create<ast::BindingDecoration>(0),
                           create<ast::GroupDecoration>(0),
                       });

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(var_a)) << b.error();

  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %1 NonWritable
OpDecorate %1 Binding 0
OpDecorate %1 DescriptorSet 0
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 0
%3 = OpTypeImage %4 2D 0 0 0 2 R32ui
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
)");
}

TEST_F(BuilderTest, GlobalVar_TextureStorageWriteOnly) {
  // var<uniform_constant> a : [[access(write)]] texture_storage_2d<r32uint>;

  auto* type = ty.storage_texture(ast::TextureDimension::k2d,
                                  ast::ImageFormat::kR32Uint);

  auto* ac = ty.access(ast::AccessControl::kWriteOnly, type);

  auto* var_a = Global("a", ac, ast::StorageClass::kNone, nullptr,
                       {
                           create<ast::BindingDecoration>(0),
                           create<ast::GroupDecoration>(0),
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
TEST_F(BuilderTest, GlobalVar_TextureStorageWithDifferentAccess) {
  // var<uniform_constant> a : [[access(read)]] texture_storage_2d<r32uint>;
  // var<uniform_constant> b : [[access(write)]] texture_storage_2d<r32uint>;

  auto* type_a = ty.access(ast::AccessControl::kReadOnly,
                           ty.storage_texture(ast::TextureDimension::k2d,
                                              ast::ImageFormat::kR32Uint));
  auto* var_a = Global("a", type_a, ast::StorageClass::kNone, nullptr,
                       {
                           create<ast::BindingDecoration>(0),
                           create<ast::GroupDecoration>(0),
                       });

  auto* type_b = ty.access(ast::AccessControl::kWriteOnly,
                           ty.storage_texture(ast::TextureDimension::k2d,
                                              ast::ImageFormat::kR32Uint));
  auto* var_b = Global("b", type_b, ast::StorageClass::kNone, nullptr,
                       {
                           create<ast::BindingDecoration>(1),
                           create<ast::GroupDecoration>(0),
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

TEST_F(BuilderTest, SampleIndex) {
  auto* var =
      Global("sample_index", ty.u32(), ast::StorageClass::kInput, nullptr,
             ast::DecorationList{
                 Builtin(ast::Builtin::kSampleIndex),
             });

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  EXPECT_EQ(DumpInstructions(b.capabilities()),
            "OpCapability SampleRateShading\n");
  EXPECT_EQ(DumpInstructions(b.annots()), "OpDecorate %1 BuiltIn SampleId\n");
  EXPECT_EQ(DumpInstructions(b.types()),
            "%3 = OpTypeInt 32 0\n"
            "%2 = OpTypePointer Input %3\n"
            "%1 = OpVariable %2 Input\n");
}

TEST_F(BuilderTest, SampleMask) {
  // Input:
  // [[builtin(sample_mask)]] var<in> mask_in : u32;
  // [[builtin(sample_mask)]] var<out> mask_out : u32;
  // [[stage(fragment)]]
  // fn main() {
  //   mask_out = mask_in;
  // }

  // After sanitization:
  // [[builtin(sample_mask)]] var<in> mask_in : array<u32, 1>;
  // [[builtin(sample_mask)]] var<out> mask_out : array<u32, 1>;
  // [[stage(fragment)]]
  // fn main() {
  //   mask_out[0] = mask_in[0];
  // }

  Global("mask_in", ty.u32(), ast::StorageClass::kInput, nullptr,
         ast::DecorationList{
             Builtin(ast::Builtin::kSampleMask),
         });
  Global("mask_out", ty.u32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{
             Builtin(ast::Builtin::kSampleMask),
         });
  Func("main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Assign("mask_out", "mask_in"),
       },
       ast::DecorationList{
           Stage(ast::PipelineStage::kCompute),
       });

  spirv::Builder& b = SanitizeAndBuild();

  ASSERT_TRUE(b.Build());
  EXPECT_EQ(DumpBuilder(b), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %11 "main" %6 %1
OpExecutionMode %11 LocalSize 1 1 1
OpName %1 "mask_in"
OpName %6 "mask_out"
OpName %11 "main"
OpDecorate %3 ArrayStride 4
OpDecorate %1 BuiltIn SampleMask
OpDecorate %6 BuiltIn SampleMask
%4 = OpTypeInt 32 0
%5 = OpConstant %4 1
%3 = OpTypeArray %4 %5
%2 = OpTypePointer Input %3
%1 = OpVariable %2 Input
%7 = OpTypePointer Output %3
%8 = OpConstantNull %3
%6 = OpVariable %7 Output %8
%10 = OpTypeVoid
%9 = OpTypeFunction %10
%13 = OpTypeInt 32 1
%14 = OpConstant %13 0
%15 = OpTypePointer Output %4
%17 = OpTypePointer Input %4
%11 = OpFunction %10 None %9
%12 = OpLabel
%16 = OpAccessChain %15 %6 %14
%18 = OpAccessChain %17 %1 %14
%19 = OpLoad %4 %18
OpStore %16 %19
OpReturn
OpFunctionEnd
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
