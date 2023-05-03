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
#include "src/tint/type/depth_texture.h"
#include "src/tint/type/texture_dimension.h"
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

// This tests that we do not push OpTypeSampledImage and float_0 type twice.
TEST_F(BuiltinBuilderTest, Call_TextureSampleCompare_Twice) {
    auto s = ty.sampler(type::SamplerKind::kComparisonSampler);
    auto t = ty.depth_texture(type::TextureDimension::k2d);

    auto* tex = GlobalVar("texture", t, Binding(0_a), Group(0_a));
    auto* sampler = GlobalVar("sampler", s, Binding(1_a), Group(0_a));

    auto* expr1 = Call("textureSampleCompare", "texture", "sampler", vec2<f32>(1_f, 2_f), 2_f);
    auto* expr2 = Call("textureSampleCompare", "texture", "sampler", vec2<f32>(1_f, 2_f), 2_f);

    Func("f1", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Let("l", expr1)),
         },
         utils::Empty);
    Func("f2", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Let("l", expr2)),
         },
         utils::Empty);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    ASSERT_TRUE(b.GenerateGlobalVariable(tex)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateGlobalVariable(sampler)) << b.Diagnostics();

    EXPECT_EQ(b.GenerateExpression(expr1), 8u) << b.Diagnostics();
    EXPECT_EQ(b.GenerateExpression(expr2), 17u) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeFloat 32
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

    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
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

TEST_F(BuiltinBuilderTest, Call_GLSLMethod_WithLoad_f32) {
    auto* var = GlobalVar("ident", ty.f32(), builtin::AddressSpace::kPrivate);
    auto* expr = Call("round", "ident");
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect =
        R"(%10 = OpExtInstImport "GLSL.std.450"
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
)";
    EXPECT_EQ(got, expect);
}

TEST_F(BuiltinBuilderTest, Call_GLSLMethod_WithLoad_f16) {
    Enable(builtin::Extension::kF16);

    auto* var = GlobalVar("ident", ty.f16(), builtin::AddressSpace::kPrivate);
    auto* expr = Call("round", "ident");
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect =
        R"(%10 = OpExtInstImport "GLSL.std.450"
OpName %1 "ident"
OpName %7 "a_func"
%3 = OpTypeFloat 16
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
)";
    EXPECT_EQ(got, expect);
}

// Tests for Logical builtins
namespace logical_builtin_tests {

using BuiltinBoolTest = BuiltinBuilderTestWithParam<BuiltinData>;
TEST_P(BuiltinBoolTest, Call_Bool_Scalar) {
    auto param = GetParam();
    auto* var = GlobalVar("v", ty.bool_(), builtin::AddressSpace::kPrivate);
    auto* expr = Call(param.name, "v");
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeBool
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%6 = OpTypeVoid
%5 = OpTypeFunction %6
)");

    // both any and all are 'passthrough' for scalar booleans
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              "%10 = OpLoad %3 %1\nOpReturn\n");
}

TEST_P(BuiltinBoolTest, Call_Bool_Vector) {
    auto param = GetParam();
    auto* var = GlobalVar("v", ty.vec3<bool>(), builtin::AddressSpace::kPrivate);
    auto* expr = Call(param.name, "v");
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeBool
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
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), expected);
}
INSTANTIATE_TEST_SUITE_P(BuiltinBuilderTest,
                         BuiltinBoolTest,
                         testing::Values(BuiltinData{"any", "OpAny"}, BuiltinData{"all", "OpAll"}));

TEST_F(BuiltinBuilderTest, Call_Select) {
    auto* v3 = GlobalVar("v3", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);

    auto* bool_v3 = GlobalVar("bool_v3", ty.vec3<bool>(), builtin::AddressSpace::kPrivate);
    auto* expr = Call("select", "v3", "v3", "bool_v3");
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(v3)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateGlobalVariable(bool_v3)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeFloat 32
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
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(%16 = OpLoad %8 %6
%17 = OpLoad %3 %1
%18 = OpLoad %3 %1
%15 = OpSelect %3 %16 %17 %18
OpReturn
)");
}

}  // namespace logical_builtin_tests

// Tests for Array builtins
namespace array_builtin_tests {

TEST_F(BuiltinBuilderTest, Call_ArrayLength) {
    auto* s = Structure("my_struct", utils::Vector{
                                         Member("a", ty.array<f32>()),
                                     });
    GlobalVar("b", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kRead, Binding(1_a),
              Group(2_a));
    auto* expr = Call("arrayLength", AddressOf(MemberAccessor("b", "a")));

    Func("a_func", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Let("l", expr)),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    ASSERT_EQ(b.Module().Functions().size(), 1_u);

    auto* expected_types = R"(%5 = OpTypeFloat 32
%4 = OpTypeRuntimeArray %5
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%7 = OpTypeVoid
%6 = OpTypeFunction %7
%11 = OpTypeInt 32 0
)";
    auto got_types = DumpInstructions(b.Module().Types());
    EXPECT_EQ(expected_types, got_types);

    auto* expected_instructions = R"(%10 = OpArrayLength %11 %1 0
OpReturn
)";
    auto got_instructions = DumpInstructions(b.Module().Functions()[0].instructions());
    EXPECT_EQ(expected_instructions, got_instructions);

    Validate(b);
}

TEST_F(BuiltinBuilderTest, Call_ArrayLength_OtherMembersInStruct) {
    auto* s = Structure("my_struct", utils::Vector{
                                         Member("z", ty.f32()),
                                         Member(4, "a", ty.array<f32>()),
                                     });
    GlobalVar("b", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kRead, Binding(1_a),
              Group(2_a));
    auto* expr = Call("arrayLength", AddressOf(MemberAccessor("b", "a")));

    Func("a_func", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Let("l", expr)),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    ASSERT_EQ(b.Module().Functions().size(), 1_u);

    auto* expected_types = R"(%4 = OpTypeFloat 32
%5 = OpTypeRuntimeArray %4
%3 = OpTypeStruct %4 %5
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%7 = OpTypeVoid
%6 = OpTypeFunction %7
%11 = OpTypeInt 32 0
)";
    auto got_types = DumpInstructions(b.Module().Types());
    EXPECT_EQ(expected_types, got_types);

    auto* expected_instructions = R"(%10 = OpArrayLength %11 %1 1
OpReturn
)";
    auto got_instructions = DumpInstructions(b.Module().Functions()[0].instructions());
    EXPECT_EQ(expected_instructions, got_instructions);

    Validate(b);
}

TEST_F(BuiltinBuilderTest, Call_ArrayLength_ViaLets) {
    auto* s = Structure("my_struct", utils::Vector{
                                         Member("a", ty.array<f32>()),
                                     });
    GlobalVar("b", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kRead, Binding(1_a),
              Group(2_a));

    auto* p = Let("p", AddressOf("b"));
    auto* p2 = Let("p2", AddressOf(MemberAccessor(Deref(p), "a")));
    auto* expr = Call("arrayLength", p2);

    Func("a_func", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(p),
             Decl(p2),
             Decl(Let("l", expr)),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    ASSERT_EQ(b.Module().Functions().size(), 1_u);

    auto* expected_types = R"(%5 = OpTypeFloat 32
%4 = OpTypeRuntimeArray %5
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%7 = OpTypeVoid
%6 = OpTypeFunction %7
%11 = OpTypeInt 32 0
)";
    auto got_types = DumpInstructions(b.Module().Types());
    EXPECT_EQ(expected_types, got_types);

    auto* expected_instructions = R"(%10 = OpArrayLength %11 %1 0
OpReturn
)";
    auto got_instructions = DumpInstructions(b.Module().Functions()[0].instructions());
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
    auto* s = Structure("my_struct", utils::Vector{
                                         Member("a", ty.array<f32>()),
                                     });
    GlobalVar("b", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kRead, Binding(1_a),
              Group(2_a));

    auto* p = Let("p", AddressOf(Deref(AddressOf("b"))));
    auto* p2 = Let("p2", AddressOf(Deref(p)));
    auto* p3 = Let("p3", AddressOf(MemberAccessor(Deref(p2), "a")));
    auto* expr = Call("arrayLength", AddressOf(Deref(p3)));

    Func("a_func", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(p),
             Decl(p2),
             Decl(p3),
             Decl(Let("l", expr)),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    ASSERT_EQ(b.Module().Functions().size(), 1_u);

    auto* expected_types = R"(%5 = OpTypeFloat 32
%4 = OpTypeRuntimeArray %5
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%7 = OpTypeVoid
%6 = OpTypeFunction %7
%11 = OpTypeInt 32 0
)";
    auto got_types = DumpInstructions(b.Module().Types());
    EXPECT_EQ(expected_types, got_types);

    auto* expected_instructions = R"(%10 = OpArrayLength %11 %1 0
OpReturn
)";
    auto got_instructions = DumpInstructions(b.Module().Functions()[0].instructions());
    EXPECT_EQ(expected_instructions, got_instructions);

    Validate(b);
}

}  // namespace array_builtin_tests

