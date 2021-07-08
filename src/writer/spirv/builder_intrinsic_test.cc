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
TEST_P(IntrinsicBoolTest, Call_Bool) {
  auto param = GetParam();

  auto* var = Global("v", ty.vec3<bool>(), ast::StorageClass::kPrivate);

  auto* expr = Call(param.name, "v");
  WrapInFunction(expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 6u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeBool
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%6 = )" + param.op +
                " %4 %7\n");
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
  WrapInFunction(expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%6 = OpTypeBool
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%5 = )" + param.op +
                " %6 %7\n");
}

TEST_P(IntrinsicFloatTest, Call_Float_Vector) {
  auto param = GetParam();

  auto* var = Global("v", ty.vec3<f32>(), ast::StorageClass::kPrivate);

  auto* expr = Call(param.name, "v");
  WrapInFunction(expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 6u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeBool
%7 = OpTypeVector %8 3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%9 = OpLoad %3 %1
%6 = )" + param.op +
                " %7 %9\n");
}
INSTANTIATE_TEST_SUITE_P(IntrinsicBuilderTest,
                         IntrinsicFloatTest,
                         testing::Values(IntrinsicData{"isNan", "OpIsNan"},
                                         IntrinsicData{"isInf", "OpIsInf"}));

TEST_F(IntrinsicBuilderTest, IsFinite_Scalar) {
  auto* var = Global("v", ty.f32(), ast::StorageClass::kPrivate);

  auto* expr = Call("isFinite", "v");
  WrapInFunction(expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%6 = OpTypeBool
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%8 = OpIsInf %6 %7
%9 = OpIsNan %6 %7
%10 = OpLogicalOr %6 %8 %9
%5 = OpLogicalNot %6 %10
)");
}

TEST_F(IntrinsicBuilderTest, IsFinite_Vector) {
  auto* var = Global("v", ty.vec3<f32>(), ast::StorageClass::kPrivate);

  auto* expr = Call("isFinite", "v");
  WrapInFunction(expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 6u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeBool
%7 = OpTypeVector %8 3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%9 = OpLoad %3 %1
%10 = OpIsInf %7 %9
%11 = OpIsNan %7 %9
%12 = OpLogicalOr %7 %10 %11
%6 = OpLogicalNot %7 %12
)");
}

TEST_F(IntrinsicBuilderTest, IsNormal_Scalar) {
  auto* var = Global("v", ty.f32(), ast::StorageClass::kPrivate);

  auto* expr = Call("isNormal", "v");
  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 9u) << b.error();
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
  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 10u) << b.error();
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
  WrapInFunction(expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%6 = OpLoad %3 %1
%5 = )" + param.op +
                " %3 %6\n");
}

TEST_P(IntrinsicIntTest, Call_SInt_Vector) {
  auto param = GetParam();

  auto* var = Global("v", ty.vec3<i32>(), ast::StorageClass::kPrivate);

  auto* expr = Call(param.name, "v");
  WrapInFunction(expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 6u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 1
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%6 = )" + param.op +
                " %3 %7\n");
}

TEST_P(IntrinsicIntTest, Call_UInt_Scalar) {
  auto param = GetParam();

  auto* var = Global("v", ty.u32(), ast::StorageClass::kPrivate);

  auto* expr = Call(param.name, "v");
  WrapInFunction(expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeInt 32 0
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%6 = OpLoad %3 %1
%5 = )" + param.op +
                " %3 %6\n");
}

TEST_P(IntrinsicIntTest, Call_UInt_Vector) {
  auto param = GetParam();

  auto* var = Global("v", ty.vec3<u32>(), ast::StorageClass::kPrivate);

  auto* expr = Call(param.name, "v");
  WrapInFunction(expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 6u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 0
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%6 = )" + param.op +
                " %3 %7\n");
}
INSTANTIATE_TEST_SUITE_P(
    IntrinsicBuilderTest,
    IntrinsicIntTest,
    testing::Values(IntrinsicData{"countOneBits", "OpBitCount"},
                    IntrinsicData{"reverseBits", "OpBitReverse"}));

TEST_F(IntrinsicBuilderTest, Call_Dot) {
  auto* var = Global("v", ty.vec3<f32>(), ast::StorageClass::kPrivate);

  auto* expr = Call("dot", "v", "v");
  WrapInFunction(expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 6u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%8 = OpLoad %3 %1
%6 = OpDot %4 %7 %8
)");
}

using IntrinsicDeriveTest = IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(IntrinsicDeriveTest, Call_Derivative_Scalar) {
  auto param = GetParam();

  auto* var = Global("v", ty.f32(), ast::StorageClass::kPrivate);

  auto* expr = Call(param.name, "v");
  Func("func", {}, ty.void_(), {Ignore(expr)},
       {create<ast::StageDecoration>(ast::PipelineStage::kFragment)});

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%6 = OpLoad %3 %1
%5 = )" + param.op +
                " %3 %6\n");
}

