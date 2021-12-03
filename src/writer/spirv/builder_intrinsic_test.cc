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

#include "src/ast/call_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/struct_block_decoration.h"
#include "src/sem/depth_texture_type.h"
#include "src/utils/string.h"
#include "src/writer/spirv/spv_dump.h"
#include "src/writer/spirv/test_helper.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using IntrinsicBuilderTest = TestHelper;

template <typename T>
using IntrinsicBuilderTestWithParam = TestParamHelper<T>;

struct IntrinsicData {
  std::string name;
  std::string op;
};
inline std::ostream& operator<<(std::ostream& out, IntrinsicData data) {
  out << data.name;
  return out;
}

using IntrinsicBoolTest = IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(IntrinsicBoolTest, Call_Bool_Scalar) {
  auto param = GetParam();
  auto* var = Global("v", ty.bool_(), ast::StorageClass::kPrivate);
  auto* expr = Call(param.name, "v");
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeBool
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%6 = OpTypeVoid
%5 = OpTypeFunction %6
)");

  // both any and all are 'passthrough' for scalar booleans
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            "%10 = OpLoad %3 %1\nOpReturn\n");
}

TEST_P(IntrinsicBoolTest, Call_Bool_Vector) {
  auto param = GetParam();
  auto* var = Global("v", ty.vec3<bool>(), ast::StorageClass::kPrivate);
  auto* expr = Call(param.name, "v");
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeBool
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%7 = OpTypeVoid
%6 = OpTypeFunction %7
)");

  auto expected = utils::ReplaceAll(R"(%11 = OpLoad %3 %1
%10 = ${op} %4 %11
OpReturn
)",
                                    "${op}", param.op);
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), expected);
}
INSTANTIATE_TEST_SUITE_P(IntrinsicBuilderTest,
                         IntrinsicBoolTest,
                         testing::Values(IntrinsicData{"any", "OpAny"},
                                         IntrinsicData{"all", "OpAll"}));

using IntrinsicFloatTest = IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(IntrinsicFloatTest, Call_Float_Scalar) {
  auto param = GetParam();
  auto* var = Global("v", ty.f32(), ast::StorageClass::kPrivate);
  auto* expr = Call(param.name, "v");
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%6 = OpTypeVoid
%5 = OpTypeFunction %6
%10 = OpTypeBool
)");

  auto expected = utils::ReplaceAll(R"(%11 = OpLoad %3 %1
%9 = ${op} %10 %11
OpReturn
)",
                                    "${op}", param.op);
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), expected);
}

TEST_P(IntrinsicFloatTest, Call_Float_Vector) {
  auto param = GetParam();
  auto* var = Global("v", ty.vec3<f32>(), ast::StorageClass::kPrivate);
  auto* expr = Call(param.name, "v");
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%7 = OpTypeVoid
%6 = OpTypeFunction %7
%12 = OpTypeBool
%11 = OpTypeVector %12 3
)");

  auto expected = utils::ReplaceAll(R"(%13 = OpLoad %3 %1
%10 = ${op} %11 %13
OpReturn
)",
                                    "${op}", param.op);
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), expected);
}
INSTANTIATE_TEST_SUITE_P(IntrinsicBuilderTest,
                         IntrinsicFloatTest,
                         testing::Values(IntrinsicData{"isNan", "OpIsNan"},
                                         IntrinsicData{"isInf", "OpIsInf"}));

TEST_F(IntrinsicBuilderTest, IsFinite_Scalar) {
  auto* var = Global("v", ty.f32(), ast::StorageClass::kPrivate);
  auto* expr = Call("isFinite", "v");
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%6 = OpTypeVoid
%5 = OpTypeFunction %6
%10 = OpTypeBool
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%11 = OpLoad %3 %1
%12 = OpIsInf %10 %11
%13 = OpIsNan %10 %11
%14 = OpLogicalOr %10 %12 %13
%9 = OpLogicalNot %10 %14
OpReturn
)");
}

TEST_F(IntrinsicBuilderTest, IsFinite_Vector) {
  auto* var = Global("v", ty.vec3<f32>(), ast::StorageClass::kPrivate);
  auto* expr = Call("isFinite", "v");
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%7 = OpTypeVoid
%6 = OpTypeFunction %7
%12 = OpTypeBool
%11 = OpTypeVector %12 3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%13 = OpLoad %3 %1
%14 = OpIsInf %11 %13
%15 = OpIsNan %11 %13
%16 = OpLogicalOr %11 %14 %15
%10 = OpLogicalNot %11 %16
OpReturn
)");
}

TEST_F(IntrinsicBuilderTest, IsNormal_Scalar) {
  auto* var = Global("v", ty.f32(), ast::StorageClass::kPrivate);
  auto* expr = Call("isNormal", "v");
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  auto got = DumpBuilder(b);
  EXPECT_EQ(got, R"(%12 = OpExtInstImport "GLSL.std.450"
OpName %1 "v"
OpName %7 "a_func"
%3 = OpTypeFloat 32
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%6 = OpTypeVoid
%5 = OpTypeFunction %6
%10 = OpTypeBool
%13 = OpTypeInt 32 0
%14 = OpConstant %13 133693440
%15 = OpConstant %13 524288
%16 = OpConstant %13 133169152
%7 = OpFunction %6 None %5
%8 = OpLabel
%11 = OpLoad %3 %1
%17 = OpBitcast %13 %11
%18 = OpBitwiseAnd %13 %17 %14
%19 = OpExtInst %13 %12 UClamp %18 %15 %16
%9 = OpIEqual %10 %18 %19
OpReturn
OpFunctionEnd
)");
}

TEST_F(IntrinsicBuilderTest, IsNormal_Vector) {
  auto* var = Global("v", ty.vec2<f32>(), ast::StorageClass::kPrivate);
  auto* expr = Call("isNormal", "v");
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  auto got = DumpBuilder(b);
  EXPECT_EQ(got, R"(%14 = OpExtInstImport "GLSL.std.450"
OpName %1 "v"
OpName %8 "a_func"
%4 = OpTypeFloat 32
%3 = OpTypeVector %4 2
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%7 = OpTypeVoid
%6 = OpTypeFunction %7
%12 = OpTypeBool
%11 = OpTypeVector %12 2
%15 = OpTypeInt 32 0
%16 = OpConstant %15 133693440
%17 = OpConstant %15 524288
%18 = OpConstant %15 133169152
%19 = OpTypeVector %15 2
%8 = OpFunction %7 None %6
%9 = OpLabel
%13 = OpLoad %3 %1
%20 = OpCompositeConstruct %19 %16 %16
%21 = OpCompositeConstruct %19 %17 %17
%22 = OpCompositeConstruct %19 %18 %18
%23 = OpBitcast %19 %13
%24 = OpBitwiseAnd %19 %23 %20
%25 = OpExtInst %19 %14 UClamp %24 %21 %22
%10 = OpIEqual %11 %24 %25
OpReturn
OpFunctionEnd
)");
}

using IntrinsicIntTest = IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(IntrinsicIntTest, Call_SInt_Scalar) {
  auto param = GetParam();
  auto* var = Global("v", ty.i32(), ast::StorageClass::kPrivate);
  auto* expr = Call(param.name, "v");
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%6 = OpTypeVoid
%5 = OpTypeFunction %6
)");

  auto expected = utils::ReplaceAll(R"(%10 = OpLoad %3 %1
%9 = ${op} %3 %10
OpReturn
)",
                                    "${op}", param.op);
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), expected);
}

TEST_P(IntrinsicIntTest, Call_SInt_Vector) {
  auto param = GetParam();
  auto* var = Global("v", ty.vec3<i32>(), ast::StorageClass::kPrivate);
  auto* expr = Call(param.name, "v");
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 1
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%7 = OpTypeVoid
%6 = OpTypeFunction %7
)");

  auto expected = utils::ReplaceAll(R"(%11 = OpLoad %3 %1
%10 = ${op} %3 %11
OpReturn
)",
                                    "${op}", param.op);
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), expected);
}

