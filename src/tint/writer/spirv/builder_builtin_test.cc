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

#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/sem/depth_texture.h"
#include "src/tint/utils/string.h"
#include "src/tint/writer/spirv/spv_dump.h"
#include "src/tint/writer/spirv/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::spirv {
namespace {

using BuiltinBuilderTest = TestHelper;

template <typename T>
using BuiltinBuilderTestWithParam = TestParamHelper<T>;

struct BuiltinData {
    std::string name;
    std::string op;
};
inline std::ostream& operator<<(std::ostream& out, BuiltinData data) {
    out << data.name;
    return out;
}

using BuiltinBoolTest = BuiltinBuilderTestWithParam<BuiltinData>;
TEST_P(BuiltinBoolTest, Call_Bool_Scalar) {
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
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), "%10 = OpLoad %3 %1\nOpReturn\n");
}

TEST_P(BuiltinBoolTest, Call_Bool_Vector) {
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
INSTANTIATE_TEST_SUITE_P(BuiltinBuilderTest,
                         BuiltinBoolTest,
                         testing::Values(BuiltinData{"any", "OpAny"}, BuiltinData{"all", "OpAll"}));

using BuiltinIntTest = BuiltinBuilderTestWithParam<BuiltinData>;
TEST_P(BuiltinIntTest, Call_SInt_Scalar) {
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

TEST_P(BuiltinIntTest, Call_SInt_Vector) {
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

TEST_P(BuiltinIntTest, Call_UInt_Scalar) {
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

TEST_P(BuiltinIntTest, Call_UInt_Vector) {
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
INSTANTIATE_TEST_SUITE_P(BuiltinBuilderTest,
                         BuiltinIntTest,
                         testing::Values(BuiltinData{"countOneBits", "OpBitCount"},
                                         BuiltinData{"reverseBits", "OpBitReverse"}));

TEST_F(BuiltinBuilderTest, Call_Dot_F32) {
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

TEST_F(BuiltinBuilderTest, Call_Dot_U32) {
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

TEST_F(BuiltinBuilderTest, Call_Dot_I32) {
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

using BuiltinDeriveTest = BuiltinBuilderTestWithParam<BuiltinData>;
TEST_P(BuiltinDeriveTest, Call_Derivative_Scalar) {
    auto param = GetParam();
    auto* var = Global("v", ty.f32(), ast::StorageClass::kPrivate);
    auto* expr = Call(param.name, "v");
    auto* func =
        Func("func", {}, ty.void_(), {CallStmt(expr)}, {Stage(ast::PipelineStage::kFragment)});

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

TEST_P(BuiltinDeriveTest, Call_Derivative_Vector) {
    auto param = GetParam();
    auto* var = Global("v", ty.vec3<f32>(), ast::StorageClass::kPrivate);
    auto* expr = Call(param.name, "v");
    auto* func =
        Func("func", {}, ty.void_(), {CallStmt(expr)}, {Stage(ast::PipelineStage::kFragment)});

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
INSTANTIATE_TEST_SUITE_P(BuiltinBuilderTest,
                         BuiltinDeriveTest,
                         testing::Values(BuiltinData{"dpdx", "OpDPdx"},
                                         BuiltinData{"dpdxFine", "OpDPdxFine"},
                                         BuiltinData{"dpdxCoarse", "OpDPdxCoarse"},
                                         BuiltinData{"dpdy", "OpDPdy"},
                                         BuiltinData{"dpdyFine", "OpDPdyFine"},
                                         BuiltinData{"dpdyCoarse", "OpDPdyCoarse"},
                                         BuiltinData{"fwidth", "OpFwidth"},
                                         BuiltinData{"fwidthFine", "OpFwidthFine"},
                                         BuiltinData{"fwidthCoarse", "OpFwidthCoarse"}));

TEST_F(BuiltinBuilderTest, Call_Select) {
    auto* v3 = Global("v3", ty.vec3<f32>(), ast::StorageClass::kPrivate);

    auto* bool_v3 = Global("bool_v3", ty.vec3<bool>(), ast::StorageClass::kPrivate);
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
TEST_F(BuiltinBuilderTest, Call_TextureSampleCompare_Twice) {
    auto* s = ty.sampler(ast::SamplerKind::kComparisonSampler);
    auto* t = ty.depth_texture(ast::TextureDimension::k2d);

    auto* tex = Global("texture", t,
                       ast::AttributeList{
                           create<ast::BindingAttribute>(0),
                           create<ast::GroupAttribute>(0),
                       });

    auto* sampler = Global("sampler", s,
                           ast::AttributeList{
                               create<ast::BindingAttribute>(1),
                               create<ast::GroupAttribute>(0),
                           });

    auto* expr1 = Call("textureSampleCompare", "texture", "sampler", vec2<f32>(1.0f, 2.0f), 2.0f);
    auto* expr2 = Call("textureSampleCompare", "texture", "sampler", vec2<f32>(1.0f, 2.0f), 2.0f);

    Func("f1", {}, ty.void_(), {CallStmt(expr1)}, {});
    Func("f2", {}, ty.void_(), {CallStmt(expr2)}, {});

    spirv::Builder& b = Build();

    b.push_function(Function{});

    ASSERT_TRUE(b.GenerateGlobalVariable(tex)) << b.error();
    ASSERT_TRUE(b.GenerateGlobalVariable(sampler)) << b.error();

    EXPECT_EQ(b.GenerateExpression(expr1), 8u) << b.error();
    EXPECT_EQ(b.GenerateExpression(expr2), 17u) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
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

TEST_F(BuiltinBuilderTest, Call_GLSLMethod_WithLoad) {
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

using Builtin_Builtin_SingleParam_Float_Test = BuiltinBuilderTestWithParam<BuiltinData>;
TEST_P(Builtin_Builtin_SingleParam_Float_Test, Call_Scalar) {
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

TEST_P(Builtin_Builtin_SingleParam_Float_Test, Call_Vector) {
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
INSTANTIATE_TEST_SUITE_P(BuiltinBuilderTest,
                         Builtin_Builtin_SingleParam_Float_Test,
                         testing::Values(BuiltinData{"abs", "FAbs"},
                                         BuiltinData{"acos", "Acos"},
                                         BuiltinData{"asin", "Asin"},
                                         BuiltinData{"atan", "Atan"},
                                         BuiltinData{"ceil", "Ceil"},
                                         BuiltinData{"cos", "Cos"},
                                         BuiltinData{"cosh", "Cosh"},
                                         BuiltinData{"degrees", "Degrees"},
                                         BuiltinData{"exp", "Exp"},
                                         BuiltinData{"exp2", "Exp2"},
                                         BuiltinData{"floor", "Floor"},
                                         BuiltinData{"fract", "Fract"},
                                         BuiltinData{"inverseSqrt", "InverseSqrt"},
                                         BuiltinData{"log", "Log"},
                                         BuiltinData{"log2", "Log2"},
                                         BuiltinData{"radians", "Radians"},
                                         BuiltinData{"round", "RoundEven"},
                                         BuiltinData{"sign", "FSign"},
                                         BuiltinData{"sin", "Sin"},
                                         BuiltinData{"sinh", "Sinh"},
                                         BuiltinData{"sqrt", "Sqrt"},
                                         BuiltinData{"tan", "Tan"},
                                         BuiltinData{"tanh", "Tanh"},
                                         BuiltinData{"trunc", "Trunc"}));

TEST_F(BuiltinBuilderTest, Call_Length_Scalar) {
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

TEST_F(BuiltinBuilderTest, Call_Length_Vector) {
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

TEST_F(BuiltinBuilderTest, Call_Normalize) {
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

using Builtin_Builtin_DualParam_Float_Test = BuiltinBuilderTestWithParam<BuiltinData>;
TEST_P(Builtin_Builtin_DualParam_Float_Test, Call_Scalar) {
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

TEST_P(Builtin_Builtin_DualParam_Float_Test, Call_Vector) {
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
INSTANTIATE_TEST_SUITE_P(BuiltinBuilderTest,
                         Builtin_Builtin_DualParam_Float_Test,
                         testing::Values(BuiltinData{"atan2", "Atan2"},
                                         BuiltinData{"max", "NMax"},
                                         BuiltinData{"min", "NMin"},
                                         BuiltinData{"pow", "Pow"},
                                         BuiltinData{"step", "Step"}));

TEST_F(BuiltinBuilderTest, Call_Reflect_Vector) {
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

TEST_F(BuiltinBuilderTest, Call_Distance_Scalar) {
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

TEST_F(BuiltinBuilderTest, Call_Distance_Vector) {
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

TEST_F(BuiltinBuilderTest, Call_Cross) {
    auto* expr = Call("cross", vec3<f32>(1.0f, 1.0f, 1.0f), vec3<f32>(1.0f, 1.0f, 1.0f));
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

using Builtin_Builtin_ThreeParam_Float_Test = BuiltinBuilderTestWithParam<BuiltinData>;
TEST_P(Builtin_Builtin_ThreeParam_Float_Test, Call_Scalar) {
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

TEST_P(Builtin_Builtin_ThreeParam_Float_Test, Call_Vector) {
    auto param = GetParam();
    auto* expr =
        Call(param.name, vec2<f32>(1.0f, 1.0f), vec2<f32>(1.0f, 1.0f), vec2<f32>(1.0f, 1.0f));
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
INSTANTIATE_TEST_SUITE_P(BuiltinBuilderTest,
                         Builtin_Builtin_ThreeParam_Float_Test,
                         testing::Values(BuiltinData{"clamp", "NClamp"},
                                         BuiltinData{"fma", "Fma"},
                                         BuiltinData{"mix", "FMix"},

                                         BuiltinData{"smoothstep", "SmoothStep"}));

TEST_F(BuiltinBuilderTest, Call_FaceForward_Vector) {
    auto* expr =
        Call("faceForward", vec2<f32>(1.0f, 1.0f), vec2<f32>(1.0f, 1.0f), vec2<f32>(1.0f, 1.0f));
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

using Builtin_Builtin_SingleParam_Sint_Test = BuiltinBuilderTestWithParam<BuiltinData>;
TEST_P(Builtin_Builtin_SingleParam_Sint_Test, Call_Scalar) {
    auto param = GetParam();
    auto* expr = Call(param.name, 1_i);
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

TEST_P(Builtin_Builtin_SingleParam_Sint_Test, Call_Vector) {
    auto param = GetParam();
    auto* expr = Call(param.name, vec2<i32>(1_i, 1_i));
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
INSTANTIATE_TEST_SUITE_P(BuiltinBuilderTest,
                         Builtin_Builtin_SingleParam_Sint_Test,
                         testing::Values(BuiltinData{"abs", "SAbs"}));

// Calling abs() on an unsigned integer scalar / vector is a no-op.
using Builtin_Builtin_Abs_Uint_Test = BuiltinBuilderTest;
TEST_F(Builtin_Builtin_Abs_Uint_Test, Call_Scalar) {
    auto* expr = Call("abs", 1_u);
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

TEST_F(Builtin_Builtin_Abs_Uint_Test, Call_Vector) {
    auto* expr = Call("abs", vec2<u32>(1_u, 1_u));
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

using Builtin_Builtin_DualParam_SInt_Test = BuiltinBuilderTestWithParam<BuiltinData>;
TEST_P(Builtin_Builtin_DualParam_SInt_Test, Call_Scalar) {
    auto param = GetParam();
    auto* expr = Call(param.name, 1_i, 1_i);
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

TEST_P(Builtin_Builtin_DualParam_SInt_Test, Call_Vector) {
    auto param = GetParam();
    auto* expr = Call(param.name, vec2<i32>(1_i, 1_i), vec2<i32>(1_i, 1_i));
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
INSTANTIATE_TEST_SUITE_P(BuiltinBuilderTest,
                         Builtin_Builtin_DualParam_SInt_Test,
                         testing::Values(BuiltinData{"max", "SMax"}, BuiltinData{"min", "SMin"}));

using Builtin_Builtin_DualParam_UInt_Test = BuiltinBuilderTestWithParam<BuiltinData>;
TEST_P(Builtin_Builtin_DualParam_UInt_Test, Call_Scalar) {
    auto param = GetParam();
    auto* expr = Call(param.name, 1_u, 1_u);
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

TEST_P(Builtin_Builtin_DualParam_UInt_Test, Call_Vector) {
    auto param = GetParam();
    auto* expr = Call(param.name, vec2<u32>(1_u, 1_u), vec2<u32>(1_u, 1_u));
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
INSTANTIATE_TEST_SUITE_P(BuiltinBuilderTest,
                         Builtin_Builtin_DualParam_UInt_Test,
                         testing::Values(BuiltinData{"max", "UMax"}, BuiltinData{"min", "UMin"}));

using Builtin_Builtin_ThreeParam_Sint_Test = BuiltinBuilderTestWithParam<BuiltinData>;
TEST_P(Builtin_Builtin_ThreeParam_Sint_Test, Call_Scalar) {
    auto param = GetParam();
    auto* expr = Call(param.name, 1_i, 1_i, 1_i);
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

TEST_P(Builtin_Builtin_ThreeParam_Sint_Test, Call_Vector) {
    auto param = GetParam();
    auto* expr = Call(param.name, vec2<i32>(1_i, 1_i), vec2<i32>(1_i, 1_i), vec2<i32>(1_i, 1_i));
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
INSTANTIATE_TEST_SUITE_P(BuiltinBuilderTest,
                         Builtin_Builtin_ThreeParam_Sint_Test,
                         testing::Values(BuiltinData{"clamp", "SClamp"}));

using Builtin_Builtin_ThreeParam_Uint_Test = BuiltinBuilderTestWithParam<BuiltinData>;
TEST_P(Builtin_Builtin_ThreeParam_Uint_Test, Call_Scalar) {
    auto param = GetParam();
    auto* expr = Call(param.name, 1_u, 1_u, 1_u);
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

TEST_P(Builtin_Builtin_ThreeParam_Uint_Test, Call_Vector) {
    auto param = GetParam();
    auto* expr = Call(param.name, vec2<u32>(1_u, 1_u), vec2<u32>(1_u, 1_u), vec2<u32>(1_u, 1_u));
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
INSTANTIATE_TEST_SUITE_P(BuiltinBuilderTest,
                         Builtin_Builtin_ThreeParam_Uint_Test,
                         testing::Values(BuiltinData{"clamp", "UClamp"}));

TEST_F(BuiltinBuilderTest, Call_Modf) {
    auto* expr = Call("modf", vec2<f32>(1.0f, 2.0f));
    Func("a_func", {}, ty.void_(), {CallStmt(expr)}, {Stage(ast::PipelineStage::kFragment)});

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

TEST_F(BuiltinBuilderTest, Call_Frexp) {
    auto* expr = Call("frexp", vec2<f32>(1.0f, 2.0f));
    Func("a_func", {}, ty.void_(), {CallStmt(expr)}, {Stage(ast::PipelineStage::kFragment)});

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

TEST_F(BuiltinBuilderTest, Call_Determinant) {
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

TEST_F(BuiltinBuilderTest, Call_Transpose) {
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

TEST_F(BuiltinBuilderTest, Call_ArrayLength) {
    auto* s = Structure("my_struct", {Member("a", ty.array<f32>(4))});
    Global("b", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kRead,
           ast::AttributeList{
               create<ast::BindingAttribute>(1),
               create<ast::GroupAttribute>(2),
           });
    auto* expr = Call("arrayLength", AddressOf(MemberAccessor("b", "a")));

    Func("a_func", {}, ty.void_(),
         ast::StatementList{
             CallStmt(expr),
         },
         ast::AttributeList{
             Stage(ast::PipelineStage::kFragment),
         });

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.error();

    ASSERT_EQ(b.functions().size(), 1_u);

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

TEST_F(BuiltinBuilderTest, Call_ArrayLength_OtherMembersInStruct) {
    auto* s = Structure("my_struct", {
                                         Member("z", ty.f32()),
                                         Member(4, "a", ty.array<f32>(4)),
                                     });
    Global("b", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kRead,
           ast::AttributeList{
               create<ast::BindingAttribute>(1),
               create<ast::GroupAttribute>(2),
           });
    auto* expr = Call("arrayLength", AddressOf(MemberAccessor("b", "a")));

    Func("a_func", {}, ty.void_(),
         ast::StatementList{
             CallStmt(expr),
         },
         ast::AttributeList{
             Stage(ast::PipelineStage::kFragment),
         });

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.error();

    ASSERT_EQ(b.functions().size(), 1_u);

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

TEST_F(BuiltinBuilderTest, Call_ArrayLength_ViaLets) {
    auto* s = Structure("my_struct", {Member("a", ty.array<f32>(4))});
    Global("b", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kRead,
           ast::AttributeList{
               create<ast::BindingAttribute>(1),
               create<ast::GroupAttribute>(2),
           });

    auto* p = Let("p", nullptr, AddressOf("b"));
    auto* p2 = Let("p2", nullptr, AddressOf(MemberAccessor(Deref(p), "a")));
    auto* expr = Call("arrayLength", p2);

    Func("a_func", {}, ty.void_(),
         ast::StatementList{
             Decl(p),
             Decl(p2),
             CallStmt(expr),
         },
         ast::AttributeList{
             Stage(ast::PipelineStage::kFragment),
         });

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.error();

    ASSERT_EQ(b.functions().size(), 1_u);

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

TEST_F(BuiltinBuilderTest, Call_ArrayLength_ViaLets_WithPtrNoise) {
    // struct my_struct {
    //   a : array<f32>;
    // };
    // @binding(1) @group(2) var<storage, read> b : my_struct;
    //
    // fn a_func() {
    //   let p = &*&b;
    //   let p2 = &*p;
    //   let p3 = &((*p).a);
    //   arrayLength(&*p3);
    // }
    auto* s = Structure("my_struct", {Member("a", ty.array<f32>(4))});
    Global("b", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kRead,
           ast::AttributeList{
               create<ast::BindingAttribute>(1),
               create<ast::GroupAttribute>(2),
           });

    auto* p = Let("p", nullptr, AddressOf(Deref(AddressOf("b"))));
    auto* p2 = Let("p2", nullptr, AddressOf(Deref(p)));
    auto* p3 = Let("p3", nullptr, AddressOf(MemberAccessor(Deref(p2), "a")));
    auto* expr = Call("arrayLength", AddressOf(Deref(p3)));

    Func("a_func", {}, ty.void_(),
         ast::StatementList{
             Decl(p),
             Decl(p2),
             Decl(p3),
             CallStmt(expr),
         },
         ast::AttributeList{
             Stage(ast::PipelineStage::kFragment),
         });

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.error();

    ASSERT_EQ(b.functions().size(), 1_u);

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

TEST_F(BuiltinBuilderTest, Call_AtomicLoad) {
    // struct S {
    //   u : atomic<u32>;
    //   i : atomic<i32>;
    // }
    //
    // @binding(1) @group(2) var<storage, read_write> b : S;
    //
    // fn a_func() {
    //   let u : u32 = atomicLoad(&b.u);
    //   let i : i32 = atomicLoad(&b.i);
    // }
    auto* s = Structure("S", {
                                 Member("u", ty.atomic<u32>()),
                                 Member("i", ty.atomic<i32>()),
                             });
    Global("b", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kReadWrite,
           ast::AttributeList{
               create<ast::BindingAttribute>(1),
               create<ast::GroupAttribute>(2),
           });

    Func("a_func", {}, ty.void_(),
         ast::StatementList{
             Decl(Let("u", ty.u32(), Call("atomicLoad", AddressOf(MemberAccessor("b", "u"))))),
             Decl(Let("i", ty.i32(), Call("atomicLoad", AddressOf(MemberAccessor("b", "i"))))),
         },
         ast::AttributeList{Stage(ast::PipelineStage::kFragment)});

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.error();

    ASSERT_EQ(b.functions().size(), 1_u);

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

TEST_F(BuiltinBuilderTest, Call_AtomicStore) {
    // struct S {
    //   u : atomic<u32>;
    //   i : atomic<i32>;
    // }
    //
    // @binding(1) @group(2) var<storage, read_write> b : S;
    //
    // fn a_func() {
    //   var u = 1_u;
    //   var i = 2;
    //   atomicStore(&b.u, u);
    //   atomicStore(&b.i, i);
    // }
    auto* s = Structure("S", {
                                 Member("u", ty.atomic<u32>()),
                                 Member("i", ty.atomic<i32>()),
                             });
    Global("b", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kReadWrite,
           ast::AttributeList{
               create<ast::BindingAttribute>(1),
               create<ast::GroupAttribute>(2),
           });

    Func("a_func", {}, ty.void_(),
         ast::StatementList{
             Decl(Var("u", nullptr, Expr(1_u))),
             Decl(Var("i", nullptr, Expr(2_i))),
             CallStmt(Call("atomicStore", AddressOf(MemberAccessor("b", "u")), "u")),
             CallStmt(Call("atomicStore", AddressOf(MemberAccessor("b", "i")), "i")),
         },
         ast::AttributeList{Stage(ast::PipelineStage::kFragment)});

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.error();

    ASSERT_EQ(b.functions().size(), 1_u);

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

using Builtin_Builtin_AtomicRMW_i32 = BuiltinBuilderTestWithParam<BuiltinData>;
TEST_P(Builtin_Builtin_AtomicRMW_i32, Test) {
    // struct S {
    //   v : atomic<i32>;
    // }
    //
    // @binding(1) @group(2) var<storage, read_write> b : S;
    //
    // fn a_func() {
    //   var v = 10;
    //   let x : i32 = atomicOP(&b.v, v);
    // }
    auto* s = Structure("S", {
                                 Member("v", ty.atomic<i32>()),
                             });
    Global("b", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kReadWrite,
           ast::AttributeList{
               create<ast::BindingAttribute>(1),
               create<ast::GroupAttribute>(2),
           });

    Func("a_func", {}, ty.void_(),
         ast::StatementList{
             Decl(Var("v", nullptr, Expr(10_i))),
             Decl(Let("x", ty.i32(),
                      Call(GetParam().name, AddressOf(MemberAccessor("b", "v")), "v"))),
         },
         ast::AttributeList{Stage(ast::PipelineStage::kFragment)});

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.error();

    ASSERT_EQ(b.functions().size(), 1_u);

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
INSTANTIATE_TEST_SUITE_P(BuiltinBuilderTest,
                         Builtin_Builtin_AtomicRMW_i32,
                         testing::Values(BuiltinData{"atomicAdd", "OpAtomicIAdd"},
                                         BuiltinData{"atomicMax", "OpAtomicSMax"},
                                         BuiltinData{"atomicMin", "OpAtomicSMin"},
                                         BuiltinData{"atomicAnd", "OpAtomicAnd"},
                                         BuiltinData{"atomicOr", "OpAtomicOr"},
                                         BuiltinData{"atomicXor", "OpAtomicXor"}));

using Builtin_Builtin_AtomicRMW_u32 = BuiltinBuilderTestWithParam<BuiltinData>;
TEST_P(Builtin_Builtin_AtomicRMW_u32, Test) {
    // struct S {
    //   v : atomic<u32>;
    // }
    //
    // @binding(1) @group(2) var<storage, read_write> b : S;
    //
    // fn a_func() {
    //   var v = 10u;
    //   let x : u32 = atomicOP(&b.v, v);
    // }
    auto* s = Structure("S", {
                                 Member("v", ty.atomic<u32>()),
                             });
    Global("b", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kReadWrite,
           ast::AttributeList{
               create<ast::BindingAttribute>(1),
               create<ast::GroupAttribute>(2),
           });

    Func("a_func", {}, ty.void_(),
         ast::StatementList{
             Decl(Var("v", nullptr, Expr(10_u))),
             Decl(Let("x", ty.u32(),
                      Call(GetParam().name, AddressOf(MemberAccessor("b", "v")), "v"))),
         },
         ast::AttributeList{Stage(ast::PipelineStage::kFragment)});

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.error();

    ASSERT_EQ(b.functions().size(), 1_u);

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
INSTANTIATE_TEST_SUITE_P(BuiltinBuilderTest,
                         Builtin_Builtin_AtomicRMW_u32,
                         testing::Values(BuiltinData{"atomicAdd", "OpAtomicIAdd"},
                                         BuiltinData{"atomicMax", "OpAtomicUMax"},
                                         BuiltinData{"atomicMin", "OpAtomicUMin"},
                                         BuiltinData{"atomicAnd", "OpAtomicAnd"},
                                         BuiltinData{"atomicOr", "OpAtomicOr"},
                                         BuiltinData{"atomicXor", "OpAtomicXor"}));

TEST_F(BuiltinBuilderTest, Call_AtomicExchange) {
    // struct S {
    //   u : atomic<u32>;
    //   i : atomic<i32>;
    // }
    //
    // @binding(1) @group(2) var<storage, read_write> b : S;
    //
    // fn a_func() {
    //   var u = 10u;
    //   var i = 10i;
    //   let r : u32 = atomicExchange(&b.u, u);
    //   let s : i32 = atomicExchange(&b.i, i);
    // }
    auto* s = Structure("S", {
                                 Member("u", ty.atomic<u32>()),
                                 Member("i", ty.atomic<i32>()),
                             });
    Global("b", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kReadWrite,
           ast::AttributeList{
               create<ast::BindingAttribute>(1),
               create<ast::GroupAttribute>(2),
           });

    Func("a_func", {}, ty.void_(),
         ast::StatementList{
             Decl(Var("u", nullptr, Expr(10_u))),
             Decl(Var("i", nullptr, Expr(10_i))),
             Decl(Let("r", ty.u32(),
                      Call("atomicExchange", AddressOf(MemberAccessor("b", "u")), "u"))),
             Decl(Let("s", ty.i32(),
                      Call("atomicExchange", AddressOf(MemberAccessor("b", "i")), "i"))),
         },
         ast::AttributeList{Stage(ast::PipelineStage::kFragment)});

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.error();

    ASSERT_EQ(b.functions().size(), 1_u);

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

TEST_F(BuiltinBuilderTest, Call_AtomicCompareExchangeWeak) {
    // struct S {
    //   u : atomic<u32>;
    //   i : atomic<i32>;
    // }
    //
    // @binding(1) @group(2) var<storage, read_write> b : S;
    //
    // fn a_func() {
    //   let u : vec2<u32> = atomicCompareExchangeWeak(&b.u, 10u);
    //   let i : vec2<i32> = atomicCompareExchangeWeak(&b.i, 10);
    // }
    auto* s = Structure("S", {
                                 Member("u", ty.atomic<u32>()),
                                 Member("i", ty.atomic<i32>()),
                             });
    Global("b", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kReadWrite,
           ast::AttributeList{
               create<ast::BindingAttribute>(1),
               create<ast::GroupAttribute>(2),
           });

    Func("a_func", {}, ty.void_(),
         ast::StatementList{
             Decl(Let("u", ty.vec2<u32>(),
                      Call("atomicCompareExchangeWeak", AddressOf(MemberAccessor("b", "u")), 10_u,
                           20_u))),
             Decl(Let("i", ty.vec2<i32>(),
                      Call("atomicCompareExchangeWeak", AddressOf(MemberAccessor("b", "i")), 10_i,
                           20_i))),
         },
         ast::AttributeList{Stage(ast::PipelineStage::kFragment)});

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.error();

    ASSERT_EQ(b.functions().size(), 1_u);

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

using Builtin_Builtin_DataPacking_Test = BuiltinBuilderTestWithParam<BuiltinData>;
TEST_P(Builtin_Builtin_DataPacking_Test, Binary) {
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

INSTANTIATE_TEST_SUITE_P(BuiltinBuilderTest,
                         Builtin_Builtin_DataPacking_Test,
                         testing::Values(BuiltinData{"pack4x8snorm", "PackSnorm4x8"},
                                         BuiltinData{"pack4x8unorm", "PackUnorm4x8"},
                                         BuiltinData{"pack2x16snorm", "PackSnorm2x16"},
                                         BuiltinData{"pack2x16unorm", "PackUnorm2x16"},
                                         BuiltinData{"pack2x16float", "PackHalf2x16"}));

using Builtin_Builtin_DataUnpacking_Test = BuiltinBuilderTestWithParam<BuiltinData>;
TEST_P(Builtin_Builtin_DataUnpacking_Test, Binary) {
    auto param = GetParam();

    bool pack4 = param.name == "unpack4x8snorm" || param.name == "unpack4x8unorm";
    auto* func = Func("a_func", {}, ty.void_(), {CallStmt(Call(param.name, 1_u))});

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

INSTANTIATE_TEST_SUITE_P(BuiltinBuilderTest,
                         Builtin_Builtin_DataUnpacking_Test,
                         testing::Values(BuiltinData{"unpack4x8snorm", "UnpackSnorm4x8"},
                                         BuiltinData{"unpack4x8unorm", "UnpackUnorm4x8"},
                                         BuiltinData{"unpack2x16snorm", "UnpackSnorm2x16"},
                                         BuiltinData{"unpack2x16unorm", "UnpackUnorm2x16"},
                                         BuiltinData{"unpack2x16float", "UnpackHalf2x16"}));

TEST_F(BuiltinBuilderTest, Call_WorkgroupBarrier) {
    Func("f", {}, ty.void_(),
         ast::StatementList{
             CallStmt(Call("workgroupBarrier")),
         },
         ast::AttributeList{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.Build()) << b.error();

    ASSERT_EQ(b.functions().size(), 1_u);

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

TEST_F(BuiltinBuilderTest, Call_StorageBarrier) {
    Func("f", {}, ty.void_(),
         ast::StatementList{
             CallStmt(Call("storageBarrier")),
         },
         ast::AttributeList{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.Build()) << b.error();

    ASSERT_EQ(b.functions().size(), 1_u);

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

TEST_F(BuiltinBuilderTest, Call_ExtractBits_i32) {
    auto* v = Var("v", ty.i32());
    auto* offset = Var("offset", ty.u32());
    auto* count = Var("count", ty.u32());
    auto* call = Call("extractBits", v, offset, count);
    auto* func = WrapInFunction(v, offset, count, call);

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

    EXPECT_EQ(DumpBuilder(b), R"(OpEntryPoint GLCompute %3 "test_function"
OpExecutionMode %3 LocalSize 1 1 1
OpName %3 "test_function"
OpName %5 "v"
OpName %9 "offset"
OpName %13 "count"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeInt 32 1
%6 = OpTypePointer Function %7
%8 = OpConstantNull %7
%11 = OpTypeInt 32 0
%10 = OpTypePointer Function %11
%12 = OpConstantNull %11
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpVariable %6 Function %8
%9 = OpVariable %10 Function %12
%13 = OpVariable %10 Function %12
%15 = OpLoad %7 %5
%16 = OpLoad %11 %9
%17 = OpLoad %11 %13
%14 = OpBitFieldSExtract %7 %15 %16 %17
OpReturn
OpFunctionEnd
)");
}

TEST_F(BuiltinBuilderTest, Call_ExtractBits_u32) {
    auto* v = Var("v", ty.u32());
    auto* offset = Var("offset", ty.u32());
    auto* count = Var("count", ty.u32());
    auto* call = Call("extractBits", v, offset, count);
    auto* func = WrapInFunction(v, offset, count, call);

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

    EXPECT_EQ(DumpBuilder(b), R"(OpEntryPoint GLCompute %3 "test_function"
OpExecutionMode %3 LocalSize 1 1 1
OpName %3 "test_function"
OpName %5 "v"
OpName %9 "offset"
OpName %10 "count"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeInt 32 0
%6 = OpTypePointer Function %7
%8 = OpConstantNull %7
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpVariable %6 Function %8
%9 = OpVariable %6 Function %8
%10 = OpVariable %6 Function %8
%12 = OpLoad %7 %5
%13 = OpLoad %7 %9
%14 = OpLoad %7 %10
%11 = OpBitFieldUExtract %7 %12 %13 %14
OpReturn
OpFunctionEnd
)");
}

TEST_F(BuiltinBuilderTest, Call_ExtractBits_vec3_i32) {
    auto* v = Var("v", ty.vec3<i32>());
    auto* offset = Var("offset", ty.u32());
    auto* count = Var("count", ty.u32());
    auto* call = Call("extractBits", v, offset, count);
    auto* func = WrapInFunction(v, offset, count, call);

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

    EXPECT_EQ(DumpBuilder(b), R"(OpEntryPoint GLCompute %3 "test_function"
OpExecutionMode %3 LocalSize 1 1 1
OpName %3 "test_function"
OpName %5 "v"
OpName %10 "offset"
OpName %14 "count"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%8 = OpTypeInt 32 1
%7 = OpTypeVector %8 3
%6 = OpTypePointer Function %7
%9 = OpConstantNull %7
%12 = OpTypeInt 32 0
%11 = OpTypePointer Function %12
%13 = OpConstantNull %12
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpVariable %6 Function %9
%10 = OpVariable %11 Function %13
%14 = OpVariable %11 Function %13
%16 = OpLoad %7 %5
%17 = OpLoad %12 %10
%18 = OpLoad %12 %14
%15 = OpBitFieldSExtract %7 %16 %17 %18
OpReturn
OpFunctionEnd
)");
}

TEST_F(BuiltinBuilderTest, Call_ExtractBits_vec3_u32) {
    auto* v = Var("v", ty.vec3<u32>());
    auto* offset = Var("offset", ty.u32());
    auto* count = Var("count", ty.u32());
    auto* call = Call("extractBits", v, offset, count);
    auto* func = WrapInFunction(v, offset, count, call);

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

    EXPECT_EQ(DumpBuilder(b), R"(OpEntryPoint GLCompute %3 "test_function"
OpExecutionMode %3 LocalSize 1 1 1
OpName %3 "test_function"
OpName %5 "v"
OpName %10 "offset"
OpName %13 "count"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%8 = OpTypeInt 32 0
%7 = OpTypeVector %8 3
%6 = OpTypePointer Function %7
%9 = OpConstantNull %7
%11 = OpTypePointer Function %8
%12 = OpConstantNull %8
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpVariable %6 Function %9
%10 = OpVariable %11 Function %12
%13 = OpVariable %11 Function %12
%15 = OpLoad %7 %5
%16 = OpLoad %8 %10
%17 = OpLoad %8 %13
%14 = OpBitFieldUExtract %7 %15 %16 %17
OpReturn
OpFunctionEnd
)");
}

TEST_F(BuiltinBuilderTest, Call_InsertBits_i32) {
    auto* v = Var("v", ty.i32());
    auto* n = Var("n", ty.i32());
    auto* offset = Var("offset", ty.u32());
    auto* count = Var("count", ty.u32());
    auto* call = Call("insertBits", v, n, offset, count);
    auto* func = WrapInFunction(v, n, offset, count, call);

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

    EXPECT_EQ(DumpBuilder(b), R"(OpEntryPoint GLCompute %3 "test_function"
OpExecutionMode %3 LocalSize 1 1 1
OpName %3 "test_function"
OpName %5 "v"
OpName %9 "n"
OpName %10 "offset"
OpName %14 "count"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeInt 32 1
%6 = OpTypePointer Function %7
%8 = OpConstantNull %7
%12 = OpTypeInt 32 0
%11 = OpTypePointer Function %12
%13 = OpConstantNull %12
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpVariable %6 Function %8
%9 = OpVariable %6 Function %8
%10 = OpVariable %11 Function %13
%14 = OpVariable %11 Function %13
%16 = OpLoad %7 %5
%17 = OpLoad %7 %9
%18 = OpLoad %12 %10
%19 = OpLoad %12 %14
%15 = OpBitFieldInsert %7 %16 %17 %18 %19
OpReturn
OpFunctionEnd
)");
}

TEST_F(BuiltinBuilderTest, Call_InsertBits_u32) {
    auto* v = Var("v", ty.u32());
    auto* n = Var("n", ty.u32());
    auto* offset = Var("offset", ty.u32());
    auto* count = Var("count", ty.u32());
    auto* call = Call("insertBits", v, n, offset, count);
    auto* func = WrapInFunction(v, n, offset, count, call);

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

    EXPECT_EQ(DumpBuilder(b), R"(OpEntryPoint GLCompute %3 "test_function"
OpExecutionMode %3 LocalSize 1 1 1
OpName %3 "test_function"
OpName %5 "v"
OpName %9 "n"
OpName %10 "offset"
OpName %11 "count"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeInt 32 0
%6 = OpTypePointer Function %7
%8 = OpConstantNull %7
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpVariable %6 Function %8
%9 = OpVariable %6 Function %8
%10 = OpVariable %6 Function %8
%11 = OpVariable %6 Function %8
%13 = OpLoad %7 %5
%14 = OpLoad %7 %9
%15 = OpLoad %7 %10
%16 = OpLoad %7 %11
%12 = OpBitFieldInsert %7 %13 %14 %15 %16
OpReturn
OpFunctionEnd
)");
}

TEST_F(BuiltinBuilderTest, Call_InsertBits_vec3_i32) {
    auto* v = Var("v", ty.vec3<i32>());
    auto* n = Var("n", ty.vec3<i32>());
    auto* offset = Var("offset", ty.u32());
    auto* count = Var("count", ty.u32());
    auto* call = Call("insertBits", v, n, offset, count);
    auto* func = WrapInFunction(v, n, offset, count, call);

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

    EXPECT_EQ(DumpBuilder(b), R"(OpEntryPoint GLCompute %3 "test_function"
OpExecutionMode %3 LocalSize 1 1 1
OpName %3 "test_function"
OpName %5 "v"
OpName %10 "n"
OpName %11 "offset"
OpName %15 "count"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%8 = OpTypeInt 32 1
%7 = OpTypeVector %8 3
%6 = OpTypePointer Function %7
%9 = OpConstantNull %7
%13 = OpTypeInt 32 0
%12 = OpTypePointer Function %13
%14 = OpConstantNull %13
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpVariable %6 Function %9
%10 = OpVariable %6 Function %9
%11 = OpVariable %12 Function %14
%15 = OpVariable %12 Function %14
%17 = OpLoad %7 %5
%18 = OpLoad %7 %10
%19 = OpLoad %13 %11
%20 = OpLoad %13 %15
%16 = OpBitFieldInsert %7 %17 %18 %19 %20
OpReturn
OpFunctionEnd
)");
}

TEST_F(BuiltinBuilderTest, Call_InsertBits_vec3_u32) {
    auto* v = Var("v", ty.vec3<u32>());
    auto* n = Var("n", ty.vec3<u32>());
    auto* offset = Var("offset", ty.u32());
    auto* count = Var("count", ty.u32());
    auto* call = Call("insertBits", v, n, offset, count);
    auto* func = WrapInFunction(v, n, offset, count, call);

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.error();

    EXPECT_EQ(DumpBuilder(b), R"(OpEntryPoint GLCompute %3 "test_function"
OpExecutionMode %3 LocalSize 1 1 1
OpName %3 "test_function"
OpName %5 "v"
OpName %10 "n"
OpName %11 "offset"
OpName %14 "count"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%8 = OpTypeInt 32 0
%7 = OpTypeVector %8 3
%6 = OpTypePointer Function %7
%9 = OpConstantNull %7
%12 = OpTypePointer Function %8
%13 = OpConstantNull %8
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpVariable %6 Function %9
%10 = OpVariable %6 Function %9
%11 = OpVariable %12 Function %13
%14 = OpVariable %12 Function %13
%16 = OpLoad %7 %5
%17 = OpLoad %7 %10
%18 = OpLoad %8 %11
%19 = OpLoad %8 %14
%15 = OpBitFieldInsert %7 %16 %17 %18 %19
OpReturn
OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::writer::spirv