// Tests for Numeric builtins with float parameter
namespace float_builtin_tests {

using Builtin_Builder_SingleParam_Float_Test = BuiltinBuilderTestWithParam<BuiltinData>;
TEST_P(Builtin_Builder_SingleParam_Float_Test, Call_Scalar_f32) {
    auto param = GetParam();
    // Use a variable to prevent the function being evaluated as constant.
    auto* scalar = Var("a", Expr(1_f));
    auto* expr = Call(param.name, scalar);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(scalar),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%11 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %7 "a"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeFloat 32
%6 = OpConstant %5 1
%8 = OpTypePointer Function %5
%9 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%7 = OpVariable %8 Function %9
OpStore %7 %6
%12 = OpLoad %5 %7
%10 = OpExtInst %5 %11 )" +
                  param.op +
                  R"( %12
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

TEST_P(Builtin_Builder_SingleParam_Float_Test, Call_Scalar_f16) {
    Enable(builtin::Extension::kF16);

    auto param = GetParam();
    // Use a variable to prevent the function being evaluated as constant.
    auto* scalar = Var("a", Expr(1_h));
    auto* expr = Call(param.name, scalar);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(scalar),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%11 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %7 "a"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeFloat 16
%6 = OpConstant %5 0x1p+0
%8 = OpTypePointer Function %5
%9 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%7 = OpVariable %8 Function %9
OpStore %7 %6
%12 = OpLoad %5 %7
%10 = OpExtInst %5 %11 )" +
                  param.op +
                  R"( %12
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

TEST_P(Builtin_Builder_SingleParam_Float_Test, Call_Vector_f32) {
    auto param = GetParam();

    // Use a variable to prevent the function being evaluated as constant.
    auto* vec = Var("a", vec2<f32>(1_f, 1_f));
    auto* expr = Call(param.name, vec);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(vec),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%13 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %9 "a"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%5 = OpTypeVector %6 2
%7 = OpConstant %6 1
%8 = OpConstantComposite %5 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%9 = OpVariable %10 Function %11
OpStore %9 %8
%14 = OpLoad %5 %9
%12 = OpExtInst %5 %13 )" +
                  param.op +
                  R"( %14
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

TEST_P(Builtin_Builder_SingleParam_Float_Test, Call_Vector_f16) {
    Enable(builtin::Extension::kF16);

    auto param = GetParam();

    // Use a variable to prevent the function being evaluated as constant.
    auto* vec = Var("a", vec2<f16>(1_h, 1_h));
    auto* expr = Call(param.name, vec);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(vec),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%13 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %9 "a"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 16
%5 = OpTypeVector %6 2
%7 = OpConstant %6 0x1p+0
%8 = OpConstantComposite %5 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%9 = OpVariable %10 Function %11
OpStore %9 %8
%14 = OpLoad %5 %9
%12 = OpExtInst %5 %13 )" +
                  param.op +
                  R"( %14
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

INSTANTIATE_TEST_SUITE_P(BuiltinBuilderTest,
                         Builtin_Builder_SingleParam_Float_Test,
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

TEST_F(BuiltinBuilderTest, Call_Length_Scalar_f32) {
    auto* scalar = Var("a", Expr(1_f));
    auto* expr = Call("length", scalar);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(scalar),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%11 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %7 "a"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeFloat 32
%6 = OpConstant %5 1
%8 = OpTypePointer Function %5
%9 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%7 = OpVariable %8 Function %9
OpStore %7 %6
%12 = OpLoad %5 %7
%10 = OpExtInst %5 %11 Length %12
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

TEST_F(BuiltinBuilderTest, Call_Length_Scalar_f16) {
    Enable(builtin::Extension::kF16);

    auto* scalar = Var("a", Expr(1_h));
    auto* expr = Call("length", scalar);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(scalar),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%11 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %7 "a"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeFloat 16
%6 = OpConstant %5 0x1p+0
%8 = OpTypePointer Function %5
%9 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%7 = OpVariable %8 Function %9
OpStore %7 %6
%12 = OpLoad %5 %7
%10 = OpExtInst %5 %11 Length %12
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

TEST_F(BuiltinBuilderTest, Call_Length_Vector_f32) {
    auto* vec = Var("a", vec2<f32>(1_f, 1_f));
    auto* expr = Call("length", vec);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(vec),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%13 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %9 "a"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%5 = OpTypeVector %6 2
%7 = OpConstant %6 1
%8 = OpConstantComposite %5 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%9 = OpVariable %10 Function %11
OpStore %9 %8
%14 = OpLoad %5 %9
%12 = OpExtInst %6 %13 Length %14
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

TEST_F(BuiltinBuilderTest, Call_Length_Vector_f16) {
    Enable(builtin::Extension::kF16);

    auto* vec = Var("a", vec2<f16>(1_h, 1_h));
    auto* expr = Call("length", vec);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(vec),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%13 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %9 "a"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 16
%5 = OpTypeVector %6 2
%7 = OpConstant %6 0x1p+0
%8 = OpConstantComposite %5 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%9 = OpVariable %10 Function %11
OpStore %9 %8
%14 = OpLoad %5 %9
%12 = OpExtInst %6 %13 Length %14
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

TEST_F(BuiltinBuilderTest, Call_Normalize_f32) {
    auto* vec = Var("a", vec2<f32>(1_f, 1_f));
    auto* expr = Call("normalize", vec);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(vec),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%13 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %9 "a"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%5 = OpTypeVector %6 2
%7 = OpConstant %6 1
%8 = OpConstantComposite %5 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%9 = OpVariable %10 Function %11
OpStore %9 %8
%14 = OpLoad %5 %9
%12 = OpExtInst %5 %13 Normalize %14
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

TEST_F(BuiltinBuilderTest, Call_Normalize_f16) {
    Enable(builtin::Extension::kF16);

    auto* vec = Var("a", vec2<f16>(1_h, 1_h));
    auto* expr = Call("normalize", vec);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(vec),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%13 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %9 "a"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 16
%5 = OpTypeVector %6 2
%7 = OpConstant %6 0x1p+0
%8 = OpConstantComposite %5 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%9 = OpVariable %10 Function %11
OpStore %9 %8
%14 = OpLoad %5 %9
%12 = OpExtInst %5 %13 Normalize %14
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

using Builtin_Builder_DualParam_Float_Test = BuiltinBuilderTestWithParam<BuiltinData>;
TEST_P(Builtin_Builder_DualParam_Float_Test, Call_Scalar_f32) {
    auto param = GetParam();
    auto* scalar = Var("scalar", Expr(1_f));
    auto* expr = Call(param.name, scalar, scalar);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(scalar),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%11 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %7 "scalar"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeFloat 32
%6 = OpConstant %5 1
%8 = OpTypePointer Function %5
%9 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%7 = OpVariable %8 Function %9
OpStore %7 %6
%12 = OpLoad %5 %7
%13 = OpLoad %5 %7
%10 = OpExtInst %5 %11 )" +
                  param.op +
                  R"( %12 %13
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

TEST_P(Builtin_Builder_DualParam_Float_Test, Call_Scalar_f16) {
    Enable(builtin::Extension::kF16);

    auto param = GetParam();
    auto* scalar = Var("scalar", Expr(1_h));
    auto* expr = Call(param.name, scalar, scalar);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(scalar),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%11 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %7 "scalar"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeFloat 16
%6 = OpConstant %5 0x1p+0
%8 = OpTypePointer Function %5
%9 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%7 = OpVariable %8 Function %9
OpStore %7 %6
%12 = OpLoad %5 %7
%13 = OpLoad %5 %7
%10 = OpExtInst %5 %11 )" +
                  param.op +
                  R"( %12 %13
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

TEST_P(Builtin_Builder_DualParam_Float_Test, Call_Vector_f32) {
    auto param = GetParam();
    auto* vec = Var("vec", vec2<f32>(1_f, 1_f));
    auto* expr = Call(param.name, vec, vec);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(vec),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%13 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %9 "vec"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%5 = OpTypeVector %6 2
%7 = OpConstant %6 1
%8 = OpConstantComposite %5 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%9 = OpVariable %10 Function %11
OpStore %9 %8
%14 = OpLoad %5 %9
%15 = OpLoad %5 %9
%12 = OpExtInst %5 %13 )" +
                  param.op +
                  R"( %14 %15
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

TEST_P(Builtin_Builder_DualParam_Float_Test, Call_Vector_f16) {
    Enable(builtin::Extension::kF16);

    auto param = GetParam();
    auto* vec = Var("vec", vec2<f16>(1_h, 1_h));
    auto* expr = Call(param.name, vec, vec);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(vec),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%13 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %9 "vec"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 16
%5 = OpTypeVector %6 2
%7 = OpConstant %6 0x1p+0
%8 = OpConstantComposite %5 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%9 = OpVariable %10 Function %11
OpStore %9 %8
%14 = OpLoad %5 %9
%15 = OpLoad %5 %9
%12 = OpExtInst %5 %13 )" +
                  param.op +
                  R"( %14 %15
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

INSTANTIATE_TEST_SUITE_P(BuiltinBuilderTest,
                         Builtin_Builder_DualParam_Float_Test,
                         testing::Values(BuiltinData{"atan2", "Atan2"},
                                         BuiltinData{"max", "NMax"},
                                         BuiltinData{"min", "NMin"},
                                         BuiltinData{"pow", "Pow"},
                                         BuiltinData{"step", "Step"}));

TEST_F(BuiltinBuilderTest, Call_Reflect_Vector_f32) {
    auto* vec = Var("vec", vec2<f32>(1_f, 1_f));
    auto* expr = Call("reflect", vec, vec);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(vec),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%13 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %9 "vec"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%5 = OpTypeVector %6 2
%7 = OpConstant %6 1
%8 = OpConstantComposite %5 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%9 = OpVariable %10 Function %11
OpStore %9 %8
%14 = OpLoad %5 %9
%15 = OpLoad %5 %9
%12 = OpExtInst %5 %13 Reflect %14 %15
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

TEST_F(BuiltinBuilderTest, Call_Reflect_Vector_f16) {
    Enable(builtin::Extension::kF16);

    auto* vec = Var("vec", vec2<f16>(1_h, 1_h));
    auto* expr = Call("reflect", vec, vec);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(vec),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%13 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %9 "vec"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 16
%5 = OpTypeVector %6 2
%7 = OpConstant %6 0x1p+0
%8 = OpConstantComposite %5 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%9 = OpVariable %10 Function %11
OpStore %9 %8
%14 = OpLoad %5 %9
%15 = OpLoad %5 %9
%12 = OpExtInst %5 %13 Reflect %14 %15
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

TEST_F(BuiltinBuilderTest, Call_Distance_Scalar_f32) {
    auto* scalar = Var("scalar", Expr(1_f));
    auto* expr = Call("distance", scalar, scalar);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(scalar),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%11 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %7 "scalar"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeFloat 32
%6 = OpConstant %5 1
%8 = OpTypePointer Function %5
%9 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%7 = OpVariable %8 Function %9
OpStore %7 %6
%12 = OpLoad %5 %7
%13 = OpLoad %5 %7
%10 = OpExtInst %5 %11 Distance %12 %13
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

TEST_F(BuiltinBuilderTest, Call_Distance_Scalar_f16) {
    Enable(builtin::Extension::kF16);

    auto* scalar = Var("scalar", Expr(1_h));
    auto* expr = Call("distance", scalar, scalar);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(scalar),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%11 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %7 "scalar"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeFloat 16
%6 = OpConstant %5 0x1p+0
%8 = OpTypePointer Function %5
%9 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%7 = OpVariable %8 Function %9
OpStore %7 %6
%12 = OpLoad %5 %7
%13 = OpLoad %5 %7
%10 = OpExtInst %5 %11 Distance %12 %13
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

TEST_F(BuiltinBuilderTest, Call_Distance_Vector_f32) {
    auto* vec = Var("vec", vec2<f32>(1_f, 1_f));
    auto* expr = Call("distance", vec, vec);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(vec),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%13 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %9 "vec"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%5 = OpTypeVector %6 2
%7 = OpConstant %6 1
%8 = OpConstantComposite %5 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%9 = OpVariable %10 Function %11
OpStore %9 %8
%14 = OpLoad %5 %9
%15 = OpLoad %5 %9
%12 = OpExtInst %6 %13 Distance %14 %15
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

TEST_F(BuiltinBuilderTest, Call_Distance_Vector_f16) {
    Enable(builtin::Extension::kF16);

    auto* vec = Var("vec", vec2<f16>(1_h, 1_h));
    auto* expr = Call("distance", vec, vec);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(vec),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%13 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %9 "vec"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 16
%5 = OpTypeVector %6 2
%7 = OpConstant %6 0x1p+0
%8 = OpConstantComposite %5 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%9 = OpVariable %10 Function %11
OpStore %9 %8
%14 = OpLoad %5 %9
%15 = OpLoad %5 %9
%12 = OpExtInst %6 %13 Distance %14 %15
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

TEST_F(BuiltinBuilderTest, Call_Cross_f32) {
    auto* vec = Var("vec", vec3<f32>(1_f, 1_f, 1_f));
    auto* expr = Call("cross", vec, vec);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(vec),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%13 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %9 "vec"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%5 = OpTypeVector %6 3
%7 = OpConstant %6 1
%8 = OpConstantComposite %5 %7 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%9 = OpVariable %10 Function %11
OpStore %9 %8
%14 = OpLoad %5 %9
%15 = OpLoad %5 %9
%12 = OpExtInst %5 %13 Cross %14 %15
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

TEST_F(BuiltinBuilderTest, Call_Cross_f16) {
    Enable(builtin::Extension::kF16);

    auto* vec = Var("vec", vec3<f16>(1_h, 1_h, 1_h));
    auto* expr = Call("cross", vec, vec);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(vec),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%13 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %9 "vec"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 16
%5 = OpTypeVector %6 3
%7 = OpConstant %6 0x1p+0
%8 = OpConstantComposite %5 %7 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%9 = OpVariable %10 Function %11
OpStore %9 %8
%14 = OpLoad %5 %9
%15 = OpLoad %5 %9
%12 = OpExtInst %5 %13 Cross %14 %15
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

using Builtin_Builder_ThreeParam_Float_Test = BuiltinBuilderTestWithParam<BuiltinData>;
TEST_P(Builtin_Builder_ThreeParam_Float_Test, Call_Scalar_f32) {
    auto param = GetParam();
    auto* scalar = Var("scalar", Expr(1_f));
    auto* expr = Call(param.name, scalar, scalar, scalar);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(scalar),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%11 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %7 "scalar"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeFloat 32
%6 = OpConstant %5 1
%8 = OpTypePointer Function %5
%9 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%7 = OpVariable %8 Function %9
OpStore %7 %6
%12 = OpLoad %5 %7
%13 = OpLoad %5 %7
%14 = OpLoad %5 %7
%10 = OpExtInst %5 %11 )" +
                  param.op +
                  R"( %12 %13 %14
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

TEST_P(Builtin_Builder_ThreeParam_Float_Test, Call_Scalar_f16) {
    Enable(builtin::Extension::kF16);

    auto param = GetParam();
    auto* scalar = Var("scalar", Expr(1_h));
    auto* expr = Call(param.name, scalar, scalar, scalar);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(scalar),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%11 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %7 "scalar"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeFloat 16
%6 = OpConstant %5 0x1p+0
%8 = OpTypePointer Function %5
%9 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%7 = OpVariable %8 Function %9
OpStore %7 %6
%12 = OpLoad %5 %7
%13 = OpLoad %5 %7
%14 = OpLoad %5 %7
%10 = OpExtInst %5 %11 )" +
                  param.op +
                  R"( %12 %13 %14
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

TEST_P(Builtin_Builder_ThreeParam_Float_Test, Call_Vector_f32) {
    auto param = GetParam();
    auto* vec = Var("vec", vec2<f32>(1_f, 1_f));
    auto* expr = Call(param.name, vec, vec, vec);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(vec),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%13 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %9 "vec"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%5 = OpTypeVector %6 2
%7 = OpConstant %6 1
%8 = OpConstantComposite %5 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%9 = OpVariable %10 Function %11
OpStore %9 %8
%14 = OpLoad %5 %9
%15 = OpLoad %5 %9
%16 = OpLoad %5 %9
%12 = OpExtInst %5 %13 )" +
                  param.op +
                  R"( %14 %15 %16
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

TEST_P(Builtin_Builder_ThreeParam_Float_Test, Call_Vector_f16) {
    Enable(builtin::Extension::kF16);

    auto param = GetParam();
    auto* vec = Var("vec", vec2<f16>(1_h, 1_h));
    auto* expr = Call(param.name, vec, vec, vec);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(vec),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%13 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %9 "vec"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 16
%5 = OpTypeVector %6 2
%7 = OpConstant %6 0x1p+0
%8 = OpConstantComposite %5 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%9 = OpVariable %10 Function %11
OpStore %9 %8
%14 = OpLoad %5 %9
%15 = OpLoad %5 %9
%16 = OpLoad %5 %9
%12 = OpExtInst %5 %13 )" +
                  param.op +
                  R"( %14 %15 %16
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

INSTANTIATE_TEST_SUITE_P(BuiltinBuilderTest,
                         Builtin_Builder_ThreeParam_Float_Test,
                         testing::Values(BuiltinData{"clamp", "NClamp"},
                                         BuiltinData{"fma", "Fma"},
                                         BuiltinData{"mix", "FMix"},

                                         BuiltinData{"smoothstep", "SmoothStep"}));

TEST_F(BuiltinBuilderTest, Call_FaceForward_Vector_f32) {
    auto* vec = Var("vec", vec2<f32>(1_f, 1_f));
    auto* expr = Call("faceForward", vec, vec, vec);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(vec),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%13 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %9 "vec"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%5 = OpTypeVector %6 2
%7 = OpConstant %6 1
%8 = OpConstantComposite %5 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%9 = OpVariable %10 Function %11
OpStore %9 %8
%14 = OpLoad %5 %9
%15 = OpLoad %5 %9
%16 = OpLoad %5 %9
%12 = OpExtInst %5 %13 FaceForward %14 %15 %16
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

TEST_F(BuiltinBuilderTest, Call_FaceForward_Vector_f16) {
    Enable(builtin::Extension::kF16);

    auto* vec = Var("vec", vec2<f16>(1_h, 1_h));
    auto* expr = Call("faceForward", vec, vec, vec);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(vec),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%13 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %9 "vec"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 16
%5 = OpTypeVector %6 2
%7 = OpConstant %6 0x1p+0
%8 = OpConstantComposite %5 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%9 = OpVariable %10 Function %11
OpStore %9 %8
%14 = OpLoad %5 %9
%15 = OpLoad %5 %9
%16 = OpLoad %5 %9
%12 = OpExtInst %5 %13 FaceForward %14 %15 %16
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

TEST_F(BuiltinBuilderTest, Runtime_Call_Modf_f32) {
    auto* vec = Var("vec", vec2<f32>(1_f, 2_f));
    auto* expr = Call("modf", vec);
    Func("a_func", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(vec),
             Decl(Let("l", expr)),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();
    auto got = DumpBuilder(b);
    auto* expect = R"(OpCapability Shader
%15 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %3 "a_func"
OpExecutionMode %3 OriginUpperLeft
OpName %3 "a_func"
OpName %10 "vec"
OpName %14 "__modf_result_vec2_f32"
OpMemberName %14 0 "fract"
OpMemberName %14 1 "whole"
OpMemberDecorate %14 0 Offset 0
OpMemberDecorate %14 1 Offset 8
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%5 = OpTypeVector %6 2
%7 = OpConstant %6 1
%8 = OpConstant %6 2
%9 = OpConstantComposite %5 %7 %8
%11 = OpTypePointer Function %5
%12 = OpConstantNull %5
%14 = OpTypeStruct %5 %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%10 = OpVariable %11 Function %12
OpStore %10 %9
%16 = OpLoad %5 %10
%13 = OpExtInst %14 %15 ModfStruct %16
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(expect, got);

    Validate(b);
}

TEST_F(BuiltinBuilderTest, Runtime_Call_Modf_f16) {
    Enable(builtin::Extension::kF16);

    auto* vec = Var("vec", vec2<f16>(1_h, 2_h));
    auto* expr = Call("modf", vec);
    Func("a_func", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(vec),
             Decl(Let("l", expr)),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();
    auto got = DumpBuilder(b);
    auto* expect = R"(OpCapability Shader
OpCapability Float16
OpCapability UniformAndStorageBuffer16BitAccess
OpCapability StorageBuffer16BitAccess
OpCapability StorageInputOutput16
%15 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %3 "a_func"
OpExecutionMode %3 OriginUpperLeft
OpName %3 "a_func"
OpName %10 "vec"
OpName %14 "__modf_result_vec2_f16"
OpMemberName %14 0 "fract"
OpMemberName %14 1 "whole"
OpMemberDecorate %14 0 Offset 0
OpMemberDecorate %14 1 Offset 4
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 16
%5 = OpTypeVector %6 2
%7 = OpConstant %6 0x1p+0
%8 = OpConstant %6 0x1p+1
%9 = OpConstantComposite %5 %7 %8
%11 = OpTypePointer Function %5
%12 = OpConstantNull %5
%14 = OpTypeStruct %5 %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%10 = OpVariable %11 Function %12
OpStore %10 %9
%16 = OpLoad %5 %10
%13 = OpExtInst %14 %15 ModfStruct %16
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(expect, got);

    Validate(b);
}

TEST_F(BuiltinBuilderTest, Const_Call_Modf_f32) {
    auto* expr = Call("modf", vec2<f32>(1_f, 2_f));
    Func("a_func", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Let("l", expr)),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();
    auto got = DumpBuilder(b);
    auto* expect = R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %3 "a_func"
OpExecutionMode %3 OriginUpperLeft
OpName %3 "a_func"
OpName %5 "__modf_result_vec2_f32"
OpMemberName %5 0 "fract"
OpMemberName %5 1 "whole"
OpMemberDecorate %5 0 Offset 0
OpMemberDecorate %5 1 Offset 8
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeFloat 32
%6 = OpTypeVector %7 2
%5 = OpTypeStruct %6 %6
%8 = OpConstantNull %6
%9 = OpConstant %7 1
%10 = OpConstant %7 2
%11 = OpConstantComposite %6 %9 %10
%12 = OpConstantComposite %5 %8 %11
%3 = OpFunction %2 None %1
%4 = OpLabel
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(expect, got);

    Validate(b);
}

TEST_F(BuiltinBuilderTest, Const_Call_Modf_f16) {
    Enable(builtin::Extension::kF16);

    auto* expr = Call("modf", vec2<f16>(1_h, 2_h));
    Func("a_func", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Let("l", expr)),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();
    auto got = DumpBuilder(b);
    auto* expect = R"(OpCapability Shader
OpCapability Float16
OpCapability UniformAndStorageBuffer16BitAccess
OpCapability StorageBuffer16BitAccess
OpCapability StorageInputOutput16
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %3 "a_func"
OpExecutionMode %3 OriginUpperLeft
OpName %3 "a_func"
OpName %5 "__modf_result_vec2_f16"
OpMemberName %5 0 "fract"
OpMemberName %5 1 "whole"
OpMemberDecorate %5 0 Offset 0
OpMemberDecorate %5 1 Offset 4
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeFloat 16
%6 = OpTypeVector %7 2
%5 = OpTypeStruct %6 %6
%8 = OpConstantNull %6
%9 = OpConstant %7 0x1p+0
%10 = OpConstant %7 0x1p+1
%11 = OpConstantComposite %6 %9 %10
%12 = OpConstantComposite %5 %8 %11
%3 = OpFunction %2 None %1
%4 = OpLabel
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(expect, got);

    Validate(b);
}

TEST_F(BuiltinBuilderTest, Runtime_Call_Frexp_f32) {
    auto* vec = Var("vec", vec2<f32>(1_f, 2_f));
    auto* expr = Call("frexp", vec);
    Func("a_func", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(vec),
             Decl(Let("l", expr)),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();
    auto got = DumpBuilder(b);
    auto* expect = R"(OpCapability Shader
%17 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %3 "a_func"
OpExecutionMode %3 OriginUpperLeft
OpName %3 "a_func"
OpName %10 "vec"
OpName %14 "__frexp_result_vec2_f32"
OpMemberName %14 0 "fract"
OpMemberName %14 1 "exp"
OpMemberDecorate %14 0 Offset 0
OpMemberDecorate %14 1 Offset 8
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%5 = OpTypeVector %6 2
%7 = OpConstant %6 1
%8 = OpConstant %6 2
%9 = OpConstantComposite %5 %7 %8
%11 = OpTypePointer Function %5
%12 = OpConstantNull %5
%16 = OpTypeInt 32 1
%15 = OpTypeVector %16 2
%14 = OpTypeStruct %5 %15
%3 = OpFunction %2 None %1
%4 = OpLabel
%10 = OpVariable %11 Function %12
OpStore %10 %9
%18 = OpLoad %5 %10
%13 = OpExtInst %14 %17 FrexpStruct %18
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(expect, got);

    Validate(b);
}

TEST_F(BuiltinBuilderTest, Runtime_Call_Frexp_f16) {
    Enable(builtin::Extension::kF16);

    auto* vec = Var("vec", vec2<f16>(1_h, 2_h));
    auto* expr = Call("frexp", vec);
    Func("a_func", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(vec),
             Decl(Let("l", expr)),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();
    auto got = DumpBuilder(b);
    auto* expect = R"(OpCapability Shader
OpCapability Float16
OpCapability UniformAndStorageBuffer16BitAccess
OpCapability StorageBuffer16BitAccess
OpCapability StorageInputOutput16
%17 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %3 "a_func"
OpExecutionMode %3 OriginUpperLeft
OpName %3 "a_func"
OpName %10 "vec"
OpName %14 "__frexp_result_vec2_f16"
OpMemberName %14 0 "fract"
OpMemberName %14 1 "exp"
OpMemberDecorate %14 0 Offset 0
OpMemberDecorate %14 1 Offset 8
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 16
%5 = OpTypeVector %6 2
%7 = OpConstant %6 0x1p+0
%8 = OpConstant %6 0x1p+1
%9 = OpConstantComposite %5 %7 %8
%11 = OpTypePointer Function %5
%12 = OpConstantNull %5
%16 = OpTypeInt 32 1
%15 = OpTypeVector %16 2
%14 = OpTypeStruct %5 %15
%3 = OpFunction %2 None %1
%4 = OpLabel
%10 = OpVariable %11 Function %12
OpStore %10 %9
%18 = OpLoad %5 %10
%13 = OpExtInst %14 %17 FrexpStruct %18
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(expect, got);

    Validate(b);
}

TEST_F(BuiltinBuilderTest, Const_Call_Frexp_f32) {
    Func("a_func", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Let("l", Call("frexp", vec2<f32>(1_f, 2_f)))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();
    auto got = DumpBuilder(b);
    auto* expect = R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %3 "a_func"
OpExecutionMode %3 OriginUpperLeft
OpName %3 "a_func"
OpName %5 "__frexp_result_vec2_f32"
OpMemberName %5 0 "fract"
OpMemberName %5 1 "exp"
OpMemberDecorate %5 0 Offset 0
OpMemberDecorate %5 1 Offset 8
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeFloat 32
%6 = OpTypeVector %7 2
%9 = OpTypeInt 32 1
%8 = OpTypeVector %9 2
%5 = OpTypeStruct %6 %8
%10 = OpConstant %7 0.5
%11 = OpConstantComposite %6 %10 %10
%12 = OpConstant %9 1
%13 = OpConstant %9 2
%14 = OpConstantComposite %8 %12 %13
%15 = OpConstantComposite %5 %11 %14
%3 = OpFunction %2 None %1
%4 = OpLabel
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(expect, got);

    Validate(b);
}

TEST_F(BuiltinBuilderTest, Const_Call_Frexp_f16) {
    Enable(builtin::Extension::kF16);

    Func("a_func", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Let("l", Call("frexp", vec2<f16>(1_h, 2_h)))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();
    auto got = DumpBuilder(b);
    auto* expect = R"(OpCapability Shader
OpCapability Float16
OpCapability UniformAndStorageBuffer16BitAccess
OpCapability StorageBuffer16BitAccess
OpCapability StorageInputOutput16
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %3 "a_func"
OpExecutionMode %3 OriginUpperLeft
OpName %3 "a_func"
OpName %5 "__frexp_result_vec2_f16"
OpMemberName %5 0 "fract"
OpMemberName %5 1 "exp"
OpMemberDecorate %5 0 Offset 0
OpMemberDecorate %5 1 Offset 8
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeFloat 16
%6 = OpTypeVector %7 2
%9 = OpTypeInt 32 1
%8 = OpTypeVector %9 2
%5 = OpTypeStruct %6 %8
%10 = OpConstant %7 0x1p-1
%11 = OpConstantComposite %6 %10 %10
%12 = OpConstant %9 1
%13 = OpConstant %9 2
%14 = OpConstantComposite %8 %12 %13
%15 = OpConstantComposite %5 %11 %14
%3 = OpFunction %2 None %1
%4 = OpLabel
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(expect, got);

    Validate(b);
}

TEST_F(BuiltinBuilderTest, Call_QuantizeToF16_Scalar) {
    GlobalVar("v", Expr(2_f), builtin::AddressSpace::kPrivate);

    Func("a_func", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Let("l", Call("quantizeToF16", "v"))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();
    auto got = DumpBuilder(b);
    auto* expect = R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %7 "a_func"
OpExecutionMode %7 OriginUpperLeft
OpName %3 "v"
OpName %7 "a_func"
%1 = OpTypeFloat 32
%2 = OpConstant %1 2
%4 = OpTypePointer Private %1
%3 = OpVariable %4 Private %2
%6 = OpTypeVoid
%5 = OpTypeFunction %6
%7 = OpFunction %6 None %5
%8 = OpLabel
%10 = OpLoad %1 %3
%9 = OpQuantizeToF16 %1 %10
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(expect, got);

    Validate(b);
}

TEST_F(BuiltinBuilderTest, Call_QuantizeToF16_Vector) {
    GlobalVar("v", vec3<f32>(2_f), builtin::AddressSpace::kPrivate);

    Func("a_func", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Let("l", Call("quantizeToF16", "v"))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();
    auto got = DumpBuilder(b);
    auto* expect = R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %24 "a_func"
OpExecutionMode %24 OriginUpperLeft
OpName %5 "v"
OpName %8 "tint_quantizeToF16"
OpName %9 "v"
OpName %24 "a_func"
%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 2
%4 = OpConstantComposite %1 %3 %3 %3
%6 = OpTypePointer Private %1
%5 = OpVariable %6 Private %4
%7 = OpTypeFunction %1 %1
%12 = OpTypeInt 32 0
%13 = OpConstantNull %12
%16 = OpConstant %12 1
%19 = OpConstant %12 2
%23 = OpTypeVoid
%22 = OpTypeFunction %23
%8 = OpFunction %1 None %7
%9 = OpFunctionParameter %1
%10 = OpLabel
%14 = OpCompositeExtract %2 %9 0
%11 = OpQuantizeToF16 %2 %14
%17 = OpCompositeExtract %2 %9 1
%15 = OpQuantizeToF16 %2 %17
%20 = OpCompositeExtract %2 %9 2
%18 = OpQuantizeToF16 %2 %20
%21 = OpCompositeConstruct %1 %11 %15 %18
OpReturnValue %21
OpFunctionEnd
%24 = OpFunction %23 None %22
%25 = OpLabel
%27 = OpLoad %1 %5
%26 = OpFunctionCall %1 %8 %27
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(expect, got);

    Validate(b);
}

}  // namespace float_builtin_tests

// Tests for Numeric builtins with all integer parameter
namespace integer_builtin_tests {

using BuiltinIntTest = BuiltinBuilderTestWithParam<BuiltinData>;
TEST_P(BuiltinIntTest, Call_SInt_Scalar) {
    auto param = GetParam();
    auto* var = GlobalVar("v", ty.i32(), builtin::AddressSpace::kPrivate);
    auto* expr = Call(param.name, "v");
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeInt 32 1
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
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), expected);
}

TEST_P(BuiltinIntTest, Call_SInt_Vector) {
    auto param = GetParam();
    auto* var = GlobalVar("v", ty.vec3<i32>(), builtin::AddressSpace::kPrivate);
    auto* expr = Call(param.name, "v");
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeInt 32 1
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
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), expected);
}

TEST_P(BuiltinIntTest, Call_UInt_Scalar) {
    auto param = GetParam();
    auto* var = GlobalVar("v", ty.u32(), builtin::AddressSpace::kPrivate);
    auto* expr = Call(param.name, "v");
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeInt 32 0
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
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), expected);
}

TEST_P(BuiltinIntTest, Call_UInt_Vector) {
    auto param = GetParam();
    auto* var = GlobalVar("v", ty.vec3<u32>(), builtin::AddressSpace::kPrivate);
    auto* expr = Call(param.name, "v");
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeInt 32 0
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
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), expected);
}
INSTANTIATE_TEST_SUITE_P(BuiltinBuilderTest,
                         BuiltinIntTest,
                         testing::Values(BuiltinData{"countOneBits", "OpBitCount"},
                                         BuiltinData{"reverseBits", "OpBitReverse"}));

using Builtin_Builder_SingleParam_Sint_Test = BuiltinBuilderTestWithParam<BuiltinData>;
TEST_P(Builtin_Builder_SingleParam_Sint_Test, Call_Scalar) {
    auto param = GetParam();
    auto* scalar = Var("scalar", Expr(1_i));
    auto* expr = Call(param.name, scalar);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(scalar),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%11 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %7 "scalar"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeInt 32 1
%6 = OpConstant %5 1
%8 = OpTypePointer Function %5
%9 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%7 = OpVariable %8 Function %9
OpStore %7 %6
%12 = OpLoad %5 %7
%10 = OpExtInst %5 %11 )" +
                  param.op +
                  R"( %12
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

TEST_P(Builtin_Builder_SingleParam_Sint_Test, Call_Vector) {
    auto param = GetParam();
    auto* vec = Var("vec", vec2<i32>(1_i, 1_i));
    auto* expr = Call(param.name, vec);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(vec),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%13 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %9 "vec"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 1
%5 = OpTypeVector %6 2
%7 = OpConstant %6 1
%8 = OpConstantComposite %5 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%9 = OpVariable %10 Function %11
OpStore %9 %8
%14 = OpLoad %5 %9
%12 = OpExtInst %5 %13 )" +
                  param.op +
                  R"( %14
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}
INSTANTIATE_TEST_SUITE_P(BuiltinBuilderTest,
                         Builtin_Builder_SingleParam_Sint_Test,
                         testing::Values(BuiltinData{"abs", "SAbs"}));

// Calling abs() on an unsigned integer scalar / vector is a no-op.
using Builtin_Builder_Abs_Uint_Test = BuiltinBuilderTest;
TEST_F(Builtin_Builder_Abs_Uint_Test, Call_Scalar) {
    auto* scalar = Var("scalar", Expr(1_u));
    auto* expr = Call("abs", scalar);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(scalar),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(OpName %3 "a_func"
OpName %7 "scalar"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeInt 32 0
%6 = OpConstant %5 1
%8 = OpTypePointer Function %5
%9 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%7 = OpVariable %8 Function %9
OpStore %7 %6
%11 = OpLoad %5 %7
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

TEST_F(Builtin_Builder_Abs_Uint_Test, Call_Vector) {
    auto* scalar = Var("scalar", vec2<u32>(1_u, 1_u));
    auto* expr = Call("abs", scalar);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(scalar),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(OpName %3 "a_func"
OpName %9 "scalar"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 0
%5 = OpTypeVector %6 2
%7 = OpConstant %6 1
%8 = OpConstantComposite %5 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%9 = OpVariable %10 Function %11
OpStore %9 %8
%13 = OpLoad %5 %9
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

using Builtin_Builder_DualParam_SInt_Test = BuiltinBuilderTestWithParam<BuiltinData>;
TEST_P(Builtin_Builder_DualParam_SInt_Test, Call_Scalar) {
    auto param = GetParam();
    auto* scalar = Var("scalar", Expr(1_i));
    auto* expr = Call(param.name, scalar, scalar);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(scalar),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%11 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %7 "scalar"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeInt 32 1
%6 = OpConstant %5 1
%8 = OpTypePointer Function %5
%9 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%7 = OpVariable %8 Function %9
OpStore %7 %6
%12 = OpLoad %5 %7
%13 = OpLoad %5 %7
%10 = OpExtInst %5 %11 )" +
                  param.op +
                  R"( %12 %13
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

TEST_P(Builtin_Builder_DualParam_SInt_Test, Call_Vector) {
    auto param = GetParam();
    auto* vec = Var("vec", vec2<i32>(1_i, 1_i));
    auto* expr = Call(param.name, vec, vec);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(vec),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%13 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %9 "vec"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 1
%5 = OpTypeVector %6 2
%7 = OpConstant %6 1
%8 = OpConstantComposite %5 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%9 = OpVariable %10 Function %11
OpStore %9 %8
%14 = OpLoad %5 %9
%15 = OpLoad %5 %9
%12 = OpExtInst %5 %13 )" +
                  param.op +
                  R"( %14 %15
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}
INSTANTIATE_TEST_SUITE_P(BuiltinBuilderTest,
                         Builtin_Builder_DualParam_SInt_Test,
                         testing::Values(BuiltinData{"max", "SMax"}, BuiltinData{"min", "SMin"}));

using Builtin_Builder_DualParam_UInt_Test = BuiltinBuilderTestWithParam<BuiltinData>;
TEST_P(Builtin_Builder_DualParam_UInt_Test, Call_Scalar) {
    auto param = GetParam();
    auto* scalar = Var("scalar", Expr(1_u));
    auto* expr = Call(param.name, scalar, scalar);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(scalar),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%11 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %7 "scalar"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeInt 32 0
%6 = OpConstant %5 1
%8 = OpTypePointer Function %5
%9 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%7 = OpVariable %8 Function %9
OpStore %7 %6
%12 = OpLoad %5 %7
%13 = OpLoad %5 %7
%10 = OpExtInst %5 %11 )" +
                  param.op +
                  R"( %12 %13
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

TEST_P(Builtin_Builder_DualParam_UInt_Test, Call_Vector) {
    auto param = GetParam();
    auto* vec = Var("vec", vec2<u32>(1_u, 1_u));
    auto* expr = Call(param.name, vec, vec);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(vec),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%13 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %9 "vec"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 0
%5 = OpTypeVector %6 2
%7 = OpConstant %6 1
%8 = OpConstantComposite %5 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%9 = OpVariable %10 Function %11
OpStore %9 %8
%14 = OpLoad %5 %9
%15 = OpLoad %5 %9
%12 = OpExtInst %5 %13 )" +
                  param.op +
                  R"( %14 %15
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}
INSTANTIATE_TEST_SUITE_P(BuiltinBuilderTest,
                         Builtin_Builder_DualParam_UInt_Test,
                         testing::Values(BuiltinData{"max", "UMax"}, BuiltinData{"min", "UMin"}));

using Builtin_Builder_ThreeParam_Sint_Test = BuiltinBuilderTestWithParam<BuiltinData>;
TEST_P(Builtin_Builder_ThreeParam_Sint_Test, Call_Scalar) {
    auto param = GetParam();
    auto* scalar = Var("scalar", Expr(1_i));
    auto* expr = Call(param.name, scalar, scalar, scalar);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(scalar),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%11 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %7 "scalar"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeInt 32 1
%6 = OpConstant %5 1
%8 = OpTypePointer Function %5
%9 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%7 = OpVariable %8 Function %9
OpStore %7 %6
%12 = OpLoad %5 %7
%13 = OpLoad %5 %7
%14 = OpLoad %5 %7
%10 = OpExtInst %5 %11 )" +
                  param.op +
                  R"( %12 %13 %14
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

TEST_P(Builtin_Builder_ThreeParam_Sint_Test, Call_Vector) {
    auto param = GetParam();
    auto* vec = Var("vec", vec2<i32>(1_i, 1_i));
    auto* expr = Call(param.name, vec, vec, vec);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(vec),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%13 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %9 "vec"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 1
%5 = OpTypeVector %6 2
%7 = OpConstant %6 1
%8 = OpConstantComposite %5 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%9 = OpVariable %10 Function %11
OpStore %9 %8
%14 = OpLoad %5 %9
%15 = OpLoad %5 %9
%16 = OpLoad %5 %9
%12 = OpExtInst %5 %13 )" +
                  param.op +
                  R"( %14 %15 %16
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}
INSTANTIATE_TEST_SUITE_P(BuiltinBuilderTest,
                         Builtin_Builder_ThreeParam_Sint_Test,
                         testing::Values(BuiltinData{"clamp", "SClamp"}));

using Builtin_Builder_ThreeParam_Uint_Test = BuiltinBuilderTestWithParam<BuiltinData>;
TEST_P(Builtin_Builder_ThreeParam_Uint_Test, Call_Scalar) {
    auto param = GetParam();
    auto* scalar = Var("scalar", Expr(1_u));
    auto* expr = Call(param.name, scalar, scalar, scalar);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(scalar),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%11 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %7 "scalar"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeInt 32 0
%6 = OpConstant %5 1
%8 = OpTypePointer Function %5
%9 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%7 = OpVariable %8 Function %9
OpStore %7 %6
%12 = OpLoad %5 %7
%13 = OpLoad %5 %7
%14 = OpLoad %5 %7
%10 = OpExtInst %5 %11 )" +
                  param.op +
                  R"( %12 %13 %14
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

TEST_P(Builtin_Builder_ThreeParam_Uint_Test, Call_Vector) {
    auto param = GetParam();
    auto* vec = Var("vec", vec2<u32>(1_u, 1_u));
    auto* expr = Call(param.name, vec, vec, vec);
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(vec),
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%13 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %9 "vec"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 0
%5 = OpTypeVector %6 2
%7 = OpConstant %6 1
%8 = OpConstantComposite %5 %7 %7
%10 = OpTypePointer Function %5
%11 = OpConstantNull %5
%3 = OpFunction %2 None %1
%4 = OpLabel
%9 = OpVariable %10 Function %11
OpStore %9 %8
%14 = OpLoad %5 %9
%15 = OpLoad %5 %9
%16 = OpLoad %5 %9
%12 = OpExtInst %5 %13 )" +
                  param.op +
                  R"( %14 %15 %16
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}
INSTANTIATE_TEST_SUITE_P(BuiltinBuilderTest,
                         Builtin_Builder_ThreeParam_Uint_Test,
                         testing::Values(BuiltinData{"clamp", "UClamp"}));

TEST_F(BuiltinBuilderTest, Call_ExtractBits_i32) {
    auto* v = Var("v", ty.i32());
    auto* offset = Var("offset", ty.u32());
    auto* count = Var("count", ty.u32());
    auto* call = Call("extractBits", v, offset, count);
    auto* func = WrapInFunction(v, offset, count, call);

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(OpEntryPoint GLCompute %3 "test_function"
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
)";
    EXPECT_EQ(got, expect);
}

TEST_F(BuiltinBuilderTest, Call_ExtractBits_u32) {
    auto* v = Var("v", ty.u32());
    auto* offset = Var("offset", ty.u32());
    auto* count = Var("count", ty.u32());
    auto* call = Call("extractBits", v, offset, count);
    auto* func = WrapInFunction(v, offset, count, call);

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(OpEntryPoint GLCompute %3 "test_function"
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
)";
    EXPECT_EQ(got, expect);
}

TEST_F(BuiltinBuilderTest, Call_ExtractBits_vec3_i32) {
    auto* v = Var("v", ty.vec3<i32>());
    auto* offset = Var("offset", ty.u32());
    auto* count = Var("count", ty.u32());
    auto* call = Call("extractBits", v, offset, count);
    auto* func = WrapInFunction(v, offset, count, call);

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(OpEntryPoint GLCompute %3 "test_function"
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
)";
    EXPECT_EQ(got, expect);
}

TEST_F(BuiltinBuilderTest, Call_ExtractBits_vec3_u32) {
    auto* v = Var("v", ty.vec3<u32>());
    auto* offset = Var("offset", ty.u32());
    auto* count = Var("count", ty.u32());
    auto* call = Call("extractBits", v, offset, count);
    auto* func = WrapInFunction(v, offset, count, call);

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(OpEntryPoint GLCompute %3 "test_function"
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
)";
    EXPECT_EQ(got, expect);
}

TEST_F(BuiltinBuilderTest, Call_InsertBits_i32) {
    auto* v = Var("v", ty.i32());
    auto* n = Var("n", ty.i32());
    auto* offset = Var("offset", ty.u32());
    auto* count = Var("count", ty.u32());
    auto* call = Call("insertBits", v, n, offset, count);
    auto* func = WrapInFunction(v, n, offset, count, call);

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(OpEntryPoint GLCompute %3 "test_function"
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
)";
    EXPECT_EQ(got, expect);
}

TEST_F(BuiltinBuilderTest, Call_InsertBits_u32) {
    auto* v = Var("v", ty.u32());
    auto* n = Var("n", ty.u32());
    auto* offset = Var("offset", ty.u32());
    auto* count = Var("count", ty.u32());
    auto* call = Call("insertBits", v, n, offset, count);
    auto* func = WrapInFunction(v, n, offset, count, call);

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(OpEntryPoint GLCompute %3 "test_function"
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
)";
    EXPECT_EQ(got, expect);
}

TEST_F(BuiltinBuilderTest, Call_InsertBits_vec3_i32) {
    auto* v = Var("v", ty.vec3<i32>());
    auto* n = Var("n", ty.vec3<i32>());
    auto* offset = Var("offset", ty.u32());
    auto* count = Var("count", ty.u32());
    auto* call = Call("insertBits", v, n, offset, count);
    auto* func = WrapInFunction(v, n, offset, count, call);

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(OpEntryPoint GLCompute %3 "test_function"
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
)";
    EXPECT_EQ(got, expect);
}

TEST_F(BuiltinBuilderTest, Call_InsertBits_vec3_u32) {
    auto* v = Var("v", ty.vec3<u32>());
    auto* n = Var("n", ty.vec3<u32>());
    auto* offset = Var("offset", ty.u32());
    auto* count = Var("count", ty.u32());
    auto* call = Call("insertBits", v, n, offset, count);
    auto* func = WrapInFunction(v, n, offset, count, call);

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(OpEntryPoint GLCompute %3 "test_function"
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
)";
    EXPECT_EQ(got, expect);
}

}  // namespace integer_builtin_tests

// Tests for Numeric builtins with matrix parameter, i.e. "determinant" and "transpose"
namespace matrix_builtin_tests {

TEST_F(BuiltinBuilderTest, Call_Determinant_f32) {
    auto* var = GlobalVar("var", ty.mat3x3<f32>(), builtin::AddressSpace::kPrivate);
    auto* expr = Call("determinant", "var");
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%12 = OpExtInstImport "GLSL.std.450"
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
)";
    EXPECT_EQ(got, expect);
}

TEST_F(BuiltinBuilderTest, Call_Determinant_f16) {
    Enable(builtin::Extension::kF16);

    auto* var = GlobalVar("var", ty.mat3x3<f16>(), builtin::AddressSpace::kPrivate);
    auto* expr = Call("determinant", "var");
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(%12 = OpExtInstImport "GLSL.std.450"
OpName %1 "var"
OpName %9 "a_func"
%5 = OpTypeFloat 16
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
)";
    EXPECT_EQ(got, expect);
}

TEST_F(BuiltinBuilderTest, Call_Transpose_f32) {
    auto* var = GlobalVar("var", ty.mat2x3<f32>(), builtin::AddressSpace::kPrivate);
    auto* expr = Call("transpose", "var");
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(OpName %1 "var"
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
)";
    EXPECT_EQ(got, expect);
}

TEST_F(BuiltinBuilderTest, Call_Transpose_f16) {
    Enable(builtin::Extension::kF16);

    auto* var = GlobalVar("var", ty.mat2x3<f16>(), builtin::AddressSpace::kPrivate);
    auto* expr = Call("transpose", "var");
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(OpName %1 "var"
OpName %9 "a_func"
%5 = OpTypeFloat 16
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
)";
    EXPECT_EQ(got, expect);
}

}  // namespace matrix_builtin_tests

// Tests for Numeric builtins with float and integer vector parameter, i.e. "dot"
namespace vector_builtin_tests {

TEST_F(BuiltinBuilderTest, Call_Dot_F32) {
    auto* var = GlobalVar("v", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);
    auto* expr = Call("dot", "v", "v");
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%7 = OpTypeVoid
%6 = OpTypeFunction %7
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(%11 = OpLoad %3 %1
%12 = OpLoad %3 %1
%10 = OpDot %4 %11 %12
OpReturn
)");
}

TEST_F(BuiltinBuilderTest, Call_Dot_F16) {
    Enable(builtin::Extension::kF16);

    auto* var = GlobalVar("v", ty.vec3<f16>(), builtin::AddressSpace::kPrivate);
    auto* expr = Call("dot", "v", "v");
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeFloat 16
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%7 = OpTypeVoid
%6 = OpTypeFunction %7
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(%11 = OpLoad %3 %1
%12 = OpLoad %3 %1
%10 = OpDot %4 %11 %12
OpReturn
)");
}

TEST_F(BuiltinBuilderTest, Call_Dot_U32) {
    auto* var = GlobalVar("v", ty.vec3<u32>(), builtin::AddressSpace::kPrivate);
    auto* expr = Call("dot", "v", "v");
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeInt 32 0
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%7 = OpTypeVoid
%6 = OpTypeFunction %7
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
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
    auto* var = GlobalVar("v", ty.vec3<i32>(), builtin::AddressSpace::kPrivate);
    auto* expr = Call("dot", "v", "v");
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(Let("l", expr)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeInt 32 1
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%7 = OpTypeVoid
%6 = OpTypeFunction %7
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
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

}  // namespace vector_builtin_tests

// Tests for Derivative builtins
namespace derivative_builtin_tests {

using BuiltinDeriveTest = BuiltinBuilderTestWithParam<BuiltinData>;
TEST_P(BuiltinDeriveTest, Call_Derivative_Scalar) {
    auto param = GetParam();
    auto* var = GlobalVar("v", ty.f32(), builtin::AddressSpace::kPrivate);
    auto* expr = Call(param.name, "v");
    auto* func = Func("func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(Let("l", expr)),
                      },
                      utils::Vector{
                          Stage(ast::PipelineStage::kFragment),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeFloat 32
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
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), expected);
}

TEST_P(BuiltinDeriveTest, Call_Derivative_Vector) {
    auto param = GetParam();
    auto* var = GlobalVar("v", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);
    auto* expr = Call(param.name, "v");
    auto* func = Func("func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(Let("l", expr)),
                      },
                      utils::Vector{
                          Stage(ast::PipelineStage::kFragment),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    if (param.name != "dpdx" && param.name != "dpdy" && param.name != "fwidth") {
        EXPECT_EQ(DumpInstructions(b.Module().Capabilities()),
                  R"(OpCapability DerivativeControl
)");
    }

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%4 = OpTypeFloat 32
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
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), expected);
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

}  // namespace derivative_builtin_tests

// Tests for Atomic builtins
namespace atomic_builtin_tests {

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
    auto* s = Structure("S", utils::Vector{
                                 Member("u", ty.atomic<u32>()),
                                 Member("i", ty.atomic<i32>()),
                             });
    GlobalVar("b", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kReadWrite,
              Binding(1_a), Group(2_a));

    Func("a_func", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Let("u", ty.u32(), Call("atomicLoad", AddressOf(MemberAccessor("b", "u"))))),
             Decl(Let("i", ty.i32(), Call("atomicLoad", AddressOf(MemberAccessor("b", "i"))))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    ASSERT_EQ(b.Module().Functions().size(), 1_u);

    auto* expected_types = R"(%5 = OpTypeInt 32 0
%6 = OpTypeInt 32 1
%4 = OpTypeStruct %5 %6
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%8 = OpTypeVoid
%7 = OpTypeFunction %8
%12 = OpConstant %5 1
%13 = OpConstant %5 0
%15 = OpTypePointer StorageBuffer %5
%19 = OpTypePointer StorageBuffer %6
)";
    auto got_types = DumpInstructions(b.Module().Types());
    EXPECT_EQ(expected_types, got_types);

    auto* expected_instructions = R"(%16 = OpAccessChain %15 %1 %13 %13
%11 = OpAtomicLoad %5 %16 %12 %13
%20 = OpAccessChain %19 %1 %13 %12
%17 = OpAtomicLoad %6 %20 %12 %13
OpReturn
)";
    auto got_instructions = DumpInstructions(b.Module().Functions()[0].instructions());
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
    auto* s = Structure("S", utils::Vector{
                                 Member("u", ty.atomic<u32>()),
                                 Member("i", ty.atomic<i32>()),
                             });
    GlobalVar("b", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kReadWrite,
              Binding(1_a), Group(2_a));

    Func("a_func", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("u", Expr(1_u))),
             Decl(Var("i", Expr(2_i))),
             CallStmt(Call("atomicStore", AddressOf(MemberAccessor("b", "u")), "u")),
             CallStmt(Call("atomicStore", AddressOf(MemberAccessor("b", "i")), "i")),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    ASSERT_EQ(b.Module().Functions().size(), 1_u);

    auto* expected_types = R"(%5 = OpTypeInt 32 0
%6 = OpTypeInt 32 1
%4 = OpTypeStruct %5 %6
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%8 = OpTypeVoid
%7 = OpTypeFunction %8
%11 = OpConstant %5 1
%13 = OpTypePointer Function %5
%14 = OpConstantNull %5
%15 = OpConstant %6 2
%17 = OpTypePointer Function %6
%18 = OpConstantNull %6
%20 = OpConstant %5 0
%22 = OpTypePointer StorageBuffer %5
%27 = OpTypePointer StorageBuffer %6
)";
    auto got_types = DumpInstructions(b.Module().Types());
    EXPECT_EQ(expected_types, got_types);

    auto* expected_instructions = R"(OpStore %12 %11
OpStore %16 %15
%23 = OpAccessChain %22 %1 %20 %20
%24 = OpLoad %5 %12
OpAtomicStore %23 %11 %20 %24
%28 = OpAccessChain %27 %1 %20 %11
%29 = OpLoad %6 %16
OpAtomicStore %28 %11 %20 %29
OpReturn
)";
    auto got_instructions = DumpInstructions(b.Module().Functions()[0].instructions());
    EXPECT_EQ(expected_instructions, got_instructions);

    Validate(b);
}

using Builtin_Builder_AtomicRMW_i32 = BuiltinBuilderTestWithParam<BuiltinData>;
TEST_P(Builtin_Builder_AtomicRMW_i32, Test) {
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
    auto* s = Structure("S", utils::Vector{
                                 Member("v", ty.atomic<i32>()),
                             });
    GlobalVar("b", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kReadWrite,
              Binding(1_a), Group(2_a));

    Func("a_func", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("v", Expr(10_i))),
             Decl(Let("x", ty.i32(),
                      Call(GetParam().name, AddressOf(MemberAccessor("b", "v")), "v"))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    ASSERT_EQ(b.Module().Functions().size(), 1_u);

    std::string expected_types = R"(%5 = OpTypeInt 32 1
%4 = OpTypeStruct %5
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%7 = OpTypeVoid
%6 = OpTypeFunction %7
%10 = OpConstant %5 10
%12 = OpTypePointer Function %5
%13 = OpConstantNull %5
%15 = OpTypeInt 32 0
%16 = OpConstant %15 1
%17 = OpConstant %15 0
%19 = OpTypePointer StorageBuffer %5
)";
    auto got_types = DumpInstructions(b.Module().Types());
    EXPECT_EQ(expected_types, got_types);

    std::string expected_instructions = R"(OpStore %11 %10
%20 = OpAccessChain %19 %1 %17 %17
%21 = OpLoad %5 %11
)";
    expected_instructions += "%14 = " + GetParam().op + " %5 %20 %16 %17 %21\n";
    expected_instructions += "OpReturn\n";

    auto got_instructions = DumpInstructions(b.Module().Functions()[0].instructions());
    EXPECT_EQ(expected_instructions, got_instructions);

    Validate(b);
}
INSTANTIATE_TEST_SUITE_P(BuiltinBuilderTest,
                         Builtin_Builder_AtomicRMW_i32,
                         testing::Values(BuiltinData{"atomicAdd", "OpAtomicIAdd"},
                                         BuiltinData{"atomicMax", "OpAtomicSMax"},
                                         BuiltinData{"atomicMin", "OpAtomicSMin"},
                                         BuiltinData{"atomicAnd", "OpAtomicAnd"},
                                         BuiltinData{"atomicOr", "OpAtomicOr"},
                                         BuiltinData{"atomicXor", "OpAtomicXor"}));

using Builtin_Builder_AtomicRMW_u32 = BuiltinBuilderTestWithParam<BuiltinData>;
TEST_P(Builtin_Builder_AtomicRMW_u32, Test) {
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
    auto* s = Structure("S", utils::Vector{
                                 Member("v", ty.atomic<u32>()),
                             });
    GlobalVar("b", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kReadWrite,
              Binding(1_a), Group(2_a));

    Func("a_func", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("v", Expr(10_u))),
             Decl(Let("x", ty.u32(),
                      Call(GetParam().name, AddressOf(MemberAccessor("b", "v")), "v"))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    ASSERT_EQ(b.Module().Functions().size(), 1_u);

    std::string expected_types = R"(%5 = OpTypeInt 32 0
%4 = OpTypeStruct %5
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%7 = OpTypeVoid
%6 = OpTypeFunction %7
%10 = OpConstant %5 10
%12 = OpTypePointer Function %5
%13 = OpConstantNull %5
%15 = OpConstant %5 1
%16 = OpConstant %5 0
%18 = OpTypePointer StorageBuffer %5
)";
    auto got_types = DumpInstructions(b.Module().Types());
    EXPECT_EQ(expected_types, got_types);

    std::string expected_instructions = R"(OpStore %11 %10
%19 = OpAccessChain %18 %1 %16 %16
%20 = OpLoad %5 %11
)";
    expected_instructions += "%14 = " + GetParam().op + " %5 %19 %15 %16 %20\n";
    expected_instructions += "OpReturn\n";

    auto got_instructions = DumpInstructions(b.Module().Functions()[0].instructions());
    EXPECT_EQ(expected_instructions, got_instructions);

    Validate(b);
}
INSTANTIATE_TEST_SUITE_P(BuiltinBuilderTest,
                         Builtin_Builder_AtomicRMW_u32,
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
    auto* s = Structure("S", utils::Vector{
                                 Member("u", ty.atomic<u32>()),
                                 Member("i", ty.atomic<i32>()),
                             });
    GlobalVar("b", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kReadWrite,
              Binding(1_a), Group(2_a));

    Func("a_func", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("u", Expr(10_u))),
             Decl(Var("i", Expr(10_i))),
             Decl(Let("r", ty.u32(),
                      Call("atomicExchange", AddressOf(MemberAccessor("b", "u")), "u"))),
             Decl(Let("s", ty.i32(),
                      Call("atomicExchange", AddressOf(MemberAccessor("b", "i")), "i"))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    ASSERT_EQ(b.Module().Functions().size(), 1_u);

    auto* expected_types = R"(%5 = OpTypeInt 32 0
%6 = OpTypeInt 32 1
%4 = OpTypeStruct %5 %6
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%8 = OpTypeVoid
%7 = OpTypeFunction %8
%11 = OpConstant %5 10
%13 = OpTypePointer Function %5
%14 = OpConstantNull %5
%15 = OpConstant %6 10
%17 = OpTypePointer Function %6
%18 = OpConstantNull %6
%20 = OpConstant %5 1
%21 = OpConstant %5 0
%23 = OpTypePointer StorageBuffer %5
%28 = OpTypePointer StorageBuffer %6
)";
    auto got_types = DumpInstructions(b.Module().Types());
    EXPECT_EQ(expected_types, got_types);

    auto* expected_instructions = R"(OpStore %12 %11
OpStore %16 %15
%24 = OpAccessChain %23 %1 %21 %21
%25 = OpLoad %5 %12
%19 = OpAtomicExchange %5 %24 %20 %21 %25
%29 = OpAccessChain %28 %1 %21 %20
%30 = OpLoad %6 %16
%26 = OpAtomicExchange %6 %29 %20 %21 %30
OpReturn
)";
    auto got_instructions = DumpInstructions(b.Module().Functions()[0].instructions());
    EXPECT_EQ(expected_instructions, got_instructions);

    Validate(b);
}

TEST_F(BuiltinBuilderTest, Call_AtomicCompareExchangeWeak) {
    // struct S {
    //   u : atomic<u32>,
    //   i : atomic<i32>,
    // }
    //
    // @binding(1) @group(2) var<storage, read_write> b : S;
    //
    // fn a_func() {
    //   let u = atomicCompareExchangeWeak(&b.u, 10u, 20u);
    //   let i = atomicCompareExchangeWeak(&b.i, 10, 10);
    // }
    auto* s = Structure("S", utils::Vector{
                                 Member("u", ty.atomic<u32>()),
                                 Member("i", ty.atomic<i32>()),
                             });
    GlobalVar("b", ty.Of(s), builtin::AddressSpace::kStorage, builtin::Access::kReadWrite,
              Binding(1_a), Group(2_a));

    Func("a_func", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Let("u", Call("atomicCompareExchangeWeak", AddressOf(MemberAccessor("b", "u")),
                                10_u, 20_u))),
             Decl(Let("i", Call("atomicCompareExchangeWeak", AddressOf(MemberAccessor("b", "i")),
                                10_i, 20_i))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    spirv::Builder& b = SanitizeAndBuild();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    ASSERT_EQ(b.Module().Functions().size(), 1_u);

    auto* expected_types = R"(%5 = OpTypeInt 32 0
%6 = OpTypeInt 32 1
%4 = OpTypeStruct %5 %6
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%8 = OpTypeVoid
%7 = OpTypeFunction %8
%13 = OpTypeBool
%12 = OpTypeStruct %5 %13
%14 = OpConstant %5 1
%15 = OpConstant %5 0
%17 = OpTypePointer StorageBuffer %5
%19 = OpConstant %5 20
%20 = OpConstant %5 10
%24 = OpTypeStruct %6 %13
%26 = OpTypePointer StorageBuffer %6
%28 = OpConstant %6 20
%29 = OpConstant %6 10
)";
    auto got_types = DumpInstructions(b.Module().Types());
    EXPECT_EQ(expected_types, got_types);

    auto* expected_instructions = R"(%18 = OpAccessChain %17 %1 %15 %15
%21 = OpAtomicCompareExchange %5 %18 %14 %15 %15 %19 %20
%22 = OpIEqual %13 %21 %20
%11 = OpCompositeConstruct %12 %21 %22
%27 = OpAccessChain %26 %1 %15 %14
%30 = OpAtomicCompareExchange %6 %27 %14 %15 %15 %28 %29
%31 = OpIEqual %13 %30 %29
%23 = OpCompositeConstruct %24 %30 %31
OpReturn
)";
    auto got_instructions = DumpInstructions(b.Module().Functions()[0].instructions());
    EXPECT_EQ(expected_instructions, got_instructions);

    Validate(b);
}

}  // namespace atomic_builtin_tests

// Tests for Data Packing builtins
namespace data_packing_builtin_tests {

using Builtin_Builder_DataPacking_Test = BuiltinBuilderTestWithParam<BuiltinData>;
TEST_P(Builtin_Builder_DataPacking_Test, Binary) {
    auto param = GetParam();

    bool pack4 = param.name == "pack4x8snorm" || param.name == "pack4x8unorm";
    auto* call = pack4 ? Call(param.name, vec4<f32>("one")) : Call(param.name, vec2<f32>("one"));
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(Let("one", Expr(1_f))),
                          Decl(Let("l", call)),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    if (pack4) {
        auto got = DumpBuilder(b);
        auto expect = R"(%9 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeFloat 32
%6 = OpConstant %5 1
%8 = OpTypeInt 32 0
%10 = OpTypeVector %5 4
%3 = OpFunction %2 None %1
%4 = OpLabel
%11 = OpCompositeConstruct %10 %6 %6 %6 %6
%7 = OpExtInst %8 %9 )" +
                      param.op +
                      R"( %11
OpReturn
OpFunctionEnd
)";
        EXPECT_EQ(got, expect);
    } else {
        auto got = DumpBuilder(b);
        auto expect = R"(%9 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeFloat 32
%6 = OpConstant %5 1
%8 = OpTypeInt 32 0
%10 = OpTypeVector %5 2
%3 = OpFunction %2 None %1
%4 = OpLabel
%11 = OpCompositeConstruct %10 %6 %6
%7 = OpExtInst %8 %9 )" +
                      param.op +
                      R"( %11
OpReturn
OpFunctionEnd
)";
        EXPECT_EQ(got, expect);
    }
}

INSTANTIATE_TEST_SUITE_P(BuiltinBuilderTest,
                         Builtin_Builder_DataPacking_Test,
                         testing::Values(BuiltinData{"pack4x8snorm", "PackSnorm4x8"},
                                         BuiltinData{"pack4x8unorm", "PackUnorm4x8"},
                                         BuiltinData{"pack2x16snorm", "PackSnorm2x16"},
                                         BuiltinData{"pack2x16unorm", "PackUnorm2x16"},
                                         BuiltinData{"pack2x16float", "PackHalf2x16"}));

}  // namespace data_packing_builtin_tests

// Tests for Data Unpacking builtins
namespace data_unpacking_builtin_tests {

using Builtin_Builder_DataUnpacking_Test = BuiltinBuilderTestWithParam<BuiltinData>;
TEST_P(Builtin_Builder_DataUnpacking_Test, Binary) {
    auto param = GetParam();

    bool pack4 = param.name == "unpack4x8snorm" || param.name == "unpack4x8unorm";
    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Decl(Let("one", Expr(1_u))),
                          Decl(Let("l", Call(param.name, "one"))),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    if (pack4) {
        auto got = DumpBuilder(b);
        auto expect = R"(%10 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeInt 32 0
%6 = OpConstant %5 1
%9 = OpTypeFloat 32
%8 = OpTypeVector %9 4
%3 = OpFunction %2 None %1
%4 = OpLabel
%7 = OpExtInst %8 %10 )" +
                      param.op +
                      R"( %6
OpReturn
OpFunctionEnd
)";
        EXPECT_EQ(got, expect);
    } else {
        auto got = DumpBuilder(b);
        auto expect = R"(%10 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeInt 32 0
%6 = OpConstant %5 1
%9 = OpTypeFloat 32
%8 = OpTypeVector %9 2
%3 = OpFunction %2 None %1
%4 = OpLabel
%7 = OpExtInst %8 %10 )" +
                      param.op +
                      R"( %6
OpReturn
OpFunctionEnd
)";
        EXPECT_EQ(got, expect);
    }
}

INSTANTIATE_TEST_SUITE_P(BuiltinBuilderTest,
                         Builtin_Builder_DataUnpacking_Test,
                         testing::Values(BuiltinData{"unpack4x8snorm", "UnpackSnorm4x8"},
                                         BuiltinData{"unpack4x8unorm", "UnpackUnorm4x8"},
                                         BuiltinData{"unpack2x16snorm", "UnpackSnorm2x16"},
                                         BuiltinData{"unpack2x16unorm", "UnpackUnorm2x16"},
                                         BuiltinData{"unpack2x16float", "UnpackHalf2x16"}));

}  // namespace data_unpacking_builtin_tests