TEST_P(IntrinsicIntTest, Call_UInt_Scalar) {
  auto param = GetParam();
  auto* var = Global("v", ty.u32(), ast::StorageClass::kPrivate);
  auto* expr = Call(param.name, "v");
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeInt 32 0
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%6 = OpTypeVoid
%5 = OpTypeFunction %6
)");

  auto expected = utils::ReplaceAll(R"(%10 = OpLoad %3 %1
%9 = ${op} %3 %10
OpReturn
)",
                                    "${op}", param.op);
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), expected);
}

TEST_P(IntrinsicIntTest, Call_UInt_Vector) {
  auto param = GetParam();
  auto* var = Global("v", ty.vec3<u32>(), ast::StorageClass::kPrivate);
  auto* expr = Call(param.name, "v");
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 0
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%7 = OpTypeVoid
%6 = OpTypeFunction %7
)");

  auto expected = utils::ReplaceAll(R"(%11 = OpLoad %3 %1
%10 = ${op} %3 %11
OpReturn
)",
                                    "${op}", param.op);
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), expected);
}
INSTANTIATE_TEST_SUITE_P(
    IntrinsicBuilderTest,
    IntrinsicIntTest,
    testing::Values(IntrinsicData{"countOneBits", "OpBitCount"},
                    IntrinsicData{"reverseBits", "OpBitReverse"}));

TEST_F(IntrinsicBuilderTest, Call_Dot_F32) {
  auto* var = Global("v", ty.vec3<f32>(), ast::StorageClass::kPrivate);
  auto* expr = Call("dot", "v", "v");
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%7 = OpTypeVoid
%6 = OpTypeFunction %7
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%11 = OpLoad %3 %1
%12 = OpLoad %3 %1
%10 = OpDot %4 %11 %12
OpReturn
)");
}

TEST_F(IntrinsicBuilderTest, Call_Dot_U32) {
  auto* var = Global("v", ty.vec3<u32>(), ast::StorageClass::kPrivate);
  auto* expr = Call("dot", "v", "v");
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 0
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%7 = OpTypeVoid
%6 = OpTypeFunction %7
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%11 = OpLoad %3 %1
%12 = OpLoad %3 %1
%13 = OpCompositeExtract %4 %11 0
%14 = OpCompositeExtract %4 %12 0
%15 = OpIMul %4 %13 %14
%16 = OpCompositeExtract %4 %11 1
%17 = OpCompositeExtract %4 %12 1
%18 = OpIMul %4 %16 %17
%19 = OpIAdd %4 %15 %18
%20 = OpCompositeExtract %4 %11 2
%21 = OpCompositeExtract %4 %12 2
%22 = OpIMul %4 %20 %21
%10 = OpIAdd %4 %19 %22
OpReturn
)");
}

TEST_F(IntrinsicBuilderTest, Call_Dot_I32) {
  auto* var = Global("v", ty.vec3<i32>(), ast::StorageClass::kPrivate);
  auto* expr = Call("dot", "v", "v");
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 1
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%7 = OpTypeVoid
%6 = OpTypeFunction %7
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%11 = OpLoad %3 %1
%12 = OpLoad %3 %1
%13 = OpCompositeExtract %4 %11 0
%14 = OpCompositeExtract %4 %12 0
%15 = OpIMul %4 %13 %14
%16 = OpCompositeExtract %4 %11 1
%17 = OpCompositeExtract %4 %12 1
%18 = OpIMul %4 %16 %17
%19 = OpIAdd %4 %15 %18
%20 = OpCompositeExtract %4 %11 2
%21 = OpCompositeExtract %4 %12 2
%22 = OpIMul %4 %20 %21
%10 = OpIAdd %4 %19 %22
OpReturn
)");
}

using IntrinsicDeriveTest = IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(IntrinsicDeriveTest, Call_Derivative_Scalar) {
  auto param = GetParam();
  auto* var = Global("v", ty.f32(), ast::StorageClass::kPrivate);
  auto* expr = Call(param.name, "v");
  auto* func = Func("func", {}, ty.void_(), {CallStmt(expr)},
                    {Stage(ast::PipelineStage::kFragment)});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%6 = OpTypeVoid
%5 = OpTypeFunction %6
)");

  auto expected = utils::ReplaceAll(R"(%10 = OpLoad %3 %1
%9 = ${op} %3 %10
OpReturn
)",
                                    "${op}", param.op);
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), expected);
}

TEST_P(IntrinsicDeriveTest, Call_Derivative_Vector) {
  auto param = GetParam();
  auto* var = Global("v", ty.vec3<f32>(), ast::StorageClass::kPrivate);
  auto* expr = Call(param.name, "v");
  auto* func = Func("func", {}, ty.void_(), {CallStmt(expr)},
                    {Stage(ast::PipelineStage::kFragment)});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  if (param.name != "dpdx" && param.name != "dpdy" && param.name != "fwidth") {
    EXPECT_EQ(DumpInstructions(b.capabilities()),
              R"(OpCapability DerivativeControl
)");
  }

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%7 = OpTypeVoid
%6 = OpTypeFunction %7
)");

  auto expected = utils::ReplaceAll(R"(%11 = OpLoad %3 %1
%10 = ${op} %3 %11
OpReturn
)",
                                    "${op}", param.op);
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), expected);
}
INSTANTIATE_TEST_SUITE_P(
    IntrinsicBuilderTest,
    IntrinsicDeriveTest,
    testing::Values(IntrinsicData{"dpdx", "OpDPdx"},
                    IntrinsicData{"dpdxFine", "OpDPdxFine"},
                    IntrinsicData{"dpdxCoarse", "OpDPdxCoarse"},
                    IntrinsicData{"dpdy", "OpDPdy"},
                    IntrinsicData{"dpdyFine", "OpDPdyFine"},
                    IntrinsicData{"dpdyCoarse", "OpDPdyCoarse"},
                    IntrinsicData{"fwidth", "OpFwidth"},
                    IntrinsicData{"fwidthFine", "OpFwidthFine"},
                    IntrinsicData{"fwidthCoarse", "OpFwidthCoarse"}));

TEST_F(IntrinsicBuilderTest, Call_Select) {
  auto* v3 = Global("v3", ty.vec3<f32>(), ast::StorageClass::kPrivate);

  auto* bool_v3 =
      Global("bool_v3", ty.vec3<bool>(), ast::StorageClass::kPrivate);
  auto* expr = Call("select", "v3", "v3", "bool_v3");
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(v3)) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(bool_v3)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%9 = OpTypeBool
%8 = OpTypeVector %9 3
%7 = OpTypePointer Private %8
%10 = OpConstantNull %8
%6 = OpVariable %7 Private %10
%12 = OpTypeVoid
%11 = OpTypeFunction %12
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%16 = OpLoad %8 %6
%17 = OpLoad %3 %1
%18 = OpLoad %3 %1
%15 = OpSelect %3 %16 %17 %18
OpReturn
)");
}

// This tests that we do not push OpTypeSampledImage and float_0 type twice.
TEST_F(IntrinsicBuilderTest, Call_TextureSampleCompare_Twice) {
  auto* s = ty.sampler(ast::SamplerKind::kComparisonSampler);
  auto* t = ty.depth_texture(ast::TextureDimension::k2d);

  auto* tex = Global("texture", t,
                     ast::DecorationList{
                         create<ast::BindingDecoration>(0),
                         create<ast::GroupDecoration>(0),
                     });

  auto* sampler = Global("sampler", s,
                         ast::DecorationList{
                             create<ast::BindingDecoration>(1),
                             create<ast::GroupDecoration>(0),
                         });

  auto* expr1 = Call("textureSampleCompare", "texture", "sampler",
                     vec2<f32>(1.0f, 2.0f), 2.0f);
  auto* expr2 = Call("textureSampleCompare", "texture", "sampler",
                     vec2<f32>(1.0f, 2.0f), 2.0f);

  Func("f1", {}, ty.void_(), {CallStmt(expr1)}, {});
  Func("f2", {}, ty.void_(), {CallStmt(expr2)}, {});

  spirv::Builder& b = Build();

  b.push_function(Function{});

  ASSERT_TRUE(b.GenerateGlobalVariable(tex)) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(sampler)) << b.error();

  EXPECT_EQ(b.GenerateExpression(expr1), 8u) << b.error();
  EXPECT_EQ(b.GenerateExpression(expr2), 17u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 0 0 1 Unknown
%2 = OpTypePointer UniformConstant %3
%1 = OpVariable %2 UniformConstant
%7 = OpTypeSampler
%6 = OpTypePointer UniformConstant %7
%5 = OpVariable %6 UniformConstant
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 2
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstantComposite %13 %14 %15
)");

  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%8 = OpImageSampleDrefImplicitLod %4 %12 %16 %15