TEST_P(IntrinsicDeriveTest, Call_Derivative_Vector) {
  auto param = GetParam();

  auto* var = Global("v", ty.vec3<f32>(), ast::StorageClass::kPrivate);

  auto* expr = Call(param.name, "v");
  Func("func", {}, ty.void_(), {Ignore(expr)},
       {create<ast::StageDecoration>(ast::PipelineStage::kFragment)});

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 6u) << b.error();

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
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%6 = )" + param.op +
                " %3 %7\n");
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
  WrapInFunction(expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(v3)) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(bool_v3)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 11u) << b.error();

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
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%12 = OpLoad %8 %6
%13 = OpLoad %3 %1
%14 = OpLoad %3 %1
%11 = OpSelect %3 %12 %13 %14
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

  Func("f1", {}, ty.void_(), {Ignore(expr1)}, {});
  Func("f2", {}, ty.void_(), {Ignore(expr2)}, {});

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
  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 9u) << b.error();
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
  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
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
  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
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

  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
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
  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
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
  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
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

  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
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

  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
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

  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
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

  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
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

  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
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

  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
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
  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
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

  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
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

  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
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
  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
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
  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
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

using Intrinsic_Builtin_SingleParam_Uint_Test =
    IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_SingleParam_Uint_Test, Call_Scalar) {
  auto param = GetParam();

  auto* expr = Call(param.name, 1u);
  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 0
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 )" + param.op +
                                R"( %8
OpReturn
OpFunctionEnd
)");
}

TEST_P(Intrinsic_Builtin_SingleParam_Uint_Test, Call_Vector) {
  auto param = GetParam();

  auto* expr = Call(param.name, vec2<u32>(1u, 1u));
  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
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
                                R"( %10
OpReturn
OpFunctionEnd
)");
}
INSTANTIATE_TEST_SUITE_P(IntrinsicBuilderTest,
                         Intrinsic_Builtin_SingleParam_Uint_Test,
                         testing::Values(IntrinsicData{"abs", "SAbs"}));

using Intrinsic_Builtin_DualParam_SInt_Test =
    IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_DualParam_SInt_Test, Call_Scalar) {
  auto param = GetParam();

  auto* expr = Call(param.name, 1, 1);
  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
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
  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
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
  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
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
  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
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
  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
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

  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
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
  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
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

  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(expr), 5u) << b.error();
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
  auto* out = Var("out", ty.vec2<f32>());
  auto* expr = Call("modf", vec2<f32>(1.0f, 2.0f), AddressOf("out"));
  Func("a_func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Decl(out),
           Ignore(expr),
       },
       ast::DecorationList{
           Stage(ast::PipelineStage::kFragment),
       });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.Build()) << b.error();
  auto got = DumpBuilder(b);
  auto* expect = R"(OpCapability Shader
%12 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %3 "a_func"
OpExecutionMode %3 OriginUpperLeft
OpName %3 "a_func"
OpName %5 "out"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%8 = OpTypeFloat 32
%7 = OpTypeVector %8 2
%6 = OpTypePointer Function %7
%9 = OpConstantNull %7
%13 = OpConstant %8 1
%14 = OpConstant %8 2
%15 = OpConstantComposite %7 %13 %14
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpVariable %6 Function %9
%11 = OpExtInst %7 %12 Modf %15 %5
OpReturn
OpFunctionEnd
)";
  EXPECT_EQ(expect, got);

  Validate(b);
}

TEST_F(IntrinsicBuilderTest, Call_Frexp) {
  auto* out = Var("out", ty.vec2<i32>());
  auto* expr = Call("frexp", vec2<f32>(1.0f, 2.0f), AddressOf("out"));
  Func("a_func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Decl(out),
           Ignore(expr),
       },
       ast::DecorationList{
           Stage(ast::PipelineStage::kFragment),
       });

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.Build()) << b.error();
  auto got = DumpBuilder(b);
  auto* expect = R"(OpCapability Shader
%14 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %3 "a_func"
OpExecutionMode %3 OriginUpperLeft
OpName %3 "a_func"
OpName %5 "out"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%8 = OpTypeInt 32 1
%7 = OpTypeVector %8 2
%6 = OpTypePointer Function %7
%9 = OpConstantNull %7
%13 = OpTypeFloat 32
%12 = OpTypeVector %13 2
%15 = OpConstant %13 1
%16 = OpConstant %13 2
%17 = OpConstantComposite %12 %15 %16
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpVariable %6 Function %9
%11 = OpExtInst %12 %14 Frexp %17 %5
OpReturn
OpFunctionEnd
)";
  EXPECT_EQ(expect, got);

  Validate(b);
}