// Tests for Synchronization builtins
namespace synchronization_builtin_tests {

TEST_F(BuiltinBuilderTest, Call_WorkgroupBarrier) {
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             CallStmt(Call("workgroupBarrier")),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    ASSERT_EQ(b.Module().Functions().size(), 1_u);

    auto* expected_types = R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 0
%7 = OpConstant %6 2
%8 = OpConstant %6 264
)";
    auto got_types = DumpInstructions(b.Module().Types());
    EXPECT_EQ(expected_types, got_types);

    auto* expected_instructions = R"(OpControlBarrier %7 %7 %8
OpReturn
)";
    auto got_instructions = DumpInstructions(b.Module().Functions()[0].instructions());
    EXPECT_EQ(expected_instructions, got_instructions);

    Validate(b);
}

TEST_F(BuiltinBuilderTest, Call_StorageBarrier) {
    Func("f", utils::Empty, ty.void_(),
         utils::Vector{
             CallStmt(Call("storageBarrier")),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    ASSERT_EQ(b.Module().Functions().size(), 1_u);

    auto* expected_types = R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 0
%7 = OpConstant %6 2
%8 = OpConstant %6 72
)";
    auto got_types = DumpInstructions(b.Module().Types());
    EXPECT_EQ(expected_types, got_types);

    auto* expected_instructions = R"(OpControlBarrier %7 %7 %8
OpReturn
)";
    auto got_instructions = DumpInstructions(b.Module().Functions()[0].instructions());
    EXPECT_EQ(expected_instructions, got_instructions);

    Validate(b);
}

}  // namespace synchronization_builtin_tests