%18 = OpLoad %7 %5
%19 = OpLoad %3 %1
%20 = OpSampledImage %11 %19 %18
%17 = OpImageSampleDrefImplicitLod %4 %20 %16 %15
)");
}

TEST_F(IntrinsicBuilderTest, Call_GLSLMethod_WithLoad) {
  auto* var = Global("ident", ty.f32(), ast::StorageClass::kPrivate);
  auto* expr = Call("round", "ident");
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(%10 = OpExtInstImport "GLSL.std.450"
OpName %1 "ident"
OpName %7 "a_func"
%3 = OpTypeFloat 32
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%6 = OpTypeVoid
%5 = OpTypeFunction %6
%7 = OpFunction %6 None %5
%8 = OpLabel
%11 = OpLoad %3 %1
%9 = OpExtInst %3 %10 RoundEven %11
OpReturn
OpFunctionEnd
)");
}

using Intrinsic_Builtin_SingleParam_Float_Test =
    IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_SingleParam_Float_Test, Call_Scalar) {
  auto param = GetParam();
  auto* expr = Call(param.name, 1.0f);
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 )" + param.op +
                                R"( %8
OpReturn
OpFunctionEnd
)");
}

TEST_P(Intrinsic_Builtin_SingleParam_Float_Test, Call_Vector) {
  auto param = GetParam();
  auto* expr = Call(param.name, vec2<f32>(1.0f, 1.0f));
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeFloat 32
%6 = OpTypeVector %7 2
%9 = OpConstant %7 1
%10 = OpConstantComposite %6 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %8 )" + param.op +
                                R"( %10
OpReturn
OpFunctionEnd
)");
}
INSTANTIATE_TEST_SUITE_P(IntrinsicBuilderTest,
                         Intrinsic_Builtin_SingleParam_Float_Test,
                         testing::Values(IntrinsicData{"abs", "FAbs"},
                                         IntrinsicData{"acos", "Acos"},
                                         IntrinsicData{"asin", "Asin"},
                                         IntrinsicData{"atan", "Atan"},
                                         IntrinsicData{"ceil", "Ceil"},
                                         IntrinsicData{"cos", "Cos"},
                                         IntrinsicData{"cosh", "Cosh"},
                                         IntrinsicData{"exp", "Exp"},
                                         IntrinsicData{"exp2", "Exp2"},
                                         IntrinsicData{"floor", "Floor"},
                                         IntrinsicData{"fract", "Fract"},
                                         IntrinsicData{"inverseSqrt",
                                                       "InverseSqrt"},
                                         IntrinsicData{"log", "Log"},
                                         IntrinsicData{"log2", "Log2"},
                                         IntrinsicData{"round", "RoundEven"},
                                         IntrinsicData{"sign", "FSign"},
                                         IntrinsicData{"sin", "Sin"},
                                         IntrinsicData{"sinh", "Sinh"},
                                         IntrinsicData{"sqrt", "Sqrt"},
                                         IntrinsicData{"tan", "Tan"},
                                         IntrinsicData{"tanh", "Tanh"},
                                         IntrinsicData{"trunc", "Trunc"}));

TEST_F(IntrinsicBuilderTest, Call_Length_Scalar) {
  auto* expr = Call("length", 1.0f);
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 Length %8
OpReturn
OpFunctionEnd
)");
}

TEST_F(IntrinsicBuilderTest, Call_Length_Vector) {
  auto* expr = Call("length", vec2<f32>(1.0f, 1.0f));
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%8 = OpTypeVector %6 2
%9 = OpConstant %6 1
%10 = OpConstantComposite %8 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 Length %10
OpReturn
OpFunctionEnd
)");
}

TEST_F(IntrinsicBuilderTest, Call_Normalize) {
  auto* expr = Call("normalize", vec2<f32>(1.0f, 1.0f));
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeFloat 32
%6 = OpTypeVector %7 2
%9 = OpConstant %7 1
%10 = OpConstantComposite %6 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %8 Normalize %10
OpReturn
OpFunctionEnd
)");
}

using Intrinsic_Builtin_DualParam_Float_Test =
    IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_DualParam_Float_Test, Call_Scalar) {
  auto param = GetParam();
  auto* expr = Call(param.name, 1.0f, 1.0f);
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 )" + param.op +
                                R"( %8 %8
OpReturn
OpFunctionEnd
)");
}

TEST_P(Intrinsic_Builtin_DualParam_Float_Test, Call_Vector) {
  auto param = GetParam();
  auto* expr = Call(param.name, vec2<f32>(1.0f, 1.0f), vec2<f32>(1.0f, 1.0f));
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeFloat 32
%6 = OpTypeVector %7 2
%9 = OpConstant %7 1
%10 = OpConstantComposite %6 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %8 )" + param.op +
                                R"( %10 %10
OpReturn
OpFunctionEnd
)");
}
INSTANTIATE_TEST_SUITE_P(IntrinsicBuilderTest,
                         Intrinsic_Builtin_DualParam_Float_Test,
                         testing::Values(IntrinsicData{"atan2", "Atan2"},
                                         IntrinsicData{"max", "NMax"},
                                         IntrinsicData{"min", "NMin"},
                                         IntrinsicData{"pow", "Pow"},
                                         IntrinsicData{"step", "Step"}));

TEST_F(IntrinsicBuilderTest, Call_Reflect_Vector) {
  auto* expr = Call("reflect", vec2<f32>(1.0f, 1.0f), vec2<f32>(1.0f, 1.0f));
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeFloat 32
%6 = OpTypeVector %7 2
%9 = OpConstant %7 1
%10 = OpConstantComposite %6 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %8 Reflect %10 %10
OpReturn
OpFunctionEnd
)");
}

TEST_F(IntrinsicBuilderTest, Call_Distance_Scalar) {
  auto* expr = Call("distance", 1.0f, 1.0f);
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 Distance %8 %8
OpReturn
OpFunctionEnd
)");
}

TEST_F(IntrinsicBuilderTest, Call_Distance_Vector) {
  auto* expr = Call("distance", vec2<f32>(1.0f, 1.0f), vec2<f32>(1.0f, 1.0f));
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%8 = OpTypeVector %6 2
%9 = OpConstant %6 1
%10 = OpConstantComposite %8 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 Distance %10 %10
OpReturn
OpFunctionEnd
)");
}

TEST_F(IntrinsicBuilderTest, Call_Cross) {
  auto* expr =
      Call("cross", vec3<f32>(1.0f, 1.0f, 1.0f), vec3<f32>(1.0f, 1.0f, 1.0f));
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeFloat 32
%6 = OpTypeVector %7 3
%9 = OpConstant %7 1
%10 = OpConstantComposite %6 %9 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %8 Cross %10 %10
OpReturn
OpFunctionEnd
)");
}

using Intrinsic_Builtin_ThreeParam_Float_Test =
    IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_ThreeParam_Float_Test, Call_Scalar) {
  auto param = GetParam();
  auto* expr = Call(param.name, 1.0f, 1.0f, 1.0f);
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 )" + param.op +
                                R"( %8 %8 %8
OpReturn
OpFunctionEnd
)");
}

TEST_P(Intrinsic_Builtin_ThreeParam_Float_Test, Call_Vector) {
  auto param = GetParam();
  auto* expr = Call(param.name, vec2<f32>(1.0f, 1.0f), vec2<f32>(1.0f, 1.0f),
                    vec2<f32>(1.0f, 1.0f));
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeFloat 32
%6 = OpTypeVector %7 2
%9 = OpConstant %7 1
%10 = OpConstantComposite %6 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %8 )" + param.op +
                                R"( %10 %10 %10