TEST_F(IntrinsicBuilderTest, Call_Determinant) {
  auto* var = Global("var", ty.mat3x3<f32>(), ast::StorageClass::kPrivate);

  auto* expr = Call("determinant", "var");
  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  EXPECT_EQ(b.GenerateCallExpression(expr), 11u) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(%12 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %5 "var"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%9 = OpTypeFloat 32
%8 = OpTypeVector %9 3
%7 = OpTypeMatrix %8 3
%6 = OpTypePointer Private %7
%10 = OpConstantNull %7
%5 = OpVariable %6 Private %10
%3 = OpFunction %2 None %1
%4 = OpLabel
%13 = OpLoad %7 %5
%11 = OpExtInst %9 %12 Determinant %13
OpReturn
OpFunctionEnd
)");
}

TEST_F(IntrinsicBuilderTest, Call_Transpose) {
  auto* var = Global("var", ty.mat2x3<f32>(), ast::StorageClass::kPrivate);

  auto* expr = Call("transpose", "var");
  WrapInFunction(expr);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  EXPECT_EQ(b.GenerateCallExpression(expr), 11u) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(OpName %3 "a_func"
OpName %5 "var"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%9 = OpTypeFloat 32
%8 = OpTypeVector %9 3
%7 = OpTypeMatrix %8 2
%6 = OpTypePointer Private %7
%10 = OpConstantNull %7
%5 = OpVariable %6 Private %10
%13 = OpTypeVector %9 2
%12 = OpTypeMatrix %13 3
%3 = OpFunction %2 None %1
%4 = OpLabel
%14 = OpLoad %7 %5
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

  Func("a_func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Ignore(expr),
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
%12 = OpTypeInt 32 0
)";
  auto got_types = DumpInstructions(b.types());
  EXPECT_EQ(expected_types, got_types);

  auto* expected_instructions = R"(%11 = OpArrayLength %12 %1 0
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

  Func("a_func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Ignore(expr),
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
%12 = OpTypeInt 32 0
)";
  auto got_types = DumpInstructions(b.types());
  EXPECT_EQ(expected_types, got_types);

  auto* expected_instructions = R"(%11 = OpArrayLength %12 %1 1
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

  Func("a_func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Decl(p),
           Decl(p2),
           Ignore(expr),
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
%12 = OpTypeInt 32 0
)";
  auto got_types = DumpInstructions(b.types());
  EXPECT_EQ(expected_types, got_types);

  auto* expected_instructions = R"(%11 = OpArrayLength %12 %1 0
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

  Func("a_func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Decl(p),
           Decl(p2),
           Decl(p3),
           Ignore(expr),
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
%12 = OpTypeInt 32 0
)";
  auto got_types = DumpInstructions(b.types());
  EXPECT_EQ(expected_types, got_types);

  auto* expected_instructions = R"(%11 = OpArrayLength %12 %1 0
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

  Func("a_func", ast::VariableList{}, ty.void_(),
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

  Func("a_func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Decl(Var("u", nullptr, Expr(1u))),
           Decl(Var("i", nullptr, Expr(2))),
           create<ast::CallStatement>(
               Call("atomicStore", AddressOf(MemberAccessor("b", "u")), "u")),
           create<ast::CallStatement>(
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

  Func("a_func", ast::VariableList{}, ty.void_(),
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

  Func("a_func", ast::VariableList{}, ty.void_(),
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

  Func("a_func", ast::VariableList{}, ty.void_(),
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

  Func("a_func", ast::VariableList{}, ty.void_(),
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
  WrapInFunction(call);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(call), 5u) << b.error();
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
  auto* call = Call(param.name, 1u);
  WrapInFunction(call);

  auto* func = Func("a_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{}, ast::DecorationList{});

  spirv::Builder& b = Build();

  ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(call), 5u) << b.error();
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
  Func("f", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::CallStatement>(Call("workgroupBarrier")),
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
)";
  auto got_instructions = DumpInstructions(b.functions()[0].instructions());
  EXPECT_EQ(expected_instructions, got_instructions);

  Validate(b);
}

TEST_F(IntrinsicBuilderTest, Call_StorageBarrier) {
  Func("f", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::CallStatement>(Call("storageBarrier")),
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
%8 = OpConstant %6 1
%9 = OpConstant %6 72
)";
  auto got_types = DumpInstructions(b.types());
  EXPECT_EQ(expected_types, got_types);

  auto* expected_instructions = R"(OpControlBarrier %7 %8 %9
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
           create<ast::CallStatement>(Call("ignore", Call("f", 1, 2, 3))),
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
)";
  auto got_instructions = DumpInstructions(b.functions()[1].instructions());
  EXPECT_EQ(expected_instructions, got_instructions);

  Validate(b);
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