// Tests for DP4A builtins, tint:1497
namespace DP4A_builtin_tests {

TEST_F(BuiltinBuilderTest, Call_Dot4I8Packed) {
    Enable(builtin::Extension::kChromiumExperimentalDp4A);

    auto* val1 = Var("val1", ty.u32());
    auto* val2 = Var("val2", ty.u32());
    auto* call = Call("dot4I8Packed", val1, val2);
    auto* func = WrapInFunction(val1, val2, call);

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(OpEntryPoint GLCompute %3 "test_function"
OpExecutionMode %3 LocalSize 1 1 1
OpName %3 "test_function"
OpName %5 "val1"
OpName %9 "val2"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeInt 32 0
%6 = OpTypePointer Function %7
%8 = OpConstantNull %7
%11 = OpTypeInt 32 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpVariable %6 Function %8
%9 = OpVariable %6 Function %8
%12 = OpLoad %7 %5
%13 = OpLoad %7 %9
%10 = OpSDot %11 %12 %13 PackedVectorFormat4x8Bit
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

TEST_F(BuiltinBuilderTest, Call_Dot4U8Packed) {
    Enable(builtin::Extension::kChromiumExperimentalDp4A);

    auto* val1 = Var("val1", ty.u32());
    auto* val2 = Var("val2", ty.u32());
    auto* call = Call("dot4U8Packed", val1, val2);
    auto* func = WrapInFunction(val1, val2, call);

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    auto got = DumpBuilder(b);
    auto expect = R"(OpEntryPoint GLCompute %3 "test_function"
OpExecutionMode %3 LocalSize 1 1 1
OpName %3 "test_function"
OpName %5 "val1"
OpName %9 "val2"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeInt 32 0
%6 = OpTypePointer Function %7
%8 = OpConstantNull %7
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpVariable %6 Function %8
%9 = OpVariable %6 Function %8
%11 = OpLoad %7 %5
%12 = OpLoad %7 %9
%10 = OpUDot %7 %11 %12 PackedVectorFormat4x8Bit
OpReturn
OpFunctionEnd
)";
    EXPECT_EQ(got, expect);
}

}  // namespace DP4A_builtin_tests

}  // namespace
}  // namespace tint::writer::spirv