OpReturn
OpFunctionEnd
)");
}
INSTANTIATE_TEST_SUITE_P(IntrinsicBuilderTest,
                         Intrinsic_Builtin_ThreeParam_Float_Test,
                         testing::Values(IntrinsicData{"clamp", "NClamp"},
                                         IntrinsicData{"fma", "Fma"},
                                         IntrinsicData{"mix", "FMix"},

                                         IntrinsicData{"smoothStep",
                                                       "SmoothStep"}));

TEST_F(IntrinsicBuilderTest, Call_FaceForward_Vector) {
  auto* expr = Call("faceForward", vec2<f32>(1.0f, 1.0f), vec2<f32>(1.0f, 1.0f),
                    vec2<f32>(1.0f, 1.0f));
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeFloat 32
%6 = OpTypeVector %7 2
%9 = OpConstant %7 1
%10 = OpConstantComposite %6 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %8 FaceForward %10 %10 %10
OpReturn
OpFunctionEnd
)");
}

using Intrinsic_Builtin_SingleParam_Sint_Test =
    IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_SingleParam_Sint_Test, Call_Scalar) {
  auto param = GetParam();
  auto* expr = Call(param.name, 1);
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 1
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 )" + param.op +
                                R"( %8
OpReturn
OpFunctionEnd
)");
}

TEST_P(Intrinsic_Builtin_SingleParam_Sint_Test, Call_Vector) {
  auto param = GetParam();
  auto* expr = Call(param.name, vec2<i32>(1, 1));
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeInt 32 1
%6 = OpTypeVector %7 2
%9 = OpConstant %7 1
%10 = OpConstantComposite %6 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %8 )" + param.op +
                                R"( %10
OpReturn
OpFunctionEnd
)");
}
INSTANTIATE_TEST_SUITE_P(IntrinsicBuilderTest,
                         Intrinsic_Builtin_SingleParam_Sint_Test,
                         testing::Values(IntrinsicData{"abs", "SAbs"}));

// Calling abs() on an unsigned integer scalar / vector is a no-op.
using Intrinsic_Builtin_Abs_Uint_Test = IntrinsicBuilderTest;
TEST_F(Intrinsic_Builtin_Abs_Uint_Test, Call_Scalar) {
  auto* expr = Call("abs", 1u);
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 0
%7 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(Intrinsic_Builtin_Abs_Uint_Test, Call_Vector) {
  auto* expr = Call("abs", vec2<u32>(1u, 1u));
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeInt 32 0
%6 = OpTypeVector %7 2
%8 = OpConstant %7 1
%9 = OpConstantComposite %6 %8 %8
%3 = OpFunction %2 None %1
%4 = OpLabel
OpReturn
OpFunctionEnd
)");
}

using Intrinsic_Builtin_DualParam_SInt_Test =
    IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_DualParam_SInt_Test, Call_Scalar) {
  auto param = GetParam();
  auto* expr = Call(param.name, 1, 1);
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 1
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 )" + param.op +
                                R"( %8 %8
OpReturn
OpFunctionEnd
)");
}

TEST_P(Intrinsic_Builtin_DualParam_SInt_Test, Call_Vector) {
  auto param = GetParam();
  auto* expr = Call(param.name, vec2<i32>(1, 1), vec2<i32>(1, 1));
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeInt 32 1
%6 = OpTypeVector %7 2
%9 = OpConstant %7 1
%10 = OpConstantComposite %6 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %8 )" + param.op +
                                R"( %10 %10
OpReturn
OpFunctionEnd
)");
}
INSTANTIATE_TEST_SUITE_P(IntrinsicBuilderTest,
                         Intrinsic_Builtin_DualParam_SInt_Test,
                         testing::Values(IntrinsicData{"max", "SMax"},
                                         IntrinsicData{"min", "SMin"}));

using Intrinsic_Builtin_DualParam_UInt_Test =
    IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_DualParam_UInt_Test, Call_Scalar) {
  auto param = GetParam();
  auto* expr = Call(param.name, 1u, 1u);
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 0
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 )" + param.op +
                                R"( %8 %8
OpReturn
OpFunctionEnd
)");
}

TEST_P(Intrinsic_Builtin_DualParam_UInt_Test, Call_Vector) {
  auto param = GetParam();
  auto* expr = Call(param.name, vec2<u32>(1u, 1u), vec2<u32>(1u, 1u));
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeInt 32 0
%6 = OpTypeVector %7 2
%9 = OpConstant %7 1
%10 = OpConstantComposite %6 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %8 )" + param.op +
                                R"( %10 %10
OpReturn
OpFunctionEnd
)");
}
INSTANTIATE_TEST_SUITE_P(IntrinsicBuilderTest,
                         Intrinsic_Builtin_DualParam_UInt_Test,
                         testing::Values(IntrinsicData{"max", "UMax"},
                                         IntrinsicData{"min", "UMin"}));

using Intrinsic_Builtin_ThreeParam_Sint_Test =
    IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_ThreeParam_Sint_Test, Call_Scalar) {
  auto param = GetParam();
  auto* expr = Call(param.name, 1, 1, 1);
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 1
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 )" + param.op +
                                R"( %8 %8 %8
OpReturn
OpFunctionEnd
)");
}

TEST_P(Intrinsic_Builtin_ThreeParam_Sint_Test, Call_Vector) {
  auto param = GetParam();
  auto* expr =
      Call(param.name, vec2<i32>(1, 1), vec2<i32>(1, 1), vec2<i32>(1, 1));
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeInt 32 1
%6 = OpTypeVector %7 2
%9 = OpConstant %7 1
%10 = OpConstantComposite %6 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %8 )" + param.op +
                                R"( %10 %10 %10
OpReturn
OpFunctionEnd
)");
}
INSTANTIATE_TEST_SUITE_P(IntrinsicBuilderTest,
                         Intrinsic_Builtin_ThreeParam_Sint_Test,
                         testing::Values(IntrinsicData{"clamp", "SClamp"}));

using Intrinsic_Builtin_ThreeParam_Uint_Test =
    IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_ThreeParam_Uint_Test, Call_Scalar) {
  auto param = GetParam();
  auto* expr = Call(param.name, 1u, 1u, 1u);
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 0
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 )" + param.op +
                                R"( %8 %8 %8
OpReturn
OpFunctionEnd
)");
}

TEST_P(Intrinsic_Builtin_ThreeParam_Uint_Test, Call_Vector) {
  auto param = GetParam();
  auto* expr =
      Call(param.name, vec2<u32>(1u, 1u), vec2<u32>(1u, 1u), vec2<u32>(1u, 1u));
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeInt 32 0
%6 = OpTypeVector %7 2
%9 = OpConstant %7 1
%10 = OpConstantComposite %6 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %8 )" + param.op +
                                R"( %10 %10 %10
OpReturn
OpFunctionEnd
)");
}
INSTANTIATE_TEST_SUITE_P(IntrinsicBuilderTest,
                         Intrinsic_Builtin_ThreeParam_Uint_Test,
                         testing::Values(IntrinsicData{"clamp", "UClamp"}));

TEST_F(IntrinsicBuilderTest, Call_Modf) {
  auto* expr = Call("modf", vec2<f32>(1.0f, 2.0f));
  Func("a_func", {}, ty.void_(), {CallStmt(expr)},
       {Stage(ast::PipelineStage::kFragment)});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.Build()) << b.error();
  auto got = DumpBuilder(b);
  auto* expect = R"(OpCapability Shader
%9 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %3 "a_func"
OpExecutionMode %3 OriginUpperLeft
OpName %3 "a_func"
OpName %6 "__modf_result_vec2"
OpMemberName %6 0 "fract"
OpMemberName %6 1 "whole"
OpMemberDecorate %6 0 Offset 0
OpMemberDecorate %6 1 Offset 8
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%8 = OpTypeFloat 32
%7 = OpTypeVector %8 2
%6 = OpTypeStruct %7 %7
%10 = OpConstant %8 1
%11 = OpConstant %8 2
%12 = OpConstantComposite %7 %10 %11
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %9 ModfStruct %12
OpReturn
OpFunctionEnd
)";
  EXPECT_EQ(expect, got);

  Validate(b);
}

TEST_F(IntrinsicBuilderTest, Call_Frexp) {
  auto* expr = Call("frexp", vec2<f32>(1.0f, 2.0f));
  Func("a_func", {}, ty.void_(), {CallStmt(expr)},
       {Stage(ast::PipelineStage::kFragment)});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.Build()) << b.error();
  auto got = DumpBuilder(b);
  auto* expect = R"(OpCapability Shader
%11 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %3 "a_func"
OpExecutionMode %3 OriginUpperLeft
OpName %3 "a_func"
OpName %6 "__frexp_result_vec2"
OpMemberName %6 0 "sig"
OpMemberName %6 1 "exp"
OpMemberDecorate %6 0 Offset 0
OpMemberDecorate %6 1 Offset 8
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%8 = OpTypeFloat 32
%7 = OpTypeVector %8 2
%10 = OpTypeInt 32 1
%9 = OpTypeVector %10 2
%6 = OpTypeStruct %7 %9
%12 = OpConstant %8 1
%13 = OpConstant %8 2
%14 = OpConstantComposite %7 %12 %13
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %11 FrexpStruct %14
OpReturn
OpFunctionEnd
)";
  EXPECT_EQ(expect, got);

  Validate(b);
}

TEST_F(IntrinsicBuilderTest, Call_Determinant) {
  auto* var = Global("var", ty.mat3x3<f32>(), ast::StorageClass::kPrivate);
  auto* expr = Call("determinant", "var");
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(%12 = OpExtInstImport "GLSL.std.450"
OpName %1 "var"
OpName %9 "a_func"
%5 = OpTypeFloat 32
%4 = OpTypeVector %5 3
%3 = OpTypeMatrix %4 3
%2 = OpTypePointer Private %3
%6 = OpConstantNull %3
%1 = OpVariable %2 Private %6
%8 = OpTypeVoid
%7 = OpTypeFunction %8
%9 = OpFunction %8 None %7
%10 = OpLabel
%13 = OpLoad %3 %1
%11 = OpExtInst %5 %12 Determinant %13
OpReturn
OpFunctionEnd
)");
}

TEST_F(IntrinsicBuilderTest, Call_Transpose) {
  auto* var = Global("var", ty.mat2x3<f32>(), ast::StorageClass::kPrivate);
  auto* expr = Call("transpose", "var");
  auto* func = Func("a_func", {}, ty.void_(),
                    {
                        Assign(Phony(), expr),
                    });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(OpName %1 "var"
OpName %9 "a_func"
%5 = OpTypeFloat 32
%4 = OpTypeVector %5 3
%3 = OpTypeMatrix %4 2
%2 = OpTypePointer Private %3
%6 = OpConstantNull %3
%1 = OpVariable %2 Private %6
%8 = OpTypeVoid
%7 = OpTypeFunction %8
%13 = OpTypeVector %5 2
%12 = OpTypeMatrix %13 3
%9 = OpFunction %8 None %7
%10 = OpLabel
%14 = OpLoad %3 %1
%11 = OpTranspose %12 %14
OpReturn
OpFunctionEnd
)");
}

TEST_F(IntrinsicBuilderTest, Call_ArrayLength) {
  auto* s = Structure("my_struct", {Member("a", ty.array<f32>(4))},
                      {create<ast::StructBlockDecoration>()});
  Global("b", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kRead,
         ast::DecorationList{
             create<ast::BindingDecoration>(1),
             create<ast::GroupDecoration>(2),
         });
  auto* expr = Call("arrayLength", AddressOf(MemberAccessor("b", "a")));

  Func("a_func", {}, ty.void_(),
       ast::StatementList{
           CallStmt(expr),
       },
       ast::DecorationList{
           Stage(ast::PipelineStage::kFragment),
       });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.Build()) << b.error();

  ASSERT_EQ(b.functions().size(), 1u);

  auto* expected_types = R"(%5 = OpTypeFloat 32
%4 = OpTypeRuntimeArray %5
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%7 = OpTypeVoid
%6 = OpTypeFunction %7
%11 = OpTypeInt 32 0
)";
  auto got_types = DumpInstructions(b.types());
  EXPECT_EQ(expected_types, got_types);

  auto* expected_instructions = R"(%10 = OpArrayLength %11 %1 0
OpReturn
)";
  auto got_instructions = DumpInstructions(b.functions()[0].instructions());
  EXPECT_EQ(expected_instructions, got_instructions);

  Validate(b);
}

TEST_F(IntrinsicBuilderTest, Call_ArrayLength_OtherMembersInStruct) {
  auto* s = Structure("my_struct",
                      {
                          Member("z", ty.f32()),
                          Member(4, "a", ty.array<f32>(4)),
                      },
                      {create<ast::StructBlockDecoration>()});
  Global("b", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kRead,
         ast::DecorationList{
             create<ast::BindingDecoration>(1),
             create<ast::GroupDecoration>(2),
         });
  auto* expr = Call("arrayLength", AddressOf(MemberAccessor("b", "a")));

  Func("a_func", {}, ty.void_(),
       ast::StatementList{
           CallStmt(expr),
       },
       ast::DecorationList{
           Stage(ast::PipelineStage::kFragment),
       });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.Build()) << b.error();

  ASSERT_EQ(b.functions().size(), 1u);

  auto* expected_types = R"(%4 = OpTypeFloat 32
%5 = OpTypeRuntimeArray %4
%3 = OpTypeStruct %4 %5
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%7 = OpTypeVoid
%6 = OpTypeFunction %7
%11 = OpTypeInt 32 0
)";
  auto got_types = DumpInstructions(b.types());
  EXPECT_EQ(expected_types, got_types);

  auto* expected_instructions = R"(%10 = OpArrayLength %11 %1 1
OpReturn
)";
  auto got_instructions = DumpInstructions(b.functions()[0].instructions());
  EXPECT_EQ(expected_instructions, got_instructions);

  Validate(b);
}

TEST_F(IntrinsicBuilderTest, Call_ArrayLength_ViaLets) {
  auto* s = Structure("my_struct", {Member("a", ty.array<f32>(4))},
                      {create<ast::StructBlockDecoration>()});
  Global("b", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kRead,
         ast::DecorationList{
             create<ast::BindingDecoration>(1),
             create<ast::GroupDecoration>(2),
         });

  auto* p = Const("p", nullptr, AddressOf("b"));
  auto* p2 = Const("p2", nullptr, AddressOf(MemberAccessor(Deref(p), "a")));
  auto* expr = Call("arrayLength", p2);

  Func("a_func", {}, ty.void_(),
       ast::StatementList{
           Decl(p),
           Decl(p2),
           CallStmt(expr),
       },
       ast::DecorationList{
           Stage(ast::PipelineStage::kFragment),
       });

  spirv::Builder& b = SanitizeAndBuild();

  ASSERT_TRUE(b.Build()) << b.error();

  ASSERT_EQ(b.functions().size(), 1u);

  auto* expected_types = R"(%5 = OpTypeFloat 32
%4 = OpTypeRuntimeArray %5
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%7 = OpTypeVoid
%6 = OpTypeFunction %7
%11 = OpTypeInt 32 0
)";
  auto got_types = DumpInstructions(b.types());
  EXPECT_EQ(expected_types, got_types);

  auto* expected_instructions = R"(%10 = OpArrayLength %11 %1 0
OpReturn
)";
  auto got_instructions = DumpInstructions(b.functions()[0].instructions());
  EXPECT_EQ(expected_instructions, got_instructions);

  Validate(b);
}

TEST_F(IntrinsicBuilderTest, Call_ArrayLength_ViaLets_WithPtrNoise) {
  // [[block]] struct my_struct {
  //   a : [[stride(4)]] array<f32>;
  // };
  // [[binding(1), group(2)]] var<storage, read> b : my_struct;
  //
  // fn a_func() {
  //   let p = &*&b;
  //   let p2 = &*p;
  //   let p3 = &((*p).a);
  //   arrayLength(&*p3);
  // }
  auto* s = Structure("my_struct", {Member("a", ty.array<f32>(4))},
                      {create<ast::StructBlockDecoration>()});
  Global("b", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kRead,
         ast::DecorationList{
             create<ast::BindingDecoration>(1),
             create<ast::GroupDecoration>(2),
         });

  auto* p = Const("p", nullptr, AddressOf(Deref(AddressOf("b"))));
  auto* p2 = Const("p2", nullptr, AddressOf(Deref(p)));
  auto* p3 = Const("p3", nullptr, AddressOf(MemberAccessor(Deref(p2), "a")));
  auto* expr = Call("arrayLength", AddressOf(Deref(p3)));

  Func("a_func", {}, ty.void_(),
       ast::StatementList{
           Decl(p),
           Decl(p2),
           Decl(p3),
           CallStmt(expr),
       },
       ast::DecorationList{
           Stage(ast::PipelineStage::kFragment),
       });

  spirv::Builder& b = SanitizeAndBuild();

  ASSERT_TRUE(b.Build()) << b.error();

  ASSERT_EQ(b.functions().size(), 1u);

  auto* expected_types = R"(%5 = OpTypeFloat 32
%4 = OpTypeRuntimeArray %5
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%7 = OpTypeVoid
%6 = OpTypeFunction %7
%11 = OpTypeInt 32 0
)";
  auto got_types = DumpInstructions(b.types());
  EXPECT_EQ(expected_types, got_types);

  auto* expected_instructions = R"(%10 = OpArrayLength %11 %1 0
OpReturn
)";
  auto got_instructions = DumpInstructions(b.functions()[0].instructions());
  EXPECT_EQ(expected_instructions, got_instructions);

  Validate(b);
}

TEST_F(IntrinsicBuilderTest, Call_AtomicLoad) {
  // [[block]] struct S {
  //   u : atomic<u32>;
  //   i : atomic<i32>;
  // }
  //
  // [[binding(1), group(2)]] var<storage, read_write> b : S;
  //
  // fn a_func() {
  //   let u : u32 = atomicLoad(&b.u);
  //   let i : i32 = atomicLoad(&b.i);
  // }
  auto* s = Structure("S",
                      {
                          Member("u", ty.atomic<u32>()),
                          Member("i", ty.atomic<i32>()),
                      },
                      {create<ast::StructBlockDecoration>()});
  Global("b", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kReadWrite,
         ast::DecorationList{
             create<ast::BindingDecoration>(1),
             create<ast::GroupDecoration>(2),
         });

  Func("a_func", {}, ty.void_(),
       ast::StatementList{
           Decl(Const("u", ty.u32(),
                      Call("atomicLoad", AddressOf(MemberAccessor("b", "u"))))),
           Decl(Const("i", ty.i32(),
                      Call("atomicLoad", AddressOf(MemberAccessor("b", "i"))))),
       },
       ast::DecorationList{Stage(ast::PipelineStage::kFragment)});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.Build()) << b.error();

  ASSERT_EQ(b.functions().size(), 1u);

  auto* expected_types = R"(%4 = OpTypeInt 32 0
%5 = OpTypeInt 32 1
%3 = OpTypeStruct %4 %5
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%7 = OpTypeVoid
%6 = OpTypeFunction %7
%11 = OpConstant %4 1
%12 = OpConstant %4 0
%14 = OpTypePointer StorageBuffer %4
%18 = OpTypePointer StorageBuffer %5
)";
  auto got_types = DumpInstructions(b.types());
  EXPECT_EQ(expected_types, got_types);

  auto* expected_instructions = R"(%15 = OpAccessChain %14 %1 %12
%10 = OpAtomicLoad %4 %15 %11 %12
%19 = OpAccessChain %18 %1 %11
%16 = OpAtomicLoad %5 %19 %11 %12
OpReturn
)";
  auto got_instructions = DumpInstructions(b.functions()[0].instructions());
  EXPECT_EQ(expected_instructions, got_instructions);

  Validate(b);
}

TEST_F(IntrinsicBuilderTest, Call_AtomicStore) {
  // [[block]] struct S {
  //   u : atomic<u32>;
  //   i : atomic<i32>;
  // }
  //
  // [[binding(1), group(2)]] var<storage, read_write> b : S;
  //
  // fn a_func() {
  //   var u = 1u;
  //   var i = 2;
  //   atomicStore(&b.u, u);
  //   atomicStore(&b.i, i);
  // }
  auto* s = Structure("S",
                      {
                          Member("u", ty.atomic<u32>()),
                          Member("i", ty.atomic<i32>()),
                      },
                      {create<ast::StructBlockDecoration>()});
  Global("b", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kReadWrite,
         ast::DecorationList{
             create<ast::BindingDecoration>(1),
             create<ast::GroupDecoration>(2),
         });

  Func("a_func", {}, ty.void_(),
       ast::StatementList{
           Decl(Var("u", nullptr, Expr(1u))),
           Decl(Var("i", nullptr, Expr(2))),
           CallStmt(
               Call("atomicStore", AddressOf(MemberAccessor("b", "u")), "u")),
           CallStmt(
               Call("atomicStore", AddressOf(MemberAccessor("b", "i")), "i")),
       },
       ast::DecorationList{Stage(ast::PipelineStage::kFragment)});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.Build()) << b.error();

  ASSERT_EQ(b.functions().size(), 1u);

  auto* expected_types = R"(%4 = OpTypeInt 32 0
%5 = OpTypeInt 32 1
%3 = OpTypeStruct %4 %5
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%7 = OpTypeVoid
%6 = OpTypeFunction %7
%10 = OpConstant %4 1
%12 = OpTypePointer Function %4
%13 = OpConstantNull %4
%14 = OpConstant %5 2
%16 = OpTypePointer Function %5
%17 = OpConstantNull %5
%19 = OpConstant %4 0
%21 = OpTypePointer StorageBuffer %4
%26 = OpTypePointer StorageBuffer %5
)";
  auto got_types = DumpInstructions(b.types());
  EXPECT_EQ(expected_types, got_types);

  auto* expected_instructions = R"(OpStore %11 %10
OpStore %15 %14
%22 = OpAccessChain %21 %1 %19
%23 = OpLoad %4 %11
OpAtomicStore %22 %10 %19 %23
%27 = OpAccessChain %26 %1 %10
%28 = OpLoad %5 %15
OpAtomicStore %27 %10 %19 %28
OpReturn
)";
  auto got_instructions = DumpInstructions(b.functions()[0].instructions());
  EXPECT_EQ(expected_instructions, got_instructions);

  Validate(b);
}

using Intrinsic_Builtin_AtomicRMW_i32 =
    IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_AtomicRMW_i32, Test) {
  // [[block]] struct S {
  //   v : atomic<i32>;
  // }
  //
  // [[binding(1), group(2)]] var<storage, read_write> b : S;
  //
  // fn a_func() {
  //   var v = 10;
  //   let x : i32 = atomicOP(&b.v, v);
  // }
  auto* s = Structure("S",
                      {
                          Member("v", ty.atomic<i32>()),
                      },
                      {create<ast::StructBlockDecoration>()});
  Global("b", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kReadWrite,
         ast::DecorationList{
             create<ast::BindingDecoration>(1),
             create<ast::GroupDecoration>(2),
         });

  Func("a_func", {}, ty.void_(),
       ast::StatementList{
           Decl(Var("v", nullptr, Expr(10))),
           Decl(Const("x", ty.i32(),
                      Call(GetParam().name, AddressOf(MemberAccessor("b", "v")),
                           "v"))),
       },
       ast::DecorationList{Stage(ast::PipelineStage::kFragment)});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.Build()) << b.error();

  ASSERT_EQ(b.functions().size(), 1u);

  std::string expected_types = R"(%4 = OpTypeInt 32 1
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%6 = OpTypeVoid
%5 = OpTypeFunction %6
%9 = OpConstant %4 10
%11 = OpTypePointer Function %4
%12 = OpConstantNull %4
%14 = OpTypeInt 32 0
%15 = OpConstant %14 1
%16 = OpConstant %14 0
%18 = OpTypePointer StorageBuffer %4
)";
  auto got_types = DumpInstructions(b.types());
  EXPECT_EQ(expected_types, got_types);

  std::string expected_instructions = R"(OpStore %10 %9
%19 = OpAccessChain %18 %1 %16
%20 = OpLoad %4 %10
)";
  expected_instructions += "%13 = " + GetParam().op + " %4 %19 %15 %16 %20\n";
  expected_instructions += "OpReturn\n";

  auto got_instructions = DumpInstructions(b.functions()[0].instructions());
  EXPECT_EQ(expected_instructions, got_instructions);

  Validate(b);
}
INSTANTIATE_TEST_SUITE_P(
    IntrinsicBuilderTest,
    Intrinsic_Builtin_AtomicRMW_i32,
    testing::Values(IntrinsicData{"atomicAdd", "OpAtomicIAdd"},
                    IntrinsicData{"atomicMax", "OpAtomicSMax"},
                    IntrinsicData{"atomicMin", "OpAtomicSMin"},
                    IntrinsicData{"atomicAnd", "OpAtomicAnd"},
                    IntrinsicData{"atomicOr", "OpAtomicOr"},
                    IntrinsicData{"atomicXor", "OpAtomicXor"}));

using Intrinsic_Builtin_AtomicRMW_u32 =
    IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_AtomicRMW_u32, Test) {
  // [[block]] struct S {
  //   v : atomic<u32>;
  // }
  //
  // [[binding(1), group(2)]] var<storage, read_write> b : S;
  //
  // fn a_func() {
  //   var v = 10u;
  //   let x : u32 = atomicOP(&b.v, v);
  // }
  auto* s = Structure("S",
                      {
                          Member("v", ty.atomic<u32>()),
                      },
                      {create<ast::StructBlockDecoration>()});
  Global("b", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kReadWrite,
         ast::DecorationList{
             create<ast::BindingDecoration>(1),
             create<ast::GroupDecoration>(2),
         });

  Func("a_func", {}, ty.void_(),
       ast::StatementList{
           Decl(Var("v", nullptr, Expr(10u))),
           Decl(Const("x", ty.u32(),
                      Call(GetParam().name, AddressOf(MemberAccessor("b", "v")),
                           "v"))),
       },
       ast::DecorationList{Stage(ast::PipelineStage::kFragment)});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.Build()) << b.error();

  ASSERT_EQ(b.functions().size(), 1u);

  std::string expected_types = R"(%4 = OpTypeInt 32 0
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%6 = OpTypeVoid
%5 = OpTypeFunction %6
%9 = OpConstant %4 10
%11 = OpTypePointer Function %4
%12 = OpConstantNull %4
%14 = OpConstant %4 1
%15 = OpConstant %4 0
%17 = OpTypePointer StorageBuffer %4
)";
  auto got_types = DumpInstructions(b.types());
  EXPECT_EQ(expected_types, got_types);

  std::string expected_instructions = R"(OpStore %10 %9
%18 = OpAccessChain %17 %1 %15
%19 = OpLoad %4 %10
)";
  expected_instructions += "%13 = " + GetParam().op + " %4 %18 %14 %15 %19\n";
  expected_instructions += "OpReturn\n";

  auto got_instructions = DumpInstructions(b.functions()[0].instructions());
  EXPECT_EQ(expected_instructions, got_instructions);

  Validate(b);
}
INSTANTIATE_TEST_SUITE_P(
    IntrinsicBuilderTest,
    Intrinsic_Builtin_AtomicRMW_u32,
    testing::Values(IntrinsicData{"atomicAdd", "OpAtomicIAdd"},
                    IntrinsicData{"atomicMax", "OpAtomicUMax"},
                    IntrinsicData{"atomicMin", "OpAtomicUMin"},
                    IntrinsicData{"atomicAnd", "OpAtomicAnd"},
                    IntrinsicData{"atomicOr", "OpAtomicOr"},
                    IntrinsicData{"atomicXor", "OpAtomicXor"}));

TEST_F(IntrinsicBuilderTest, Call_AtomicExchange) {
  // [[block]] struct S {
  //   u : atomic<u32>;
  //   i : atomic<i32>;
  // }
  //
  // [[binding(1), group(2)]] var<storage, read_write> b : S;
  //
  // fn a_func() {
  //   var u = 10u;
  //   var i = 10;
  //   let r : u32 = atomicExchange(&b.u, u);
  //   let s : i32 = atomicExchange(&b.i, i);
  // }
  auto* s = Structure("S",
                      {
                          Member("u", ty.atomic<u32>()),
                          Member("i", ty.atomic<i32>()),
                      },
                      {create<ast::StructBlockDecoration>()});
  Global("b", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kReadWrite,
         ast::DecorationList{
             create<ast::BindingDecoration>(1),
             create<ast::GroupDecoration>(2),
         });

  Func("a_func", {}, ty.void_(),
       ast::StatementList{
           Decl(Var("u", nullptr, Expr(10u))),
           Decl(Var("i", nullptr, Expr(10))),
           Decl(Const("r", ty.u32(),
                      Call("atomicExchange",
                           AddressOf(MemberAccessor("b", "u")), "u"))),
           Decl(Const("s", ty.i32(),
                      Call("atomicExchange",
                           AddressOf(MemberAccessor("b", "i")), "i"))),
       },
       ast::DecorationList{Stage(ast::PipelineStage::kFragment)});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.Build()) << b.error();

  ASSERT_EQ(b.functions().size(), 1u);

  auto* expected_types = R"(%4 = OpTypeInt 32 0
%5 = OpTypeInt 32 1
%3 = OpTypeStruct %4 %5
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%7 = OpTypeVoid
%6 = OpTypeFunction %7
%10 = OpConstant %4 10
%12 = OpTypePointer Function %4
%13 = OpConstantNull %4
%14 = OpConstant %5 10
%16 = OpTypePointer Function %5
%17 = OpConstantNull %5
%19 = OpConstant %4 1
%20 = OpConstant %4 0
%22 = OpTypePointer StorageBuffer %4
%27 = OpTypePointer StorageBuffer %5
)";
  auto got_types = DumpInstructions(b.types());
  EXPECT_EQ(expected_types, got_types);

  auto* expected_instructions = R"(OpStore %11 %10
OpStore %15 %14
%23 = OpAccessChain %22 %1 %20
%24 = OpLoad %4 %11
%18 = OpAtomicExchange %4 %23 %19 %20 %24
%28 = OpAccessChain %27 %1 %19
%29 = OpLoad %5 %15
%25 = OpAtomicExchange %5 %28 %19 %20 %29
OpReturn
)";
  auto got_instructions = DumpInstructions(b.functions()[0].instructions());
  EXPECT_EQ(expected_instructions, got_instructions);

  Validate(b);
}

TEST_F(IntrinsicBuilderTest, Call_AtomicCompareExchangeWeak) {
  // [[block]] struct S {
  //   u : atomic<u32>;
  //   i : atomic<i32>;
  // }
  //
  // [[binding(1), group(2)]] var<storage, read_write> b : S;
  //
  // fn a_func() {
  //   let u : vec2<u32> = atomicCompareExchangeWeak(&b.u, 10u);
  //   let i : vec2<i32> = atomicCompareExchangeWeak(&b.i, 10);
  // }
  auto* s = Structure("S",
                      {
                          Member("u", ty.atomic<u32>()),
                          Member("i", ty.atomic<i32>()),
                      },
                      {create<ast::StructBlockDecoration>()});
  Global("b", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kReadWrite,
         ast::DecorationList{
             create<ast::BindingDecoration>(1),
             create<ast::GroupDecoration>(2),
         });

  Func("a_func", {}, ty.void_(),
       ast::StatementList{
           Decl(Const("u", ty.vec2<u32>(),
                      Call("atomicCompareExchangeWeak",
                           AddressOf(MemberAccessor("b", "u")), 10u, 20u))),
           Decl(Const("i", ty.vec2<i32>(),
                      Call("atomicCompareExchangeWeak",
                           AddressOf(MemberAccessor("b", "i")), 10, 20))),
       },
       ast::DecorationList{Stage(ast::PipelineStage::kFragment)});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.Build()) << b.error();

  ASSERT_EQ(b.functions().size(), 1u);

  auto* expected_types = R"(%4 = OpTypeInt 32 0
%5 = OpTypeInt 32 1
%3 = OpTypeStruct %4 %5
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%7 = OpTypeVoid
%6 = OpTypeFunction %7
%11 = OpTypeVector %4 2
%12 = OpConstant %4 1
%13 = OpConstant %4 0
%15 = OpTypePointer StorageBuffer %4
%17 = OpConstant %4 20
%18 = OpConstant %4 10
%19 = OpTypeBool
%24 = OpTypeVector %5 2
%26 = OpTypePointer StorageBuffer %5
%28 = OpConstant %5 20
%29 = OpConstant %5 10
%32 = OpConstant %5 0
%33 = OpConstant %5 1
)";
  auto got_types = DumpInstructions(b.types());
  EXPECT_EQ(expected_types, got_types);

  auto* expected_instructions = R"(%16 = OpAccessChain %15 %1 %13
%20 = OpAtomicCompareExchange %4 %16 %12 %13 %13 %17 %18
%21 = OpIEqual %19 %20 %17
%22 = OpSelect %4 %21 %12 %13
%10 = OpCompositeConstruct %11 %20 %22
%27 = OpAccessChain %26 %1 %12
%30 = OpAtomicCompareExchange %5 %27 %12 %13 %13 %28 %29
%31 = OpIEqual %19 %30 %28
%34 = OpSelect %5 %31 %33 %32
%23 = OpCompositeConstruct %24 %30 %34
OpReturn
)";
  auto got_instructions = DumpInstructions(b.functions()[0].instructions());
  EXPECT_EQ(expected_instructions, got_instructions);

  Validate(b);
}

using Intrinsic_Builtin_DataPacking_Test =
    IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_DataPacking_Test, Binary) {
  auto param = GetParam();

  bool pack4 = param.name == "pack4x8snorm" || param.name == "pack4x8unorm";
  auto* call = pack4 ? Call(param.name, vec4<float>(1.0f, 1.0f, 1.0f, 1.0f))
                     : Call(param.name, vec2<float>(1.0f, 1.0f));
  auto* func = Func("a_func", {}, ty.void_(), {CallStmt(call)});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  if (pack4) {
    EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 0
%9 = OpTypeFloat 32
%8 = OpTypeVector %9 4
%10 = OpConstant %9 1
%11 = OpConstantComposite %8 %10 %10 %10 %10
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 )" + param.op +
                                  R"( %11
OpReturn
OpFunctionEnd
)");
  } else {
    EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 0
%9 = OpTypeFloat 32
%8 = OpTypeVector %9 2
%10 = OpConstant %9 1
%11 = OpConstantComposite %8 %10 %10
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 )" + param.op +
                                  R"( %11
OpReturn
OpFunctionEnd
)");
  }
}

INSTANTIATE_TEST_SUITE_P(
    IntrinsicBuilderTest,
    Intrinsic_Builtin_DataPacking_Test,
    testing::Values(IntrinsicData{"pack4x8snorm", "PackSnorm4x8"},
                    IntrinsicData{"pack4x8unorm", "PackUnorm4x8"},
                    IntrinsicData{"pack2x16snorm", "PackSnorm2x16"},
                    IntrinsicData{"pack2x16unorm", "PackUnorm2x16"},
                    IntrinsicData{"pack2x16float", "PackHalf2x16"}));

using Intrinsic_Builtin_DataUnpacking_Test =
    IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_DataUnpacking_Test, Binary) {
  auto param = GetParam();

  bool pack4 = param.name == "unpack4x8snorm" || param.name == "unpack4x8unorm";
  auto* func = Func("a_func", {}, ty.void_(), {CallStmt(Call(param.name, 1u))});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  if (pack4) {
    EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeFloat 32
%6 = OpTypeVector %7 4
%9 = OpTypeInt 32 0
%10 = OpConstant %9 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %8 )" + param.op +
                                  R"( %10
OpReturn
OpFunctionEnd
)");
  } else {
    EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeFloat 32
%6 = OpTypeVector %7 2
%9 = OpTypeInt 32 0
%10 = OpConstant %9 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %8 )" + param.op +
                                  R"( %10
OpReturn
OpFunctionEnd
)");
  }
}

INSTANTIATE_TEST_SUITE_P(
    IntrinsicBuilderTest,
    Intrinsic_Builtin_DataUnpacking_Test,
    testing::Values(IntrinsicData{"unpack4x8snorm", "UnpackSnorm4x8"},
                    IntrinsicData{"unpack4x8unorm", "UnpackUnorm4x8"},
                    IntrinsicData{"unpack2x16snorm", "UnpackSnorm2x16"},
                    IntrinsicData{"unpack2x16unorm", "UnpackUnorm2x16"},
                    IntrinsicData{"unpack2x16float", "UnpackHalf2x16"}));

TEST_F(IntrinsicBuilderTest, Call_WorkgroupBarrier) {
  Func("f", {}, ty.void_(),
       ast::StatementList{
           CallStmt(Call("workgroupBarrier")),
       },
       ast::DecorationList{
           Stage(ast::PipelineStage::kCompute),
           WorkgroupSize(1),
       });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.Build()) << b.error();

  ASSERT_EQ(b.functions().size(), 1u);

  auto* expected_types = R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 0
%7 = OpConstant %6 2
%8 = OpConstant %6 264
)";
  auto got_types = DumpInstructions(b.types());
  EXPECT_EQ(expected_types, got_types);

  auto* expected_instructions = R"(OpControlBarrier %7 %7 %8
OpReturn
)";
  auto got_instructions = DumpInstructions(b.functions()[0].instructions());
  EXPECT_EQ(expected_instructions, got_instructions);

  Validate(b);
}

TEST_F(IntrinsicBuilderTest, Call_StorageBarrier) {
  Func("f", {}, ty.void_(),
       ast::StatementList{
           CallStmt(Call("storageBarrier")),
       },
       ast::DecorationList{
           Stage(ast::PipelineStage::kCompute),
           WorkgroupSize(1),
       });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.Build()) << b.error();

  ASSERT_EQ(b.functions().size(), 1u);

  auto* expected_types = R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 0
%7 = OpConstant %6 2
%8 = OpConstant %6 72
)";
  auto got_types = DumpInstructions(b.types());
  EXPECT_EQ(expected_types, got_types);

  auto* expected_instructions = R"(OpControlBarrier %7 %7 %8
OpReturn
)";
  auto got_instructions = DumpInstructions(b.functions()[0].instructions());
  EXPECT_EQ(expected_instructions, got_instructions);

  Validate(b);
}

TEST_F(IntrinsicBuilderTest, Call_Ignore) {
  Func("f", {Param("a", ty.i32()), Param("b", ty.i32()), Param("c", ty.i32())},
       ty.i32(), {Return(Mul(Add("a", "b"), "c"))});

  Func("main", {}, ty.void_(),
       {
           CallStmt(Call("ignore", Call("f", 1, 2, 3))),
       },
       {
           Stage(ast::PipelineStage::kCompute),
           WorkgroupSize(1),
       });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.Build()) << b.error();

  ASSERT_EQ(b.functions().size(), 2u);

  auto* expected_types = R"(%2 = OpTypeInt 32 1
%1 = OpTypeFunction %2 %2 %2 %2
%11 = OpTypeVoid
%10 = OpTypeFunction %11
%16 = OpConstant %2 1
%17 = OpConstant %2 2
%18 = OpConstant %2 3
)";
  auto got_types = DumpInstructions(b.types());
  EXPECT_EQ(expected_types, got_types);

  auto* expected_instructions = R"(%15 = OpFunctionCall %2 %3 %16 %17 %18
OpReturn
)";
  auto got_instructions = DumpInstructions(b.functions()[1].instructions());
  EXPECT_EQ(expected_instructions, got_instructions);

  Validate(b);
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
